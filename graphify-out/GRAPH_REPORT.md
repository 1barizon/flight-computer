# Graph Report - flight-computer  (2026-11-18)

## Corpus Check
- 68 files · ~81,380 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 60 file(s) not represented in the graph (top: (none) 9, .ino 9, .zip 7)

## Summary
- 777 nodes · 1180 edges · 48 communities (43 shown, 5 thin omitted)
- Extraction: 94% EXTRACTED · 6% INFERRED · 0% AMBIGUOUS · INFERRED: 66 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- Project Documentation
- FSM Analysis Tools
- Flight State Machine
- Flight Simulation
- Arming Persistence
- GPS Sensor Module
- Barometer Sensor
- Flight Event Detection
- Sensor Data Model
- IMU Sensor Module
- FSM Robustness Tests
- FSM Validation Tests
- OpenCode Workflows
- Serial Flight Insertion
- Barometric Modeling
- Barometer Implementation
- IMU Implementation
- Logger and LoRa
- Flight Control Task
- FSM Trigger Utilities
- FSM Scenario Validation
- Telemetry Task
- System Overview
- Low-Rate FSM Validation
- Filesystem Storage
- Telemetry Protocol
- Flight Architecture
- FSM State Persistence
- Hardware Modules
- Freefall Analysis
- Flight Event Concepts
- Hardware Diagnostics
- Servo and Buzzer
- Servo Timing Analysis
- Telemetry Format Tests
- State Transition Logic
- NVS Snapshot
- Architecture Release Docs
- Vertical Velocity Analysis
- Servo Load Testing
- Log Messages
- Recovery Architecture
- Logger Statistics
- FreeRTOS Task IDs
- Flight Telemetry Records
- Mechanical CAD
- Sensor Updates

## God Nodes (most connected - your core abstractions)
1. `FlightStateMachine` - 58 edges
2. `BMP585Sensor` - 35 edges
3. `SensorData` - 27 edges
4. `LSM6DS3Sensor` - 22 edges
5. `GPSModule` - 18 edges
6. `FSM` - 17 edges
7. `FSM` - 12 edges
8. `FlightSim` - 11 edges
9. `ISensor` - 11 edges
10. `main()` - 10 edges

## Surprising Connections (you probably didn't know these)
- `5 Hz flight loop fix` --semantically_similar_to--> `5 Hz flight loop tuning parameter`  [EXTRACTED] [semantically similar]
  CHANGELOG.md → docs/architecture.md
- `Electronic failure risk assessment` --rationale_for--> `Safety-critical aerospace firmware`  [EXTRACTED]
  docs/extracao-eletronica-telemetria-payload.md → AGENTS.md
- `Four-state flight state machine` --semantically_similar_to--> `IDLE ASCENT DESCENT LANDED FSM`  [EXTRACTED] [semantically similar]
  README.md → docs/architecture.md
- `taskTelemetry()` --calls--> `buzzRecoveryBeep()`  [INFERRED]
  firmware/TelemetryTask.cpp → firmware/modules/buzzer_module.h
- `Flight Computer Development Guide` --references--> `Flight Computer v2.0 architecture specification`  [EXTRACTED]
  AGENTS.md → docs/architecture.md

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Participants in new feature workflow** — _opencode_skills_embedded_architect_skill, _opencode_skills_firmware_developer_skill, _opencode_skills_fsm_specialist_skill, _opencode_skills_test_engineer_skill, _opencode_skills_documentation_specialist_skill, _opencode_skills_code_reviewer_skill [EXTRACTED 1.00]
- **Participants in fast bug-fix workflow** — _opencode_skills_fsm_specialist_skill, _opencode_skills_firmware_developer_skill, _opencode_skills_test_engineer_skill, _opencode_skills_code_reviewer_skill, _opencode_skills_documentation_specialist_skill [EXTRACTED 1.00]
- **Participants in architectural refactoring workflow** — _opencode_skills_embedded_architect_skill, _opencode_skills_firmware_developer_skill, _opencode_skills_test_engineer_skill, _opencode_skills_code_reviewer_skill, _opencode_skills_documentation_specialist_skill [EXTRACTED 1.00]
- **v2.0 safety-critical flight architecture** — docs_architecture_four_state_fsm, docs_architecture_option_a_parachute_deploy, docs_architecture_freefall_backstop, docs_architecture_nvs_watchdog_recovery [EXTRACTED 1.00]
- **Telemetry acquisition and delivery pipeline** — docs_software_task_telemetry, docs_telemetry_format_satellite_packet, docs_hardware_rfm95w_lora, docs_hardware_microsd_storage, docs_telemetry_format_receiver_packet [EXTRACTED 1.00]
- **Flight state transition chain** — extras_fsm_tester_explicacao_liftoff_detection, extras_fsm_tester_explicacao_motor_burnout_detection, extras_fsm_tester_explicacao_apogee_detection, extras_fsm_tester_explicacao_freefall_detection, extras_fsm_tester_explicacao_parachute_altitude_trigger [EXTRACTED 1.00]
- **Bench validation stack** — test_bench_readme_bench_self_test, test_bench_readme_gps_uart_wiring, test_bench_readme_lora_rfm95w, test_bench_readme_parachute_door_servo [EXTRACTED 1.00]
- **Deploy mechanism approval gate** — test_parachute_servo_readme_real_load_servo_mechanism_test, test_parachute_servo_readme_bench_microswitch_fixture, test_parachute_servo_readme_bec_rail_stall_current_limit, test_parachute_servo_readme_twenty_trial_pass_fail_criteria [EXTRACTED 1.00]
- **Comparison of native telemetry, firmware field, and 5 Hz simulated vz** — flight_records_thonyan_flight_vz_comparison_native_telemetry_trace, flight_records_thonyan_flight_vz_comparison_firmware_trace, flight_records_thonyan_flight_vz_comparison_simulated_5hz_trace [EXTRACTED 1.00]

## Communities (48 total, 5 thin omitted)

### Community 0 - "Project Documentation"
Cohesion: 0.05
Nodes (58): Aligned v2.0 architecture, Cross-reference integration opportunity, Consistency Analysis, Aligned naming conventions, Shared reference documents, Shared safety-critical principles, Six validated specialists, OpenCode project configuration (+50 more)

### Community 1 - "FSM Analysis Tools"
Cohesion: 0.09
Nodes (21): analyze_flight(), check_bench(), fmt_stats(), load_csv(), main(), noise_context(), resample_50hz(), run_fsm_50hz() (+13 more)

### Community 2 - "Flight State Machine"
Cohesion: 0.05
Nodes (28): FlightStateMachine, _altAboveCount, _apogeeDetected, _baro, _burnoutDetected, _filtAx, _filtAy, _filtAz (+20 more)

### Community 3 - "Flight Simulation"
Cohesion: 0.10
Nodes (15): BaroStaleContingency, check_frozen_case(), compute_vz_list(), find_freeze_index(), load(), run_alive(), run_bench(), run_frozen() (+7 more)

### Community 4 - "Arming Persistence"
Cohesion: 0.10
Nodes (13): ArmingFSM, _deploy_ok(), _f(), FlightSim, load(), pad_only(), run_dataset(), scenario_a() (+5 more)

### Community 5 - "GPS Sensor Module"
Cohesion: 0.07
Nodes (9): GPSModule::getData(), GPSModule::getDateString(), GPSModule::getTimeString(), GPSModule::GPSModule(), GPSModule, _gps, _ready, _serial (+1 more)

### Community 6 - "Barometer Sensor"
Cohesion: 0.07
Nodes (13): BMP585Sensor, _altitude, _basePressure, _bmp, _bmp280, _maxAltitude, _pressure, _prevAltitude (+5 more)

### Community 7 - "Flight Event Detection"
Cohesion: 0.09
Nodes (3): FlightStateMachine::detectLiftoffTimed(), FlightStateMachine::getData(), FlightStateMachine::restoreFromNVS()

### Community 8 - "Sensor Data Model"
Cohesion: 0.09
Nodes (22): SensorData, accelX, accelY, accelZ, altitude, gps_valid, gpsAltitude, gyroX (+14 more)

### Community 9 - "IMU Sensor Module"
Cohesion: 0.10
Nodes (11): FlightStateMachine::FlightStateMachine(), LSM6DS3Sensor, _accelX, _accelY, _accelZ, _gyroX, _gyroY, _gyroZ (+3 more)

### Community 10 - "FSM Robustness Tests"
Cohesion: 0.15
Nodes (10): classify(), inject_wobble(), load(), main(), max_tolerated_amp(), outcome(), recommendation(), run_fsm() (+2 more)

### Community 11 - "FSM Validation Tests"
Cohesion: 0.15
Nodes (9): FSM, high_apogee_flight(), run(), sensor_accel(), sensor_accel_high(), smooth(), soil_vibration_test(), synthetic_flight() (+1 more)

### Community 12 - "OpenCode Workflows"
Cohesion: 0.15
Nodes (19): Configured technology stack, OpenCode workflow definitions, Bug fix workflow, Multi-layer code review checklist, OpenCode Skills README, Flight Computer v2.0 architecture, New feature workflow, Real flight telemetry with 1873 points (+11 more)

### Community 13 - "Serial Flight Insertion"
Cohesion: 0.22
Nodes (7): main(), open_serial(), parse_arguments(), read_csv(), send_data(), serial_reader(), setup_logging()

### Community 14 - "Barometric Modeling"
Cohesion: 0.14
Nodes (7): BaroModel, calibrate_base(), load_13_30(), load_filtrados(), load_resampled_50hz(), resample_50hz_generic(), load_filtrados()

### Community 17 - "Logger and LoRa"
Cohesion: 0.17
Nodes (7): getLogLevelName(), taskLogger(), isLoRaAvailable(), loraAvailabilityRef(), markLoRaAvailable(), sendLoRa(), setupLoRa()

### Community 18 - "Flight Control Task"
Cohesion: 0.23
Nodes (11): beginSensorWithRetry(), buildSensorData(), checkAutoRezero(), checkBaroGlitch(), checkBaroStaleContingency(), checkFreefallBackstop(), deployParachute(), handleArmCommand() (+3 more)

### Community 19 - "FSM Trigger Utilities"
Cohesion: 0.16
Nodes (5): altitude_trigger_factory(), detect_apogee_acceleration(), detect_freefall(), detect_liftoff(), detect_motor_burnout()

### Community 20 - "FSM Scenario Validation"
Cohesion: 0.15
Nodes (7): load_13_30(), load_resampled_50hz(), run_backstop(), run_fsm(), synth_drone_drop(), synth_spike(), total()

### Community 21 - "Telemetry Task"
Cohesion: 0.26
Nodes (4): assembleTelemetry(), buildDataFilePath(), formatForSerial(), taskTelemetry()

### Community 22 - "System Overview"
Cohesion: 0.17
Nodes (13): ESP32-C3 SuperMini legacy platform, ESP32-S3 primary platform, Flight Computer Development Guide, FSM Python simulator validation, v2.0 OOP + FreeRTOS + FSM architecture, Safety-critical aerospace firmware, Emergency deploy board, Sensor validation and last-good-value fallback (+5 more)

### Community 23 - "Low-Rate FSM Validation"
Cohesion: 0.22
Nodes (6): liftoff_accum_last_true(), load_csv(), main(), resample_5hz(), run_fsm_5hz(), smooth()

### Community 24 - "Filesystem Storage"
Cohesion: 0.21
Nodes (7): appendFile(), getStorageName(), getStorageType(), isStorageReady(), setupStorage(), writeFile(), initTelemetryTask()

### Community 25 - "Telemetry Protocol"
Cohesion: 0.18
Nodes (12): 22-field telemetry format, Telemetry format single source of truth policy, FlightControl sensor FSM actuation pipeline, Flight Computer v2.0 system flowchart, Telemetry multi-channel fan-out, NEO-8M GPS module, RFM95W LoRa module, taskTelemetry (+4 more)

### Community 26 - "Flight Architecture"
Cohesion: 0.20
Nodes (12): 5 Hz flight loop tuning parameter, IDLE ASCENT DESCENT LANDED FSM, FSM-independent freefall backstop, Option A apogee parachute deployment, BMP585 barometric pressure sensor, LSM6DS3 inertial measurement unit, taskFlightControl, Four-state flight state machine (+4 more)

### Community 27 - "FSM State Persistence"
Cohesion: 0.18
Nodes (10): FlightState, ASCENT, DESCENT, IDLE, LANDED, getFlightStateName(), FlightStateMachine::begin(), FlightStateMachine::getState() (+2 more)

### Community 28 - "Hardware Modules"
Cohesion: 0.18
Nodes (11): Servo pin and team ID alignment, Selective object-oriented sensor design, MicroSD primary flight-data storage, Parachute release servo, SD and LittleFS storage fallback, ISensor abstraction layer, Firmware module architecture, Parachute actuator module (+3 more)

### Community 29 - "Freefall Analysis"
Cohesion: 0.25
Nodes (5): analyze(), load(), total_accel(), run_fsm(), total()

### Community 30 - "Flight Event Concepts"
Cohesion: 0.31
Nodes (11): Apogee detection, FSM simulator explanation, Finite state machine, Freefall detection, Liftoff detection, Motor burnout detection, Parachute altitude trigger, Sensor data validation (+3 more)

### Community 31 - "Hardware Diagnostics"
Cohesion: 0.25
Nodes (11): Interactive bench self-test, Bench self-test README, External CH343 USB-serial bridge, GPS UART wiring RX GPIO17 TX GPIO18, RFM95W LoRa radio, Parachute door servo actuator, Stale FSM snapshot in NVS, GPS raw sniffer README (+3 more)

### Community 32 - "Servo and Buzzer"
Cohesion: 0.22
Nodes (3): buzzRecoveryBeep(), buzzSignal(), setupServo()

### Community 33 - "Servo Timing Analysis"
Cohesion: 0.29
Nodes (4): analyze(), main(), parse_rows(), report()

### Community 34 - "Telemetry Format Tests"
Cohesion: 0.27
Nodes (4): flight_emit(), main(), receiver_build(), receiver_parse()

### Community 36 - "NVS Snapshot"
Cohesion: 0.22
Nodes (9): NvsSnapshot, basePressure, confirmCount, flags, magic, maxAltitude, reserved, state (+1 more)

### Community 37 - "Architecture Release Docs"
Cohesion: 0.25
Nodes (8): 5 Hz flight loop fix, Unreleased LASC 2026 post-flight changes, v2.0 OOP FreeRTOS release, Flight Computer v2.0 architecture specification, Three-task FreeRTOS separation, Sensor data queue, v1.0 to v2.0 firmware migration, Software architecture v2.0

### Community 38 - "Vertical Velocity Analysis"
Cohesion: 0.38
Nodes (7): Thonyan #51 vertical-velocity comparison chart, Firmware vz field with zero-second intervals inserted, Firmware trace shows zero plateaus and abrupt oscillations from inserted intervals, Native telemetry vz derived from altitude (dAlt/dt), packets approximately 460 ms apart, Native telemetry and 5 Hz simulation share a smooth rise-to-descent trend, vz simulated at 5 Hz using real 200 ms time steps, Vertical velocity vz (m/s) over time

### Community 39 - "Servo Load Testing"
Cohesion: 0.48
Nodes (7): Deploy at apogee invariant, BEC rail stall-current limit, Bench microswitch fixture, Parachute servo load-test README, No deployment feedback in flight firmware, Servo and real mechanism load test, Twenty-trial servo pass/fail criteria

### Community 40 - "Log Messages"
Cohesion: 0.33
Nodes (5): LogMessage, level, message, taskId, timestamp

### Community 41 - "Recovery Architecture"
Cohesion: 0.50
Nodes (4): Layered redundant recovery architecture, Electronic failure risk assessment, Scientific flight-data payload, Electronic telemetry and payload system extraction

### Community 42 - "Logger Statistics"
Cohesion: 0.50
Nodes (4): LoggerStats, droppedCount, printedCount, receivedCount

### Community 43 - "FreeRTOS Task IDs"
Cohesion: 0.50
Nodes (4): TaskId, TASK_ID_FLIGHT_CONTROL, TASK_ID_LOGGER, TASK_ID_TELEMETRY

### Community 44 - "Flight Telemetry Records"
Cohesion: 0.67
Nodes (4): Thonyan flight record, GPS position data, LoRa telemetry packets, Lost packet counts

### Community 45 - "Mechanical CAD"
Cohesion: 0.50
Nodes (4): Mechanical assembly README, SolidWorks mechanical parts and assemblies, STEP neutral format, STEP source of truth for cutting and CAM

## Knowledge Gaps
- **150 isolated node(s):** `_baro`, `_imu`, `_state`, `_ready`, `_liftoffDetected` (+145 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 372 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **5 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `FlightStateMachine` connect `Flight State Machine` to `State Transition Logic`, `NVS Snapshot`, `GPS Sensor Module`, `Barometer Sensor`, `Flight Event Detection`, `IMU Sensor Module`, `IMU Implementation`, `FSM State Persistence`?**
  _High betweenness centrality (0.067) - this node is a cross-community bridge._
- **What connects `_baro`, `_imu`, `_state` to the rest of the system?**
  _150 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Project Documentation` be split into smaller, more focused modules?**
  _Cohesion score 0.05202661826981246 - nodes in this community are weakly interconnected._
- **Why does `BMP585Sensor` connect `Barometer Sensor` to `Flight State Machine`, `GPS Sensor Module`, `IMU Sensor Module`, `Sensor Libraries`, `Sensor Updates`?**
  _High betweenness centrality (0.034) - this node is a cross-community bridge._
- **Should `FSM Analysis Tools` be split into smaller, more focused modules?**
  _Cohesion score 0.08562367864693446 - nodes in this community are weakly interconnected._
- **Why does `SensorData` connect `Sensor Data Model` to `Flight Control Task`, `FSM State Persistence`, `Telemetry Task`?**
  _High betweenness centrality (0.024) - this node is a cross-community bridge._
- **Should `Flight State Machine` be split into smaller, more focused modules?**
  _Cohesion score 0.04878048780487805 - nodes in this community are weakly interconnected._