/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sensors/ei_microphone.h"
#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

#include <zephyr/kernel.h>
#include <zephyr/audio/dmic.h>
#include <string.h>

#define AUDIO_FREQ          EI_CLASSIFIER_FREQUENCY
#define AUDIO_SAMPLE_BIT    16
#define BYTES_PER_SAMPLE    (AUDIO_SAMPLE_BIT / 8)

// Audio buffer configuration  
#define PCM_BUF_COUNT       4
#define PCM_BUF_SIZE_MS     100
#define BLOCK_SIZE          ((AUDIO_FREQ * PCM_BUF_SIZE_MS * BYTES_PER_SAMPLE) / 1000)
#define READ_TIMEOUT        1000

static bool is_sampling = false;

// DMIC device
static const struct device *dmic_dev = DEVICE_DT_GET(DT_NODELABEL(pdm0));

// DMIC memory slab
K_MEM_SLAB_DEFINE_STATIC(dmic_rx_mem_slab, BLOCK_SIZE, PCM_BUF_COUNT, 4);

// Callback from inference engine (defined in inferencing.cpp)
extern "C" bool ei_samples_callback(const void *raw_sample, uint32_t raw_sample_size);

/**
 * @brief Initialize microphone interface
 */
bool ei_microphone_init(void)
{
    if (!device_is_ready(dmic_dev)) {
        ei_printf("ERR: DMIC device not ready\n");
        return false;
    }

    ei_printf("Microphone initialized (DMIC)\n");
    ei_printf("  Sample rate: %d Hz\n", AUDIO_FREQ);
    ei_printf("  Bit depth: %d\n", AUDIO_SAMPLE_BIT);

    return true;
}

/**
 * @brief Start audio sampling
 */
bool ei_microphone_start(void)
{
    if (is_sampling) {
        return true;
    }

    ei_printf("[MIC] Starting DMIC microphone...\n");

    // Configure DMIC
    struct pcm_stream_cfg stream = {
        .pcm_width = AUDIO_SAMPLE_BIT,
        .mem_slab = &dmic_rx_mem_slab,
    };

    struct dmic_cfg cfg = {
        .io = {
            .min_pdm_clk_freq = 1000000,   // 1 MHz
            .max_pdm_clk_freq = 3500000,   // 3.5 MHz
            .min_pdm_clk_dc = 40,
            .max_pdm_clk_dc = 60,
        },
        .streams = &stream,
        .channel = {
            .req_chan_map_lo = dmic_build_channel_map(0, 0, PDM_CHAN_LEFT),
            .req_num_chan = 1,
            .req_num_streams = 1,
        },
    };

    cfg.streams[0].pcm_rate = AUDIO_FREQ;
    cfg.streams[0].block_size = BLOCK_SIZE;

    int ret = dmic_configure(dmic_dev, &cfg);
    if (ret < 0) {
        ei_printf("ERR: Failed to configure DMIC: %d\n", ret);
        return false;
    }

    ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
    if (ret < 0) {
        ei_printf("ERR: Failed to start DMIC: %d\n", ret);
        return false;
    }

    is_sampling = true;
    
    ei_printf("[MIC] DMIC microphone started successfully\n");

    return true;
}

/**
 * @brief Stop audio sampling
 */
bool ei_microphone_stop(void)
{
    if (!is_sampling) {
        return true;
    }

    is_sampling = false;

    int ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
    if (ret < 0) {
        ei_printf("ERR: Failed to stop DMIC: %d\n", ret);
        return false;
    }

    ei_printf("Microphone sampling stopped\n");

    return true;
}

/**
 * @brief Sample audio data and call callback
 */
bool ei_microphone_sample(void)
{
    if (!is_sampling) {
        return false;
    }

    void *buffer = NULL;
    uint32_t size = 0;

    int ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
    if (ret < 0) {
        if (ret != -EAGAIN) {
            ei_printf("ERR: DMIC read failed: %d\n", ret);
        }
        return false;
    }
    
    if (size == 0 || buffer == NULL) {
        return false;
    }

    // DMIC gives us mono int16 samples
    int16_t *samples = (int16_t *)buffer;
    size_t sample_count = size / sizeof(int16_t);
    
    // Use fixed-size buffer
    const size_t max_samples = BLOCK_SIZE / sizeof(int16_t);
    static float float_buffer[BLOCK_SIZE / sizeof(int16_t)];
    
    size_t samples_to_process = (sample_count < max_samples) ? sample_count : max_samples;

    // Convert int16 to float and normalize to [-1.0, 1.0]
    for (size_t i = 0; i < samples_to_process; i++) {
        float_buffer[i] = (float)samples[i] / 32768.0f;
    }

    // Free the DMIC buffer
    k_mem_slab_free(&dmic_rx_mem_slab, buffer);

    // Call the inference callback with float samples
    ei_samples_callback(float_buffer, samples_to_process * sizeof(float));

    return true;
}

/**
 * @brief Check if sampling is active
 */
bool ei_microphone_is_sampling(void)
{
    return is_sampling;
}
