# Avionics - Onboard Computer

> **V2 Flight Computer** for Serra Rocketry (#11).
>
> The previous V1 architecture (LASC 2025) has reached End of Life and is
> preserved via tagged release.

## What it does

Real-time avionics firmware for a sounding rocket. Runs on an **ESP32-S3**
(the v2.0 target platform; the ESP32-C3 SuperMini was used for the prototype
firmware) under FreeRTOS, and is responsible for:

- Reading altitude (BMP585), IMU (LSM6DS3) and GPS (NEO-8M)
- Running a flight state machine (liftoff → burnout → apogee → freefall →
  parachute → landed)
- Deploying the parachute at **apogee** (validated against RocketPy + real
  flight data)
- Transmitting telemetry over **LoRa 915 MHz** to the ground receiver
  (`recovery-webui/components/receiver-lora`)
- Logging telemetry to LittleFS for post-flight recovery

## Architecture

| Layer | Path | Description |
|-------|------|-------------|
| Sensors | `firmware/sensors/` | OOP `ISensor` implementations (BMP585, LSM6DS3, GPS) |
| Modules | `firmware/modules/` | Actuators/peripherals (parachute, LoRa, buzzer, filesystem) |
| Flight | `firmware/flight/` | FreeRTOS tasks + `FlightStateMachine` |
| Config | `firmware/config.h` | Pins, thresholds, radio parameters |

Two FreeRTOS cores:

- **Core 1 (critical)**: `FlightControlTask` @ 50 Hz — sensors, FSM, parachute.
- **Core 0 (non-critical)**: `TelemetryTask` @ 5 Hz (Serial + LoRa + file) and
  `LoggerTask` (low priority).

See [`docs/software.md`](docs/software.md) and
[`docs/hardware.md`](docs/hardware.md) for the full specification. The
telemetry wire format is the single source of truth in
[`docs/telemetry-format.md`](docs/telemetry-format.md).

## Build & Flash

**Arduino IDE** (recommended):

1. Board: `ESP32-C3 Dev Module`
2. Open `firmware/firmware.ino`
3. Compile (`Ctrl/Cmd+R`) and upload (`Ctrl/Cmd+U`)

**PlatformIO** (alternative):

```bash
platformio run -e esp32-c3          # build
platformio run -e esp32-c3 -t upload  # flash
```

## Validate without hardware

```bash
python3 extras/FSM_tester/FSM_Tester.py          # FSM against real flight data
python3 extras/validate_telemetry_format.py      # telemetry v2.0 indices (flight↔receiver)
```

## Repository layout

```
firmware/   v2.0 firmware (.ino + OOP modules, tasks, FSM)
test/       hardware validation sketches (bmp/lora/gps/servo/FSM)
docs/       software.md, hardware.md, telemetry-format.md, flowchart.md
hardware/   KiCad schematic + BOM
extras/     scripts, FSM tester, format validator
```

## Status

All refactoring phases (1-10) are **complete**. Current work is around
documentation sync and receiver integration (see open issues).

## License

Serra Rocketry. Contributions under the project license — see `CONTRIBUTING.md`.
