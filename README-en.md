# ESP32 Desktop Drumkit v0 Prototype

> **Languages/语言**: [English](README-en.md) | [简体中文](README-zh-CN.md)

This is the English version of the README.

## Project Overview

This is an ESP32-based electronic drum pad prototype project supporting both BLE MIDI and serial MIDI output. The project includes multi-channel piezo drum pad/cymbal sensing, velocity normalization, dual-mode output (serial & BLE), and battery management.  
This project is suitable for DIY music enthusiasts, electronic drum developers, and embedded system learners for reference and further development.

---

## Features

- **Multi-Pad Sensing**: Supports multiple piezo sensors (drum pads, cymbals), each with independent threshold settings.
- **MIDI Output**:
  - **BLE MIDI** (wireless connection to phones/computers)
  - **Serial MIDI** (connects to PC via CH340/USB-to-serial, supported by Hairless MIDI bridge software)
- **Velocity Normalization**: Maps detected strike intensity to standard MIDI velocity (1-127).
- **Anti-Crosstalk Algorithm**: Only allows the pad with the highest ADC value to trigger during each sampling period, with silent periods for other pads to prevent false triggers.
- **Battery Voltage Management**: Supports battery monitoring and low-power alerts.
- **LED/Buzzer Indicators**: Mode switching and status feedback.
- **One-Button Mode Switching**: Toggle between BLE MIDI and Serial MIDI modes.
- **Enclosure Design**: Modular design for easy expansion.

---

## Hardware Specifications

- **Main Controller**: ESP32 (ESP32-S3 recommended for native USB support)
- **Piezo Elements**: ≥42mm for drum pads, 27mm for cymbals; *strongly* recommend shielded signal wires with parallel 0.1μF~1μF ceramic capacitors
- **Serial Chip**: CH340 (or other USB-UART converters)
- **Battery**: Optional lithium battery with voltage monitoring circuit
- **Others**: LEDs, buzzer, buttons - see `def.h` for peripheral interfaces

---

## Software Specifications

- **PlatformIO/Arduino compatible**, PlatformIO with Arduino framework recommended
- **Dependencies**:
  - `BLEMIDI_Transport` / `hardware/BLEMIDI_ESP32`
  - `OneButtonTiny` (button detection)
  - See source code headers for others

---

## Usage Instructions

### 1. Hardware Setup

- Connect piezo elements to GPIO as defined in `def.h`
- Connect serial TX/RX to computer via CH340 (recognized as COM port)
- BLE MIDI supports phone/computer Bluetooth connections
- Button for mode switching

### 2. Compilation & Flashing

- Using PlatformIO: `pio run -t upload`
- Or Arduino IDE: select correct board and port

### 3. MIDI Output

#### Serial MIDI

- Open [Hairless MIDI <-> Serial Bridge](https://projectgus.github.io/hairless-midiserial/) or [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html)
- Set baud rate to 115200 (matches code)
- After bridging, DAW/audio software can recognize virtual MIDI port

#### BLE MIDI

- Connect Bluetooth device to "ESP32 Desktop DrumKit v1"
- DAW or mobile MIDI apps can directly recognize

### 4. Anti-Interference Notes

- Only the pad with highest ADC value triggers per sampling period, others enter silent period (software anti-crosstalk)
- Independent threshold settings for pads/cymbals (see `Pad` constructor parameters)
- For extreme false triggers, adjust thresholds, silent periods, or hardware voltage dividers/capacitors

---

## Code Structure

- `main.cpp`: Core logic, pad management, MIDI output and mode switching
- `pad.h` / `pad.cpp`: Pad object definition, sampling and trigger detection
- `def.h`: Hardware pin definitions, MIDI note mapping
- `OneButtonTiny.h` / `.cpp`: Button library
- `battery.h` / `.cpp`: Battery management
- Other supporting files

---

## Common Issues

1. **False triggers/missed triggers/multiple pads sounding**
   - Check hardware voltage dividers/capacitors/shielding
   - Increase pad thresholds, decrease cymbal thresholds
   - Software already implements max-ADC-value triggering + silent period protection

  *As this is my first instrument project, the hardware design didn't properly account for vibration transmission through piezo elements, leading to insensitive pads. The next version v1 will incorporate proper electronic drum structural design.*

2. **Serial MIDI not recognized as MIDI device by PC**  
   - Use Hairless MIDI or loopMIDI for serial-MIDI bridging

---

## Contribution/Customization Suggestions

- Modify `Pad` constructor parameters for finer per-pad threshold control
- Add more pads or implement double-tap/roll detection
- For native USB MIDI (no serial bridge), consider ESP32-S3 with TinyUSB library
- PRs and issues welcome

---

## Acknowledgments

- Thanks to all open-source library authors and the electronic drum DIY community
- Inspired by [Victor2805/Homemade-electronic-drum-kit-with-arduino](https://github.com/Victor2805/Homemade-electronic-drum-kit-with-arduino)
- Special thanks to my percussion-loving and DAW-expert cousin @[Woodbreeze](https://github.com/WoodBreeze) ;)
---

**WilliTourt / 2025.9 Prototype Release**
