# Módulos do Flight Computer (v2.0)

Documentação da arquitetura atual do firmware (FreeRTOS + OOP). Substitui a
versão antiga que citava bmp280/mpu6050/telemetry_module/server (removidos na
refatoração v2.0). Fonte de verdade para módulos e tarefas.

## Estrutura de pastas

```
firmware/
├── config.h                 # Pinos, thresholds, parâmetros LoRa (constantes)
├── firmware.ino             # setup()/loop() — apenas orquestra as init*Task()
├── sensors/                 # Abstração de hardware (ISensor)
│   ├── ISensor.h            # Interface comum (begin/update/getData/isReady)
│   ├── BMP585Sensor.h/.cpp  # Barômetro (altitude, pressão, temp, Vz)
│   ├── LSM6DS3Sensor.h/.cpp # IMU (aceleração + giroscópio)
│   └── GPSModule.h/.cpp     # GPS (lat/lon/alt/sats, non-blocking NMEA)
├── modules/                 # Atuadores e periféricos
│   ├── parachute_module.h   # Dono do servo (ParachuteServo + setupServo)
│   ├── lora_module.h        # Rádio LoRa (setupLoRa/sendLoRa)
│   ├── buzzer_module.h      # Buzzer de status
│   └── filesystem_module.h  # LittleFS (writeFile/appendFile)
└── flight/                  # Lógica de voo (FreeRTOS tasks + FSM)
    ├── SensorData.h         # Struct SensorData + enum FlightState
    ├── FlightStateMachine.h/.cpp  # FSM (4 estados + 7 sub-eventos)
    ├── FlightControlTask.h/.cpp   # Task 1 — 50Hz (FSM + deploy + queue)
    ├── TelemetryTask.h/.cpp       # Task 2 — 5Hz (assemble + LoRa + file)
    └── LoggerTask.h/.cpp          # Task 3 — baixa prioridade (log Serial)
```

## sensors/ (camada ISensor)

Todos os sensores implementam `ISensor` (`begin`, `update`, `getData`,
`isReady`).

- **BMP585Sensor** — altitude barométrica (m), pressão (hPa), temperatura (C),
  e velocidade vertical `Vz` (m/s) derivada por filtro. Métodos: `getAltitude()`,
  `getVerticalVelocity()`, `getPressure()`, `getMaxAltitude()`, `getPreviousAltitude()`.
- **LSM6DS3Sensor** — aceleração `ax,ay,az` (m/s²) e giroscópio `gx,gy,gz`
  (rad/s). `getTotalAccel()` retorna a magnitude.
- **GPSModule** — fix não-bloqueante via UART1. `hasValidFix()`, `getLatitude()`,
  `getLongitude()`, `getGPSAltitude()`, `getSatellites()`, `getTimeString()`.

## modules/ (atuadores e periféricos)

- **parachute_module.h** — ÚNICO dono do atuador: `Servo ParachuteServo` e
  `bool setupServo()`. A lógica de *decisão* de deploy vive na FSM +
  FlightControlTask (Option A: abre no apogeu). O módulo só posiciona o servo.
- **lora_module.h** — `setupLoRa()` aplica 915E6 / SYNC 0xF3 / SF7 / BW125k /
  CR5 / TP17 / CRC, e `sendLoRa(String)`.
- **buzzer_module.h** — sinais sonoros de status (init/flight/landed).
- **filesystem_module.h** — `setupLittleFS()`, `writeFile()`, `appendFile()`
  (LittleFS).

## flight/ (lógica de voo)

- **SensorData.h** — `struct SensorData` (payload da fila) e `enum FlightState`
  {IDLE, LIFTOFF, BURNOUT, APOGEE, FREEFALL, PARACHUTE, LANDED}. Os 4 estados
  "externos" do plano (IDLE/ASCENT/DESCENT/LANDED) são representados aqui como
  LIFTOFF..LANDED; os sub-eventos (burnout/apogee/freefall) são flags internas.
- **FlightStateMachine** — consume BMP585 + LSM6DS3; detecta liftoff, burnout,
  apogee, freefall, parachute (flags booleanas). Deploy de paraquedas ocorre no
  **apogeu** (Option A): `detectParachute()` confirma `Vz` negativo por
  `PARACHUTE_CONFIRM_CYCLES` ciclos, com piso `PARACHUTE_MIN_ALTITUDE` (50 m).
- **FlightControlTask** — Task 1 @50Hz (Core 1). Atualiza sensores + FSM,
  aciona `deployParachute()` (usa `ParachuteServo` do módulo) quando a FSM
  confirma, e empurra `SensorData` para `sensorDataQueue`. Arma o TWDT dentro
  da própria task.
- **TelemetryTask** — Task 2 @5Hz (Core 0). Drena `sensorDataQueue` (mantém a
  amostra mais nova), enriquece com GPS, monta CSV v2.0 (`assembleTelemetry`),
  e faz fan-out para Serial + LoRa + LittleFS.
- **LoggerTask** — Task 3 (baixa prioridade). Consome `logQueue` e imprime no
  Serial (filtro de `LOG_LEVEL`).

## Filas (config.h)

- `SENSOR_DATA_QUEUE_LEN` — entre FlightControl e Telemetry.
- `LOG_QUEUE_LEN` — entre qualquer task e Logger.

## Formato de telemetria

Ver `docs/telemetry-format.md` (fonte única, Fase 10). Resumo: 22 campos no
satellite → 24 campos no protocolo do receiver (com hora/data GPS local +
RSSI real).
