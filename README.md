# Edge Impulse IMU Inference on Zephyr

This repository demonstrates how to run IMU-based Edge Impulse models on Zephyr using the **Edge Impulse Zephyr Module**.  
Drop in your model > build > flash > get real-time motion inference.

## Initialize This Repo

```bash
git clone https://github.com/edgeimpulse/ei-zephyr-imu-inference.git
cd ei-zephyr-imu-inference
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

Choose your board (Example: Nucleo U585ZI):

```bash
west build --pristine -b nucleo_u585zi_q
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

## Sensors Supported

The firmware supports any IMU exposed through Zephyr's sensor API, including:

| IMU | Zephyr Driver |
|-----|---------------|
| IIS2DLPC | `CONFIG_IIS2DLPC=y` |
| LSM6DSOX / LSM6DSO | `CONFIG_LSM6DSOX=y` |
| ISM330DHCX | `CONFIG_ISM330DHCX=y` |
| BMI270 | `CONFIG_BMI270=y` |
| ICM42688 | `CONFIG_ICM42688=y` |

All sensors accessible through I²C/SPI + Zephyr sensor drivers are compatible.

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Zephyr config
├── west.yml                # Manifest (declares Edge Impulse SDK module)
├── model/                  # Your Edge Impulse model (Zephyr library)
└── src/
    ├── main.cpp
    ├── inference/          # Inference state machine
    └── sensors/            # IMU interface
```

## How It Works

1. **Initialize** - Sensor setup via Zephyr sensor API
2. **Sample** - Continuous data collection at model frequency
3. **Buffer** - Circular buffer stores samples
4. **Infer** - Run classifier when buffer full
5. **Output** - Print classification results
6. **Loop** - Repeat

## Configuration

Key settings in `prj.conf`:

```properties
CONFIG_EDGE_IMPULSE_SDK=y        # Enable Edge Impulse SDK
CONFIG_MAIN_STACK_SIZE=8192      # Adjust for your model size
CONFIG_SENSOR=y                  # Enable sensor subsystem
CONFIG_I2C=y                     # Enable I2C for sensors
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
