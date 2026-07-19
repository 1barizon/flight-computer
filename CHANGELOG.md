# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Fixed

- **Buffer overflow fix in GPSModule** (`GPSModule.cpp:59,66`):
  - Replaced `sprintf()` with `snprintf()` in `getTimeString()` and `getDateString()`
  - Eliminates risk of buffer overflow with malformed GPS data

- **Sensor fallback on corrupted readings** (`BMP585Sensor.cpp`):
  - Added NaN and range validation (`-500` to `50000` m) on altitude reads
  - Invalid readings silently discarded — last known good values preserved
  - Prevents NaN propagation to FSM from transient sensor glitches

### Changed

- **Private member naming aligned with v2.0 convention** (`BMP585Sensor`, `LSM6DS3Sensor`):
  - All private members renamed from `snake_case` to `_camelCase` per AGENTS.md
  - Affects `_basePressure`, `_altitude`, `_prevAltitude`, `_accelX`, `_totalAccel`, etc.
  - `smoothFilter()` changed to `static` (no instance state dependency)

### Documentation

- Fixed critical pin table mismatch in `docs/hardware.md` (was showing C3 pins, now matches S3 config.h)
- Rewrote `docs/flowchart.md` from v1.0 to v2.0 (FreeRTOS tasks + 4-state FSM)
- Rewrote `CONTRIBUTING.md` for v2.0 (S3 target, OOP, FreeRTOS patterns)
- Updated `AGENTS.md` — v2.0 status "in planning" → "implemented", 868E6 → 915E6
- Updated `REFACTORING_PLAN.md` — all 10 phases marked complete
- Cleaned up dead v1.0 constants from `config.h` (`INTERVAL`, `ALTITUDE_THRESHOLD`, `VELOCITY_THRESHOLD`, `ALTITUDE_DROP_THRESHOLD`)

---

## [2.0.0] - 2026-07-19

### Added

- **Phase 1-2: Project structure and base interfaces**
  - `firmware/sensors/ISensor.h` — Abstract interface for all sensors (BMP585, LSM6DS3, GPS)
  - `firmware/flight/SensorData.h` — Shared data structures (SensorData, LogMessage)
  - Sensor abstraction layer enabling polymorphic sensor implementations
  - FreeRTOS-compatible data structures for inter-task communication via queues
  - Complete Doxygen documentation for all interfaces

- **Phase 3: BMP585Sensor class**
  - Barometric altitude, pressure, temperature reading
  - Vertical velocity (Vz) via numerical differentiation, clipped to ±200 m/s
  - NaN/Inf and range validation with fallback to last known good value
  - IIR low-pass filter (alpha=0.2) for accelerometer data

- **Phase 4: LSM6DS3Sensor class**
  - 6-axis accelerometer + gyroscope with safety validations
  - NaN/Inf rejection and range checking (±200 m/s² accel, ±2000 °/s gyro)
  - Vector accessors via pointer parameters (`getAcceleration`, `getGyroscope`)

- **Phase 5: GPSModule class**
  - Non-blocking NMEA parsing via TinyGPS++ at 5Hz
  - HardwareSerial dependency injection for testability
  - GPS time used for CSV file naming (NOFIX fallback)

- **Phase 6: 4-state Flight State Machine**
  - IDLE → ASCENT → DESCENT → LANDED with sub-event flags (liftoff, burnout, apogee, freefall, parachute)
  - Thresholds validated against real flight data (1,873 points from `13_30_11-Dados.csv`)
  - IIR filter with seed on first reading (avoids transient)
  - Parachute deploy with multi-cycle confirmation (3 cycles of stable negative Vz)
  - Ground guard (50m) preventing deployment near terrain

- **Phase 7: FreeRTOS multi-task architecture**
  - **FlightControlTask** (Core 1, Priority 20, 50Hz): sensor reads + FSM + parachute + watchdog
  - **TelemetryTask** (Core 0, Priority 5, 5Hz): GPS + queue drain + LoRa + file logging
  - **LoggerTask** (Core 0, Priority 1): async log queue with level filtering
  - Queue-based inter-task communication (no shared variables)
  - ESP32 TWDT armed inside the running task (not in init)
  - Performance metrics (cycle count, overruns, queue drops, exec time)

- **Phase 8: firmware.ino integration**
  - `setup()` delegates to `initFlightControlTask()` / `initTelemetryTask()` / `initLoggerTask()`
  - Safe-hold on critical failure (infinite loop + buzzer) — no `ESP.restart()`
  - `loop()` blocks on `vTaskDelay(portMAX_DELAY)`; all work in FreeRTOS tasks

- **Phase 9: Module adaptation**
  - `parachute_module` — Servo actuator (one-shot, idempotent deploy)
  - `lora_module` — RFM95W at 915 MHz, SF7, BW125k, CR4/5, CRC enabled
  - `filesystem_module` — SD card primary + LittleFS fallback, transparent dispatch
  - `buzzer_module` — Status tones (init success/failure)

- **Phase 10: Telemetry format (22 fields)**
  - CSV format aligned with `recovery-webui` receiver: `TEAM_ID,millis,count,altp,temp,umi,p,gx,gy,gz,ax,ay,az,vz,maxAltitude,state,alt,lat,lon,sat,parachute,rssi`
  - Single `snprintf` call (no heap fragmentation)
  - GPS enrichment in TelemetryTask (not FlightControlTask)

### Changed

- Migrated from v1.0 procedural architecture to v2.0 Object-Oriented design
- Restructured firmware directory: `sensors/`, `flight/`, `modules/`
- **FSM simplified to 4 main states** with internal event flags (was 7-state model)
- Replaced BMP280 with BMP585 (I2C, improved accuracy)
- Replaced MPU6050 with LSM6DS3 (6-axis, better range)
- Replaced NEO-6M GPS with NEO-8M (multi-constellation)
- Removed WiFi AP + Web server (v1.0 feature, not needed in v2.0)
- Updated `docs/software.md` with new module structure and component mapping
- Pin table in `docs/hardware.md` updated for ESP32-S3 pinout
- `docs/flowchart.md` rewritten for FreeRTOS + 4-state FSM
- `CONTRIBUTING.md` rewritten for v2.0 workflows

### Fixed

- **Critical Safety Initializations** (commit 4b0c239):
  - All SensorData struct fields now have safe default values
  - LogMessage buffer initialized with zero-terminator
  - Prevents undefined behavior from uninitialized variables
  - Ensures parachute_deployed flag cannot be random on startup
  - Protects against NaN propagation in FSM transitions

### Documentation Added

- `docs/adr/002-sensor-abstraction.md` — Architectural Decision Record for ISensor interface

---

## [1.0.0] - 2026-01-27

### Added

- Initial project structure
- Base firmware for ESP32-C3 Super Mini
- Altitude monitoring system (MPU6050 sensor)
- GPS integration
- LoRa communication with operational base
- Telemetry storage in LittleFS
- Web interface for data access
- Component unit tests
- Software and hardware documentation

### Release Notes

- First functional version of the onboard computer
- Parachute deployment system still under testing
- Sensor calibration required before flight

---

## Versioning Guide

### MAJOR (X.0.0)

- Incompatible changes to firmware API or data structure

### MINOR (0.X.0)

- New features backward compatible with previous version
- Functionality improvements

### PATCH (0.0.X)

- Bug fixes
- Performance optimizations
- Documentation updates

## How to Report Changes

When making commits or pull requests, use the following categories:

- `feat:` for new features
- `fix:` for bug fixes
- `docs:` for documentation
- `test:` for tests
- `refactor:` for code refactoring
- `perf:` for performance improvements
- `chore:` for maintenance tasks

Example: `feat: add temperature sensor`
