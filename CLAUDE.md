# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an Mbed OS embedded systems project for the STM32L4 IoT Discovery Kit (DISCO_L475VG_IOT01A target). The project demonstrates sensor integration and data acquisition using I2C-based environmental sensors (HTS221 for temperature/humidity and LPS22HB for pressure).

### Current Application
- **Main application**: Multi-sensor data acquisition demonstrating I2C communication
- **Target board**: DISCO_L475VG_IOT01A
- **RTOS**: Uses Mbed OS RTOS with threading support
- **Sensors**: HTS221 (temperature/humidity) and LPS22HB (pressure) via I2C bus on pins PB_11 (SDA) and PB_10 (SCL)

## Build System

This project uses **CMake with Mbed CLI 2** (requires Mbed OS >= 6.5).

### Common Build Commands

```bash
# Build the project for your target
mbed-tools compile -m DISCO_L475VG_IOT01A -t <TOOLCHAIN> -o verbose

# Build and flash to connected board
mbed-tools compile -m DISCO_L475VG_IOT01A -t <TOOLCHAIN> --flash

# Rebuild cleanly
mbed-tools compile -m DISCO_L475VG_IOT01A -t <TOOLCHAIN> --clean

# Build verbosely (pass VERBOSE_BUILD=ON to CMake)
mbed-tools compile -m DISCO_L475VG_IOT01A -t <TOOLCHAIN> -c
```

**Supported toolchains**: ARM (default), GCC_ARM, IAR (check Mbed OS docs for availability)

**Build artifacts** (Mbed CLI 2):
- Output binary: `./cmake_build/DISCO_L475VG_IOT01A/develop/<TOOLCHAIN>/`

## Project Structure

```
.
├── main.cpp / Temp&HumidityDetection.cpp   # Application entry point
├── HTS221/                                  # HTS221 sensor library
├── LPS22HB/                                 # LPS22HB pressure sensor library
├── mbed-os/                                 # Mbed OS kernel and HAL
├── CMakeLists.txt                          # Build configuration
├── mbed_app.json                           # Mbed configuration (printf settings)
└── .mbed                                    # Project metadata (target, serial)
```

## Architecture Notes

### Sensor Integration Pattern
The project uses a simple polling model:
1. **Device initialization**: Create DevI2C instances with explicit pin assignments (PB_11=SDA, PB_10=SCL)
2. **Sensor drivers**: Use mbed-provided sensor abstractions (HTS221Sensor, etc.) that wrap HAL calls
3. **Main loop**: Infinite polling loop with sleep intervals between sensor reads

### Key Files and Their Roles
- **main.cpp**: Contains application logic, sensor initialization, and main polling loop
- **mbed-os/**: The Mbed OS kernel - contains RTOS, device HAL, and driver abstractions (do not modify unless debugging core issues)
- **CMakeLists.txt**: Links application sources to mbed-os and configures post-build steps

### Configuration (mbed_app.json)
The `mbed_app.json` configures:
- Printf behavior (uses standard C lib for performance)
- Floating-point printf disabled by default (preserved for embedded systems)

## Development Workflow

### Adding a New Sensor or Peripheral
1. Reference the sensor library URL in a `.lib` file (format: URL pointing to mbed-os team code)
2. Add sensor header includes in your source
3. Initialize via DevI2C or appropriate hardware interface
4. Integrate into the main polling loop

### Debugging
- Use Serial communication (UART) - print debug info via `printf()`
- Connect USB to serial console to observe output
- The `.mbed` file contains target serial and code information for flashing

### Modifying I2C Pins
If changing I2C pins from PB_11/PB_10:
1. Update DevI2C constructor pins in source code
2. Verify pins are valid on target (check DISCO_L475VG_IOT01A datasheet)
3. Rebuild and retest

## Dependencies

- **Mbed CLI 2**: Build tool (installation required)
- **Mbed OS**: Submodule in `mbed-os/`
- **Arm GCC Embedded** or **IAR Embedded Workbench**: Compiler toolchain
- **Sensor libraries**: HTS221 and LPS22HB (managed as `.lib` references)

All Mbed dependencies are managed through `.lib` files and git submodules.

## Target Information

- **Board**: DISCO_L475VG_IOT01A (STM32L475 + X-NUCLEO expansion)
- **MCU**: STM32L475VGT6 (ARM Cortex-M4 with FPU)
- **RTOS support**: Full RTOS with threading (required for this example)
