# Migration Guide: v1.0 → v2.0 (Flight Computer)

This document records the architectural decisions and concrete changes made
when moving the Flight Computer firmware from the v1.0 procedural design
(branch `main`, pre-refactor) to the v2.0 OOP + FreeRTOS design (branch
`dev-2026`). It is the rationale companion to `REFACTORING_PLAN.md` and the
`docs/` files — read it when you need to understand *why* something changed,
not just *what* the code looks like now.

## TL;DR

| Aspect | v1.0 (main, legacy) | v2.0 (dev-2026) |
|--------|---------------------|-----------------|
| Architecture | Procedural `loop()` in one `firmware.ino` (~680 lines) | OOP + FreeRTOS, modular `sensors/` `modules/` `flight/` |
| Sensors | BMP280 + MPU6050 + NEO-6M | BMP585 + LSM6DS3 + NEO-8M |
| Flight logic | `handleParachute()` with altitude ceiling | `FlightStateMachine` (4 states + 7 sub-events) |
| Parachute | Deploy at `ALTITUDE_THRESHOLD = 750 m` | Deploy at **apogee** |
| Comms | LoRa 868 MHz, ad-hoc CSV | LoRa 915 MHz, v2.0 22-field format |
| Web UI | ESPAsyncWebServer on board | Removed (receiver provides web UI) |
| Storage | LittleFS, v1.0 CSV header | SD card (filesystem_module.h `setupStorage`), LittleFS fallback; 22-field v2.0 CSV |

## Side-by-side: Architecture

### Project structure

**v1.0** — flat, monolithic:

```
firmware/
├── firmware.ino              # ~680 lines: setup() + loop() + all logic
├── config.h
├── data/server/              # ESPAsyncWebServer web UI (index.html/js/css)
├── bmp280_sensor.h           # legacy sensor module
├── mpu6050_sensor.h
├── gps_module.h
├── lora_module.h
├── filesystem_module.h
├── parachute_module.h
└── buzzer_module.h
```

**v2.0** — modular OOP + FreeRTOS:

```
firmware/
├── firmware.ino              # thin entry point: setup() calls init*Task()
├── config.h                  # pins, thresholds, LoRa params
├── sensors/                  # ISensor abstraction
│   ├── ISensor.h
│   ├── BMP585Sensor.h/.cpp
│   ├── LSM6DS3Sensor.h/.cpp
│   └── GPSModule.h/.cpp
├── modules/                  # actuators / peripherals
│   ├── parachute_module.h    # owns ParachuteServo
│   ├── lora_module.h
│   ├── buzzer_module.h
│   └── filesystem_module.h
└── flight/                   # FreeRTOS tasks + FSM
    ├── SensorData.h
    ├── FlightStateMachine.h/.cpp
    ├── FlightControlTask.h/.cpp
    ├── TelemetryTask.h/.cpp
    └── LoggerTask.h/.cpp
```

### Data flow

**v1.0** — synchronous, single thread:

```
loop() [every INTERVAL ms]
  └─ readSensors()        # blocking I2C/UART reads
       └─ handleParachute()   # direct servo write if alt < 750 m
            └─ logData()      # append to LittleFS
                 └─ sendLoRa() # one long CSV string
```

**v2.0** — concurrent, queue-driven:

```
Core 1 (50 Hz, prio 20):  taskFlightControl
  ├─ BMP585/LSM6DS3 update()
  ├─ FlightStateMachine.update()  → detectParachute() at apogee
  ├─ deployParachute() via parachute_module
  └─ xQueueSend(sensorDataQueue)

Core 0 (5 Hz, prio 5):    taskTelemetry
  ├─ GPSModule.update()
  ├─ xQueueReceive(sensorDataQueue)  → newest sample
  ├─ assembleTelemetry()  → 22-field CSV
  └─ fan-out: Serial + sendLoRa() + appendFile()

Core 0 (low prio):        taskLogger
  └─ xQueueReceive(logQueue) → Serial print (level filter)
```

### Build / runtime footprint

| | v1.0 | v2.0 |
|--|------|------|
| Entry point | `loop()` does everything | `loop()` empty (`vTaskDelay(portMAX_DELAY)`) |
| Concurrency | none (blocking calls) | 3 FreeRTOS tasks, 2 cores |
| Real-time guarantee | none | FlightControl @50Hz isolated on Core 1 |
| Watchdog | none | TWDT armed inside `taskFlightControl` |

## Side-by-side: Code

### Entry point (`firmware.ino`)

**v1.0** — everything in `loop()`:

```cpp
void loop() {
  if (millis() - lastRead >= INTERVAL) {
    lastRead = millis();
    readSensors();
    handleParachute();   // direct: if (altitude < 750) servo.write(90);
    logData();
    sendLoRa();
  }
}
```

**v2.0** — thin setup, tasks own the work:

```cpp
void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  if (!initFlightControlTask()) { Serial.println("FATAL"); ESP.restart(); }
  if (!initTelemetryTask())      { Serial.println("FATAL"); ESP.restart(); }
  if (!initLoggerTask())         { Serial.println("FATAL"); ESP.restart(); }
}

void loop() {
  vTaskDelay(portMAX_DELAY);   // all work runs in FreeRTOS tasks
}
```

### Parachute decision

**v1.0** — fixed altitude ceiling (`handleParachute`):

```cpp
const float ALTITUDE_THRESHOLD = 750.0;        // m
const float ALTITUDE_DROP_THRESHOLD = 10.0;   // m
const float VELOCITY_THRESHOLD = 80.0;        // m/s

void handleParachute() {
  if (altitude < ALTITUDE_THRESHOLD &&
      (peakAltitude - altitude) > ALTITUDE_DROP_THRESHOLD &&
      descentVelocity > VELOCITY_THRESHOLD) {
    servo.write(90);   // open
  }
}
```

**v2.0** — apogee detection in the FSM (`detectParachute`, Option A):

```cpp
// config.h
#define PARACHUTE_MIN_ALTITUDE 50.0f   // ground guard only
#define PARACHUTE_CONFIRM_VZ   -2.0f   // m/s
#define PARACHUTE_CONFIRM_CYCLES 3u

// FlightStateMachine::detectParachute()
bool detectParachute(float height, float vz) const {
  if (height < PARACHUTE_MIN_ALTITUDE) return false;     // never near ground
  if (vz < PARACHUTE_CONFIRM_VZ) {                       // stable negative Vz
    return (_parachuteConfirmCount >= PARACHUTE_CONFIRM_CYCLES);
  }
  return false;
}
// actuated by taskFlightControl → parachute_module.deployParachute()
```

### Sensor reading

**v1.0** — global sensor objects, free functions:

```cpp
Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;

void readSensors() {
  float altitude = bmp.readAltitude(BASE_PRESSURE);
  sensors_event_t a, g;
  mpu.getEvent(&a, &g, &temp);
  // ... used directly, no type safety
}
```

**v2.0** — `ISensor` interface, polymorphism:

```cpp
class BMP585Sensor : public ISensor {
  bool begin() override;
  void update() override;          // reads + filters + derives Vz
  String getData() override;      // CSV fragment
  float getAltitude() const;
  float getVerticalVelocity() const;
};

// FlightControlTask calls through the interface:
_baro->update();
_imu->update();
float vz = _baro->getVerticalVelocity();
```

### Telemetry line

**v1.0** — ad-hoc CSV, receiver-incompatible ordering:

```cpp
String line = String(ID) + "," + Packet + "," + Time + "," +
              Latitude + "," + Longitude + "," + GPS_Altitude + "," +
              Satellites + "," + Date + "," + Hours + "," + Minutes + "," +
              Seconds + "," + BMP_Altitude + "," + Pressure + "," +
              AccX + "," + AccY + "," + AccZ + "," + GyroX + "," +
              GyroY + "," + GyroZ + "," + Temp + "," + ParachuteStatus;
```

**v2.0** — 22-field v2.0 format (see `docs/telemetry-format.md`):

```cpp
// TelemetryTask::assembleTelemetry()
String line =
  TEAM_ID + "," + millis + "," + count + "," + altp + "," + temp + "," +
  umi + "," + p + "," + gx + "," + gy + "," + gz + "," + ax + "," + ay + "," +
  az + "," + vz + "," + maxAltitude + "," + state + "," + alt + "," + lat + "," +
  lon + "," + sat + "," + parachute + "," + rssi;
```

### Radio configuration

**v1.0**:

```cpp
#define LORA_FREQ 868E6     // Europe
LoRa.setSyncWord(0xF3);
// SF/BW/CR left at library defaults
```

**v2.0**:

```cpp
#define LORA_FREQ 915E6     // Americas/Brazil, matches receiver
LoRa.setSyncWord(SYNC_WORD);          // 0xF3
LoRa.setSpreadingFactor(LORA_SF);     // 7
LoRa.setSignalBandwidth(LORA_BW);     // 125E3
LoRa.setCodingRate4(LORA_CR);         // 5
LoRa.setTxPower(LORA_TX_POWER);       // 17
```

## Decisions and rationale

### 1. OOP + FreeRTOS instead of a single procedural `loop()`

**v1.0**: all logic lived in `firmware.ino` with `readSensors()`,
`handleParachute()`, `logData()`, `sendLoRa()` called from `loop()` on a
`INTERVAL` timer. Hard to test, no real-time guarantees.

**v2.0**: sensor classes implement a common `ISensor` interface; flight logic
is split into three FreeRTOS tasks:

- `taskFlightControl` — Core 1, 50 Hz, priority 20 (sensors + FSM + deploy)
- `taskTelemetry` — Core 0, 5 Hz, priority 5 (GPS + assemble + fan-out)
- `taskLogger` — Core 0, low priority (log queue consumer)

Tasks communicate via `sensorDataQueue` and `logQueue`. The Arduino `loop()`
is intentionally empty (`vTaskDelay(portMAX_DELAY)`).

**Why**: deterministic timing for the safety-critical path, testable units,
cleaner separation. The monolithic `firmware.ino` is now only an entry point
that calls `init*Task()` functions.

### 2. Parachute at apogee (Option A), not at a fixed altitude ceiling

**v1.0**: `handleParachute()` opened the servo when altitude dropped below
`ALTITUDE_THRESHOLD = 750 m` with a velocity/drop guard
(`ALTITUDE_DROP_THRESHOLD = 10 m`, `VELOCITY_THRESHOLD = 80 m/s`).

**v2.0**: `FlightStateMachine::detectParachute()` opens at **apogee** — it
confirms a stable negative vertical velocity `Vz` for
`PARACHUTE_CONFIRM_CYCLES` cycles and never below `PARACHUTE_MIN_ALTITUDE`
(50 m ground guard only). The servo is actuated by `taskFlightControl` via
`parachute_module` (`ParachuteServo`).

**Why**: a fixed 750 m ceiling is wrong for a ~1000 m flight — it would deploy
far too early (or never, on shorter flights). Deploying at apogee maximizes
drag time and is validated against both a RocketPy simulation (apogee 951 m →
deploy 949.5 m) and real flight data (apogee 272 m → deploy 268 m). This was
the user-mandated decision ("deploy MUST open at apogee for ANY flight above
min altitude").

### 3. Telemetry format — Option B (v2.0 22-field), not a shrink to 19

**v1.0**: ad-hoc CSV, different field order than the receiver expected.

**v2.0**: chose **Option B** — a clean v2.0 wire format of 22 fields emitted by
the flight computer and parsed by the receiver
(`recovery-webui/components/receiver-lora`):

```
TEAM_ID,millis,count,altp,temp,umi,p,gx,gy,gz,ax,ay,az,vz,
maxAltitude,state,alt,lat,lon,sat,parachute,rssi
```

The receiver re-emits 24 fields (inserts local GPS `hora`/`data` + real `rssi`).
Single source of truth: `docs/telemetry-format.md`.

**Why Option B over Option A** (shrinking the flight CSV to the old 19-field
v1.0 layout): keeps the richer v2.0 diagnostics (`vz`, `maxAltitude`, `state`,
`parachute`) and fixes a real parsing bug where the old 19-field parser shifted
`alt`/`lat`/`lon`/`sat`/`rssi` onto the wrong indices. Validated end-to-end
with `extras/validate_telemetry_format.py`.

### 4. Radio frequency 868 MHz → 915 MHz

**v1.0**: `LORA_FREQ 868E6` (Europe ISM).

**v2.0**: `LORA_FREQ 915E6` (Americas/Brazil ISM), matching the receiver.
Sync word `0xF3`, SF7, BW 125 kHz, CR 4/5, TX +17 dBm, CRC on — all aligned
explicitly in `lora_module.h` `setupLoRa()`.

**Why**: the flight computer and receiver must share the same frequency to
communicate at all. The receiver was already on 915 MHz, so the flight side
was moved to match.

### 5. Sensor hardware upgrade

| v1.0 | v2.0 | Note |
|------|------|------|
| BMP280 (barometer) | BMP585 | higher accuracy, same I2C 0x77 |
| MPU6050 (IMU) | LSM6DS3 | lower noise, same I2C |
| NEO-6M (GPS) | NEO-8M | more constellations, non-blocking UART |

### 6. Web interface removed from the board

**v1.0**: an `ESPAsyncWebServer` + `WiFi` UI served from `firmware/data/server/`.

**v2.0**: removed. Telemetry is recovered via the ground **receiver**, which
provides its own web UI and logs. The board focuses on flight-critical work
only.

### 7. Parachute actuation ownership

**v1.0**: servo written directly from flight logic (`Servo.h`).

**v2.0**: `parachute_module.h` is the sole owner of the `Servo ParachuteServo`
object (`setupServo()`, `deployParachute()`). The FSM/Task decide *when*; the
module *acts*. Dead code (`handleParachute`, `printBoth`, globals) was removed
in the Fase 9 cleanup.

## Files removed vs v1.0

- `firmware/firmware.ino` monolithic logic → replaced by modular `flight/` +
  `sensors/` + `modules/` + thin entry point.
- `firmware/data/server/` (web UI) → removed.
- Legacy sensor modules `bmp280_sensor.h`, `mpu6050_sensor.h`, `gps_module.h`
  → replaced by `sensors/BMP585Sensor.*`, `sensors/LSM6DS3Sensor.*`,
  `sensors/GPSModule.*`.
- Test sketches from v1.0 (`test/basico`, `test/sem_lora`, `test/server`,
  legacy `test/LittleFS`, `test/testeGPS`) → superseded by v2.0 tests
  (`test/FSM/FSM.ino`, `test/test_gps`, etc.).

## Files added in v2.0

- `firmware/sensors/` (ISensor, BMP585, LSM6DS3, GPS)
- `firmware/flight/` (SensorData, FlightStateMachine, FlightControlTask,
  TelemetryTask, LoggerTask)
- `firmware/modules/` (parachute, lora, buzzer, filesystem)
- `firmware/REFACTORING_PLAN.md`, `firmware/MODULOS.md`
- `docs/telemetry-format.md`, this file
- `extras/validate_telemetry_format.py`, `extras/FSM_tester/`

## Validation

- FSM: `python3 extras/FSM_tester/FSM_Tester.py` (real 1,873-point flight data)
- Telemetry: `python3 extras/validate_telemetry_format.py` (flight→receiver
  index match)
- Parachute Option A: validated with RocketPy simulation + real flight CSV.

## Open items / carry-over

- Hardware I2C pins are the ESP32-C3 default (SDA/SCL); verify against the
  KiCad silkscreen before next board spin.
- `docs/flowchart.md` and `hardware/` still reference some legacy artifacts —
  tracked separately from this migration.
