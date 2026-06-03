# ESP32 Motion Rep Counter

Portable exercise tracker based on ESP32-C3 for abdominal repetition counting and plank timing using onboard IMU sensors.

The project also includes a custom 3D-printed enclosure designed for wearable or portable use.

## Main Goals

- Count abdominal repetitions
- Measure plank duration
- Detect body movement using IMU data
- Provide visual feedback through onboard LEDs
- Operate from Li-Ion battery
- Fit inside a custom 3D-printed enclosure

## Hardware

- ESP32-C3-DevKit-RUST-2
- ESP32-C3-MINI-1
- ICM-42670-P IMU
- SHTC3 temperature/humidity sensor
- WS2812 RGB LED
- USB-C interface
- Li-Ion battery support

## Features

- Abdominal repetition counting
- Plank timer
- Motion detection using IMU
- Real-time feedback
- Serial monitor debugging
- Low power operation
- Future BLE support

## Development Environment

- ESP-IDF
- FreeRTOS
- VS Code
- Linux

## Project Structure

```text
firmware/   -> ESP-IDF source code
docs/       -> schematics, datasheets and notes
hardware/   -> hardware-related files
tests/      -> test applications
```

## Planned Features

- Automatic exercise detection
- Calibration mode
- OLED/TFT support
- BLE smartphone integration
- Workout statistics
- Exercise history
- Battery monitoring

## Board Information

ESP32-C3-DevKit-RUST-2 v1.3a (05/2025)

| Signal | GPIO |
|---|---|
| SDA | GPIO7 |
| SCL | GPIO8 |

Onboard I2C devices:

| Address | Device |
|---|---|
| 0x68 | ICM-42670-P IMU |
| 0x70 | SHTC3 temperature/humidity sensor |

## License

MIT License