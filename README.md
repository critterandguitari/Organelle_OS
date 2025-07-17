# Organelle OS

This repository contains the core operating system software for the Critter & Guitari Organelle music computer. The Organelle OS provides a menu interface for patch management, hardware control, and system functions.

## Overview

The Organelle OS is a C++ application that serves as the bridge between the hardware controls (encoders, keys, knobs, OLED display) and the audio patches (e.g. Pd patches) running on the system. It provides:

- **Patch Management**: Browse, load, and organize patches
- **Hardware Interface**: Control encoders, buttons, knobs, keys, and OLED display
- **System Functions**: WiFi setup, MIDI configuration, storage management
- **OSC Communication**: Real-time communication with patches and system scripts

## Architecture

### Core Components

- **`main.cpp`**: Main application loop handling hardware events and OSC messages
- **`MainMenu.cpp/h`**: Menu navigation and patch loading system
- **`AppData.cpp/h`**: Global application state and configuration management
- **`OledScreen.cpp/h`**: OLED display rendering and graphics functions

### Hardware Abstraction

The system supports multiple hardware platforms through a hardware abstraction layer:

- **`hw_interfaces/`**: Platform-specific hardware interface implementations
  - `SerialMCU.h` - Original Organelle (serial communication with MCU)
  - `CM3GPIO.h` - Organelle M (direct GPIO on Raspberry Pi CM3)
  - `CM4OG4.h` - Organelle 4 (Raspberry Pi CM4)
  - `SDLPi.h` - SDL-based interface for development/testing

### System Scripts

- **`fw_dir/`**: Core system scripts and utilities
  - `scripts/` - System configuration scripts including many of the functions in the System menu
  - `mother.pd` - Main Pure Data patch coordinator
  - Configuration files and utilities

### Platform-Specific Files

- **`platforms/`**: Platform-specific configurations and overrides
  - `organelle/` - Original Organelle files
  - `organelle_m/` - Organelle M specific files  
  - `organelle_4/` - Organelle 4 specific files
  - Each contains `rootfs/` (system files) and `fw_dir/` (for script overrides)

## Build System

The project uses Make with platform-specific targets:

```bash
# Build for different platforms
make organelle      # Original Organelle
make organelle_m    # Organelle M  
make organelle_4    # Organelle 4
make sdlpi          # SDL development version

# Deploy to device (copies files to system locations)
make organelle_deploy
make organelle_m_deploy  
make organelle_4_deploy
```

### Build Configuration

Platform-specific features are controlled via preprocessor defines:

- `SERIAL_HW` - Use serial MCU communication
- `CM3GPIO_HW` - Use CM3 GPIO interface
- `CM4OG4_HW` - Use CM4/OG4 interface
- `BATTERY_METER` - Enable battery monitoring
- `MICSEL_SWITCH` - Enable mic/line input selection
- `PWR_SWITCH` - Enable power switch support

## Communication

### OSC (Open Sound Control)

The system communicates via OSC on multiple UDP ports:

- **Port 4001**: Receives messages (main application input)
- **Port 4000**: Sends to Pure Data patches
- **Port 4002**: Sends to auxiliary programs/scripts

Key OSC message types:

- `/oled/*` - Display control and graphics
- `/knobs` - Knob position data
- `/key` - Key press/release events
- `/encoder/*` - Encoder turn and button events
- System control messages for patch loading, configuration, etc.

### Hardware Events

The main loop continuously polls hardware and processes:

- Encoder rotation and button presses
- Key presses (25-key keyboard)
- Knob/potentiometer changes (6 analog inputs including footswitch / exprpession pedal input)

## File Structure

```
/
├── main.cpp              # Main application entry point
├── MainMenu.cpp/h        # Menu system and patch management
├── AppData.cpp/h         # Application state management
├── OledScreen.cpp/h      # Display rendering
├── hw_interfaces/        # Hardware abstraction layer
├── fw_dir/              # Core system scripts and files
├── platforms/           # Platform-specific configurations
├── OSC/                 # OSC message handling library
└── Makefile             # Build configuration
```

## Development

### Prerequisites

- C++11 compatible compiler
- Make build system
- Target platform SDK (for cross-compilation)

### Building for Development

The SDL interface allows development and testing on desktop systems:

```bash
make sdlpi
./fw_dir/mother
```

### Hardware Interface Development

To add support for new hardware:

1. Create new interface class in `hw_interfaces/`
2. Implement required methods: `init()`, `poll()`, `updateOLED()`, etc.
3. Add platform define and build target in Makefile
4. Create platform-specific files in `platforms/`


## Hardware Compatibility

- **Organelle (Original)**: Serial MCU communication
- **Organelle M**: Raspberry Pi CM3 with direct GPIO
- **Organelle 4**: Raspberry Pi CM4 with enhanced features
- Battery monitoring and power management on supported models
- Mic/Line input switching on supported models


