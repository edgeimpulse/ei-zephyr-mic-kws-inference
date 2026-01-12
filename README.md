# Edge Impulse Microphone Inference on Zephyr

This repository demonstrates how to run microphone-based Edge Impulse models on Zephyr using the **Edge Impulse Zephyr Module**.  
Drop in your model > build > flash > get real-time audio inference.

## Initialize This Repo

```bash
 west init https://github.com/edgeimpulse/ei-zephyr-mic-kws-inference.git
cd ei-zephyr-mic-kws-inference
west update
```

This fetches:
- Zephyr RTOS
- Edge Impulse Zephyr SDK module
- All module dependencies

## Update the Model

In Edge Impulse Studio go to:  
**Deployment** > **Zephyr library** > **Build**

Download the generated `.zip`

Extract into the `model/` folder:

```bash
unzip -o ~/Downloads/your-model.zip -d model/
```

Your `model/` directory should contain:
- `CMakeLists.txt`
- `edge-impulse-sdk/`
- `model-parameters/`
- `tflite-model/`

## Build

Choose your board (Example: Nordic thingy53):

```bash
west build --pristine -b thingy53/nrf5340/cpuapp
```

## Flash

```bash
west flash
```

Or specify runner:

```bash
west flash --runner jlink
west flash --runner nrfjprog
west flash --runner openocd
```

## Microphones Supported

PDM microphones accessible through Zephyr's DMIC driver are compatible.

## Shield supported

The example has been tested with the [X-NUCLEO-IKS02A1](https://www.st.com/en/evaluation-tools/x-nucleo-iks02a1.html) shield.

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Zephyr config
├── west.yml                # Manifest (declares Edge Impulse SDK module)
├── model/                  # Your Edge Impulse model (Zephyr library)
└── src/
    ├── main.cpp
    ├── inference/          # Inference state machine
    └── microphone/         # Microphone interface
```

## How It Works

1. **Initialize** - Microphone setup via Zephyr DMIC API
2. **Sample** - Continuous audio data collection at model frequency
3. **Buffer** - Circular buffer stores audio samples
4. **Infer** - Run classifier when buffer full
5. **Output** - Print classification results
6. **Loop** - Repeat

## Configuration

Key settings in `prj.conf`:

```properties
CONFIG_MAIN_STACK_SIZE=8192      # Adjust for your model size
CONFIG_AUDIO=y                   # Enable audio subsystem
CONFIG_AUDIO_DMIC=y              # Enable DMIC for PDM microphones
```

For larger models, increase stack size:

```properties
CONFIG_MAIN_STACK_SIZE=16384
```

## Using in Your Own Project

Add to your `west.yml`:

```yaml
projects:
  - name: edge-impulse-sdk-zephyr
    path: modules/edge-impulse-sdk-zephyr
    revision: v1.75.4  # See https://github.com/edgeimpulse/edge-impulse-sdk-zephyr/tags
    url: https://github.com/edgeimpulse/edge-impulse-sdk-zephyr
```

Then:

```bash
west update
```

Add to your `CMakeLists.txt`:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/model)
```

## Troubleshooting

**Module not found**
```bash
west update
```

**Insufficient memory**
```properties
# In prj.conf
CONFIG_MAIN_STACK_SIZE=16384
```

**Sensor not detected**
```properties
# In prj.conf - enable debug logging
CONFIG_I2C_LOG_LEVEL_DBG=y
CONFIG_SENSOR_LOG_LEVEL_DBG=y
```

## Resources


- [Edge Impulse SDK for Zephyr](https://github.com/edgeimpulse/edge-impulse-sdk-zephyr)



Clear BSD License - see `LICENSE` file  
Copyright (c) 2025 EdgeImpulse Inc.
