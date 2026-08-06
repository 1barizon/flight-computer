# 🚀 Plano de Refatoração: POO + FreeRTOS + FSM

**Projeto:** Flight Computer - #11  
**Hardware:** ESP32-C3 SuperMini (atual), ESP32-S3-DevKitC-1-N8R8 (v2.0 alvo)  
**Data Início:** 2026-03-18  
**Status:** 🚀 Fases 1-10 concluídas — v2.0 completa

---

## 📊 Resumo Executivo

### Objetivos
1. ✅ Refatorar código procedural para **POO seletivo** (sensores apenas)
2. ✅ Implementar **FSM** para controle de estados de voo
3. ✅ Usar **FreeRTOS** para separar lógica crítica (FSM) de I/O (telemetria)
4. ✅ Substituir sensores: **BMP280→BMP585**, **MPU6050→LSM6DS3**, **GPS N6M→N8M**
5. ✅ Preparar arquitetura para modificações futuras

### Progresso Atual
- ✅ Fases 1-4 concluídas (sensores e base OOP implementados)
- ✅ Fase 5 concluída (GPSModule)
- ✅ Fase 6 concluída (FSM 4 estados, mergeada via PR #16)
- ✅ Fase 7 concluída (FreeRTOS Tasks: FlightControl @50Hz, Telemetry @5Hz, Logger)
- ✅ Fase 8 concluída (Integração firmware.ino com init*Task())
- ✅ Fase 9 concluída (Módulos adaptados: parachute, lora, filesystem, buzzer)
- ✅ Fase 10 concluída (Formato de telemetria 22 campos alinhado com receiver)

### Motivação
- Código monolítico (584 linhas) chegou ao **EOL**
- Troca de sensores planejada requer abstração
- FSM necessária para controle de voo robusto
- Separação safety-critical (paraquedas) de I/O (logging)

---

## 🎯 Decisões Arquiteturais

| Aspecto | Decisão | Justificativa |
|---------|---------|---------------|
| **Abordagem POO** | Seletivo | Apenas sensores que mudam viram classes |
| **FSM** | Enum/Switch | Simples, direto, baixo overhead |
| **FreeRTOS** | 3 Tasks | Separação FSM (50Hz) / Telemetry (5Hz) / Logger |
| **Migração** | Big Bang | Refatoração completa de uma vez |
| **Testes** | Manuais/Hardware | Sem infraestrutura de unit tests |
| **Queue Size** | 25 (sensores), 50 (logs) | Cobre pior caso com margem |
| **Watchdog** | 5s, apenas Task FSM | Reset automático se travamento crítico |
| **Logging** | Híbrido (Queue + #ifdef) | Debug verboso em dev, seletivo em produção |

---

## 🏗️ Arquitetura Final

### Estrutura de Diretórios

```
firmware/
│
├── firmware.ino                    # Main - Setup + Tasks creation
│
├── config.h                        # Global configs + FSM thresholds
│
├── sensors/                        # 🆕 POO - Sensor abstractions
│   ├── ISensor.h                   # Abstract base interface
│   ├── BMP585Sensor.h/cpp          # Barometer (altitude, pressure, temp)
│   ├── LSM6DS3Sensor.h/cpp         # IMU (accelerometer + gyroscope)
│   └── GPSModule.h/cpp             # GPS positioning and time
│
├── flight/                         # 🆕 FSM + FreeRTOS Tasks
│   ├── SensorData.h                # Shared structs (SensorData, LogMessage)
│   ├── FlightStateMachine.h/cpp    # FSM - 4 states + transitions (SIMPLIFICADO)
│   ├── FlightControlTask.h/cpp     # Task 1 - FSM + Safety (50Hz, Core 1)
│   ├── TelemetryTask.h/cpp         # Task 2 - Logging + TX (5Hz, Core 0)
│   └── LoggerTask.h/cpp            # Task 3 - Debug logger (low priority)
│
└── modules/                        # Procedural modules (unchanged)
    ├── buzzer_module.h             # Audio feedback
    ├── filesystem_module.h         # LittleFS operations
    ├── lora_module.h               # LoRa transmission
    ├── parachute_module.h          # Servo + deployment logic
```

### Diagrama de Tasks FreeRTOS

```
┌────────────────────────────────────────────────────────────┐
│               ESP32-S3 Dual Core Architecture              │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  CORE 1 (pinned)             CORE 0 (shared)              │
│  ┌──────────────────────┐    ┌──────────────────────┐     │
│  │ Task 1: FlightCtrl   │    │ Task 2: Telemetry    │     │
│  │ Priority: 20 (HIGH)  │    │ Priority: 5 (LOW)    │     │
│  │ Freq: 50Hz (20ms)    │    │ Freq: 5Hz (200ms)    │     │
│  ├──────────────────────┤    ├──────────────────────┤     │
│  │ • BMP585->update()   │    │ • GPS->update()      │     │
│  │ • LSM6DS3->update()  │    │ • Receive Queue      │     │
│  │ • FSM->update()      │    │ • Serial.println()   │     │
│  │ • Parachute deploy   │    │ • sendLoRa()         │     │
│  │ • Send to Queue      │    │ • appendFile()       │     │
│  │ • Watchdog reset     │    │ • buzzSignal()       │     │
│  └──────────────────────┘    └──────────────────────┘     │
│           │                            │                   │
│           └──── sensorDataQueue ───────┘                   │
│                (25 slots, ~1.25KB)                         │
│                                                            │
│  ┌────────────────────────────────────────────────────┐   │
│  │ Task 3: Logger (CORE 0, Priority: 1, IDLE+)       │   │
│  │ • Receive from logQueue                           │   │
│  │ • Serial.printf() thread-safe                     │   │
│  │ • Append to /logs/flight.log (if DEBUG_MODE)      │   │
│  └────────────────────────────────────────────────────┘   │
│                      │                                     │
│                 logQueue (50 slots, ~6KB)                  │
│                      △                                     │
│          ┌───────────┴────────────┐                        │
│     Task 1 logs            Task 2 logs                     │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

### FSM - Estados de Voo (4 estados - versão simplificada)

**Nota:** A FSM adotada para a v2.0 é a versão **simplificada de 4 estados**, usada
no código atual (IDLE, ASCENT, DESCENT, LANDED). A validação por dados reais
continua sendo feita, porém sem a granularidade do modelo anterior de 7 estados.

```
┌─────────┐
│  IDLE   │  Pré-lançamento, aguardando no solo
└────┬────┘
     │ Trigger: aceleração total acima do limiar
     ▼
┌──────────┐
│ ASCENT   │  Subida (motorizada ou balística)
└────┬─────┘
     │ Trigger: velocidade vertical negativa persistente
     ▼
┌──────────┐
│ DESCENT  │  Queda/descida controlada
└────┬─────┘
     │ Trigger: baixa velocidade + altitude estável
     ▼
┌──────────┐
│  LANDED  │  Pouso detectado, fim do voo
└──────────┘
```

**Validações de Segurança OBRIGATÓRIAS:**
```cpp
// CRÍTICO: Todas as funções de detecção devem incluir:
// 1. Verificação NaN/Inf
if (!std::isfinite(vz) || !std::isfinite(az) || !std::isfinite(totalAccel)) {
    return false;  // Dados inválidos
}

// 2. Guards mínimos para evitar false positives
if (height <= SAFETY_MIN_ALTITUDE) {
    return false;
}

// 3. Clipping de Velocidade Vertical
vz = constrain(vz, -200.0, 200.0);
```

---

## 📦 Bibliotecas e Dependências

### Sensores

| Sensor Antigo | Sensor Novo | Biblioteca | Instalação |
|---------------|-------------|------------|------------|
| BMP280 | **BMP585** | `Adafruit_BMP5xx` | Library Manager |
| MPU6050 | **LSM6DS3** | `Adafruit_LSM6DS` | Library Manager |
| GPS N6M | **GPS N8M** | `TinyGPSPlus` (mantém) | Já instalado |

### Dependências
- `Adafruit_Sensor` (unified sensor interface)
- `Adafruit_BusIO` (I2C/SPI abstraction)
- `ArduinoJson` (telemetry formatting)
- `LoRa` (wireless communication)

### Instalação
```bash
# Via Arduino Library Manager:
Adafruit BMP5xx
Adafruit LSM6DS
Adafruit Unified Sensor
Adafruit BusIO
TinyGPSPlus
ArduinoJson
LoRa
```

---

## 🗂️ Estruturas de Dados

### SensorData (Queue: Task 1 → Task 2)

```cpp
struct SensorData {
  // Timestamp
  unsigned long timestamp;
  uint16_t packet_count;
  
  // BMP585
  float altitude;
  float pressure;
  float temperature;
  float verticalVelocity;
  float maxAltitude;
  
  // LSM6DS3
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float totalAccel;  // 🆕 Calculado pelo LSM6DS3Sensor
  
  // GPS (apenas se fix disponível)
  double latitude, longitude;
  float gpsAltitude;
  uint8_t satellites;
  bool gps_valid;
  
  // FSM
  FlightState state;
  bool parachute_deployed;
};

// Queue: 25 slots × 96 bytes = ~2.4KB RAM
QueueHandle_t sensorDataQueue;
```

### LogMessage (Queue: Tasks 1,2 → Task 3)

```cpp
struct LogMessage {
  char message[128];
  unsigned long timestamp;
  uint8_t taskId;    // 1=FSM, 2=Telemetry, 3=Logger
  uint8_t level;     // 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
};

// Queue: 50 slots × 140 bytes = ~7KB RAM
QueueHandle_t logQueue;
```

---

## 📝 Plano de Implementação (9 Fases)

### Checklist Geral

| Fase | Descrição | Tempo | Status |
|------|-----------|-------|--------|
| 1 | Setup e preparação | 30 min | ✅ Completa |
| 2 | Interface base + structs | 45 min | ✅ Completa |
| 3 | BMP585Sensor (classe) | 2 h | ✅ Completa |
| 4 | LSM6DS3Sensor (classe) | 1.5 h | ✅ Completa |
| 5 | GPSModule (classe) | 1 h | ✅ Completa |
| 6 | FSM - Máquina de estados (4 estados) | **4 h** | ✅ Completa (PR #16) |
| 7 | FreeRTOS Tasks | 4 h | ✅ Completa (Tasks 1, 2 e 3) |
| 8 | Integração firmware.ino | 2 h | ✅ Completa |
| 9 | Adaptar módulos dependentes | 1.5 h | ✅ Completa |
| 10 | Comunicação com o Receiver | 2 h | ✅ Completa |
| **TOTAL** | | **19.25h** | **100% 🚀** |

---

### FASE 1: Setup e Preparação ⏱️ 30min

**Status:** ✅ **COMPLETA**

**Objetivos:**
- [x] Criar estrutura de diretórios (`sensors/`, `flight/`)
- [x] Reorganizar módulos existentes em `modules/`
- [x] Instalar bibliotecas via Arduino Library Manager
- [x] Criar branch `feature/oop-freertos-refactor`

**Ações:**
```bash
# 1. Criar branch
git checkout -b feature/oop-freertos-refactor

# 2. Criar estrutura de pastas
mkdir -p sensors flight modules

# 3. Mover módulos existentes
mv buzzer_module.h modules/
mv filesystem_module.h modules/
mv lora_module.h modules/
mv parachute_module.h modules/

# 4. Instalar bibliotecas (via Arduino IDE Library Manager)
# - Adafruit_BMP5xx
# - Adafruit_LSM6DS
# - Adafruit_Sensor
# - Adafruit_BusIO
```

**Entregável:**
- ✅ Estrutura de pastas criada
- ✅ Bibliotecas instaladas
- ✅ Branch criado

**Notas:**
- Manter código antigo intacto até validação completa
- Testar bibliotecas com sketches simples antes de integrar

---

### FASE 2: Interface Base + Structs ⏱️ 45min

**Status:** ✅ **COMPLETA (implementação)**

**Objetivos:**
- [x] Criar interface abstrata `ISensor`
- [x] Criar structs de comunicação (`SensorData`, `LogMessage`)
- [x] Definir enum `FlightState`

**Arquivos a Criar:**

**`sensors/ISensor.h`**
```cpp
#ifndef ISENSOR_H
#define ISENSOR_H

#include <Arduino.h>

/**
 * @brief Interface abstrata para sensores do flight computer
 */
class ISensor {
public:
  virtual ~ISensor() = default;
  
  virtual bool begin() = 0;
  virtual void update() = 0;
  virtual String getData() = 0;
  virtual bool isReady() = 0;
};

#endif
```

**`flight/SensorData.h`**
```cpp
#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

enum FlightState {
  IDLE = 0,
  ASCENT = 1,
  DESCENT = 2,
  LANDED = 3
};

struct SensorData {
  unsigned long timestamp;
  uint16_t packet_count;
  
  float altitude;
  float pressure;
  float temperature;
  float verticalVelocity;
  float maxAltitude;
  
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  
  FlightState state;
  bool parachute_deployed;
};

struct LogMessage {
  char message[128];
  unsigned long timestamp;
  uint8_t taskId;
  uint8_t level;
};

#endif
```

**Validação (pendente):**
- [ ] Compilação OK (headers apenas)
- [ ] Sem erros de sintaxe

---

### FASE 3: BMP585Sensor (Classe) ⏱️ 2h

**Status:** ✅ **COMPLETA (implementação)**

**Objetivos:**
- [x] Criar classe do barômetro BMP585
- [x] Migrar lógica de `bmp280_sensor.h`
- [x] **IMPLEMENTAR cálculo de velocidade vertical (Vz)** - CRÍTICO para FSM
- [x] Testar leitura de altitude/pressão/temperatura

**Arquivos:** `sensors/BMP585Sensor.h` + `sensors/BMP585Sensor.cpp`

**API Pública:**
```cpp
class BMP585Sensor : public ISensor {
public:
  BMP585Sensor();
  
  // Interface ISensor
  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;
  
  // Específicos do barômetro
  float getAltitude() const;
  float getMaxAltitude() const;
  float getVerticalVelocity() const;  // ⚠️ CRÍTICO: Calculado via diferenciação numérica
  void checkHighest();
  
private:
  Adafruit_BMP5XX _bmp;
  float _basePressure;
  float _maxAltitude;
  float _prevAltitude;
  unsigned long _prevTime;
  float _verticalVelocity;  // Vz = (altitude_current - altitude_previous) / dt
};
```

**Migração de `bmp280_sensor.h`:**
- ❌ Remover: variáveis globais `BMP`, `max_altitude`, `base_pressure`, etc.
- ✅ Transformar em: membros privados da classe
- ✅ Adaptar: `Adafruit_BMP280` → `Adafruit_BMP5XX`
- ✅ **IMPLEMENTAR cálculo de Vz** (referência: linha 201 do `FSM_Tester.py`)

**Cálculo de Velocidade Vertical (CRÍTICO):**
```cpp
// Em BMP585Sensor::update()
void BMP585Sensor::update() {
  sensors_event_t temp_event, pressure_event;
  _bmp.getEvent(&pressure_event, &temp_event);
  
  float altitude_current = _bmp.readAltitude(_basePressure);
  unsigned long time_current = millis();
  
  // Diferenciação numérica (Python linha 201)
  float dt = (time_current - _prevTime) / 1000.0;  // segundos
  if (dt > 0.001) {  // Evitar divisão por zero
    _verticalVelocity = (altitude_current - _prevAltitude) / dt;
    _verticalVelocity = constrain(_verticalVelocity, -200.0, 200.0);  // Clipping
    
    _prevAltitude = altitude_current;
    _prevTime = time_current;
  }
}
```

**Teste Simples:**
```cpp
// test_bmp585.ino
#include "sensors/BMP585Sensor.h"

BMP585Sensor baro;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (baro.begin()) {
    Serial.println("BMP585 OK!");
  }
}

void loop() {
  baro.update();
  Serial.println(baro.getData());
  delay(1000);
}
```

**Validação (pendente):**
- [ ] Compilação OK
- [ ] `begin()` retorna true
- [ ] `getData()` retorna CSV válido
- [ ] `getAltitude()` retorna valor razoável

---

### FASE 4: LSM6DS3Sensor (Classe) ⏱️ 1.5h

**Status:** ✅ **COMPLETA (implementação)**

**Objetivos:**
- [x] Criar classe do IMU LSM6DS3
- [x] Migrar lógica de `mpu6050_sensor.h`
- [x] **IMPLEMENTAR cálculo de aceleração total** - CRÍTICO para FSM
- [x] Testar leitura de aceleração/giroscópio

**Arquivos:** `sensors/LSM6DS3Sensor.h` + `sensors/LSM6DS3Sensor.cpp`

**API Pública:**
```cpp
class LSM6DS3Sensor : public ISensor {
public:
  LSM6DS3Sensor();
  
  // Interface ISensor
  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;
  
  // Específicos do IMU
  float getAccelZ() const;
  float getTotalAccel() const;  // ⚠️ CRÍTICO: sqrt(ax² + ay² + az²)
  
private:
  Adafruit_LSM6DS3 _lsm;
  float _accelX, _accelY, _accelZ;
  float _gyroX, _gyroY, _gyroZ;
  float _totalAccel;  // Calculado em update()
};
```

**Migração:**
- ✅ API Adafruit_MPU6050 → Adafruit_LSM6DS3 (quase idêntica!)
- ✅ Manter uso de `sensors_event_t` (compatível)
- ✅ **IMPLEMENTAR cálculo de totalAccel** (referência: linha 37 do `FSM_Tester.py`)

**Cálculo de Aceleração Total (CRÍTICO):**
```cpp
// Em LSM6DS3Sensor::update()
void LSM6DS3Sensor::update() {
  sensors_event_t accel, gyro, temp;
  _lsm.getEvent(&accel, &gyro, &temp);
  
  _accelX = accel.acceleration.x;
  _accelY = accel.acceleration.y;
  _accelZ = accel.acceleration.z;
  
  _gyroX = gyro.gyro.x;
  _gyroY = gyro.gyro.y;
  _gyroZ = gyro.gyro.z;
  
  // Magnitude total da aceleração (Python linha 37)
  _totalAccel = sqrt(_accelX*_accelX + _accelY*_accelY + _accelZ*_accelZ);
}
```

**Validação (pendente):**
- [ ] Compilação OK
- [ ] `begin()` retorna true
- [ ] `getAccelZ()` detecta gravidade (~9.8 m/s²)

---

### FASE 5: GPSModule (Classe) ⏱️ 1h

**Status:** ✅ **COMPLETA**

**Objetivos:**
- [x] Encapsular GPS em classe
- [x] Manter `TinyGPSPlus` (sem mudança de biblioteca)
- [x] Testar recepção de coordenadas

**Arquivos:** `sensors/GPSModule.h` + `sensors/GPSModule.cpp`

**API Pública:**
```cpp
class GPSModule : public ISensor {
public:
  GPSModule(HardwareSerial* serial);

  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;

  String getTimeString() const;
  String getDateString() const;
};
```

**Validação:**
- [x] Compilação OK
- [x] GPS recebe NMEA sentences
- [x] `getTimeString()` retorna tempo válido após fix

---

### FASE 6: FSM - Máquina de Estados ⏱️ 4h

**Status:** ✅ **COMPLETA (PR #16)** — atualizada para refletir a implementação real

**⚠️ ATENÇÃO: Esta é a fase mais crítica - adaptação da lógica validada para 4 estados exteriores + sub-eventos**

**Objetivos (atendidos):**
- [x] Implementar FSM com **4 estados exteriores** (IDLE → ASCENT → DESCENT → LANDED)
- [x] Rastrear **7 sub-eventos** via flags booleanas (liftoff, burnout, apogee, freefall, parachute, + ASCENT/DESCENT)
- [x] Adaptar a lógica validada do `FSM_Tester.py` para o modelo simplificado
- [x] Implementar validações de segurança obrigatórias (NaN/Inf, guards mínimos)
- [x] Deploy de paraquedas no **apogeu** (Option A), não no DESCENT — ver `detectParachute()`
- [x] Testar transições com dados simulados e reais (RocketPy + voo real)

**Arquivos:** `flight/FlightStateMachine.h` + `flight/FlightStateMachine.cpp`

**Estrutura (fiel à implementação):**
```cpp
class FlightStateMachine : public ISensor {
private:
  FlightState currentState;
  BMP585Sensor* baro;
  LSM6DS3Sensor* imu;

  // Sub-event flags (set once, cleared on reset)
  bool _liftoffDetected, _burnoutDetected, _apogeeDetected,
       _freefallDetected, _parachuteDeployed;

  // Parachute deploy confirmation (Option A: apogee + stable negative Vz)
  uint8_t _parachuteConfirmCount;

  // IIR filter state for accelerometer (ALPHA=0.2)
  float _filtAx, _filtAy, _filtAz;
  bool  _firstReading;



  void transitionTo(FlightState next);

  // Detection helpers (exact port from test/FSM/FSM.ino)
  bool detectLiftoff(float ax, float ay, float az) const;
  bool detectBurnout(float ax, float ay, float az, float height, float vz) const;
  bool detectApogee(float vz, float az) const;
  bool detectFreefall(float vz, float height, float totalAcc) const;
  bool detectParachute(float height, float vz) const;  // Option A: apogee
  bool detectLanded(float vz, float height) const;

  float smoothFilter(float value, float prev) const;
  static float totalAccel(float ax, float ay, float az);

public:
  FlightStateMachine(BMP585Sensor* b, LSM6DS3Sensor* i);
  bool begin() override;
  void update() override;
  FlightState getState() const;
  const char* getStateName() const;
  void reset();
};
```

**Detecção de liftoff (one-shot + IIR, port fiel do teste validado):**
```cpp
bool FlightStateMachine::detectLiftoff(float ax, float ay, float az) const {
  float total = totalAccel(ax, ay, az);
  if (!std::isfinite(total)) return false;
  // IIR smoothing (ALPHA=0.2) + one-shot: sobe para ASCENT uma vez
  return (total > LIFTOFF_TOTAL_ACCEL_THRESHOLD);
}
```

**Detecção de apogeu / deploy de paraquedas (Option A — NÃO é "vz < 0"):**
```cpp
bool FlightStateMachine::detectParachute(float height, float vz) const {
  // Abre no APOGEU: Vz negativo confirmado por PARACHUTE_CONFIRM_CYCLES
  // ciclos, e nunca abaixo de PARACHUTE_MIN_ALTITUDE (piso de solo).
  if (height < PARACHUTE_MIN_ALTITUDE) return false;   // 50 m
  if (vz < PARACHUTE_CONFIRM_VZ) {                     // -2.0 m/s
    // contador incrementado em update(); deploy quando >= PARACHUTE_CONFIRM_CYCLES
    return (_parachuteConfirmCount >= PARACHUTE_CONFIRM_CYCLES);
  }
  return false;
}
```
> O modelo antigo desta seção usava `detectDescent()` com `vz < 0` e deploy no
> estado DESCENT. Isso foi substituído pela **Option A** (deploy no apogeu),
> validada com RocketPy (apogeu 951 m → deploy 949.5 m) e voo real
> (apogeu 272 m → deploy 268 m). Ver commits da Fase 9/Option A.
```

**Lógica de Transições (4 estados exteriores + sub-eventos):**

| De | Para | Gatilho (sub-evento) |
|----|------|--------|
| IDLE | ASCENT | `detectLiftoff()` (totalAccel > limiar, one-shot) |
| ASCENT | DESCENT | `detectApogee()` (Vz cruza zero / pico) — seta flag `apogee` |
| DESCENT | LANDED | `detectLanded()` (Vz~0 e altura estável por guard) |

Sub-eventos rastreados por flags (não mudam o estado exterior, só diagnóstico):
`liftoff`, `burnout` (fim de empuxo), `apogee`, `freefall`, `parachute`.

**Deploy de paraquedas (Option A — no APOGEU, não no DESCENT):**
O deploy é acionado pela FlightControlTask quando `detectParachute()` confirma
apogeu + Vz negativo estável (contador `PARACHUTE_CONFIRM_CYCLES`), respeitando
o piso `PARACHUTE_MIN_ALTITUDE` (50 m, só guarda de solo). O estado vai para
PARACHUTE após o acionamento.

**Loop Principal (`update()`):**
```cpp
void FlightStateMachine::update() {
  if (!_ready) return;
  // lê sensores, aplica IIR, roda detect* na ordem dos sub-eventos
  // transitionTo() nos limiares; detectParachute() incrementa o contador
  // e sinaliza deploy (FlightControlTask efetua ParachuteServo.write)
}
```

**Loop Principal (`checkTransitions()` — port fiel da implementação):**
```cpp
void FlightStateMachine::checkTransitions() {
  switch (currentState) {
    case IDLE:
      if (detectLiftoff(_filtAx, _filtAy, _filtAz)) {
        _liftoffDetected = true;
        transitionTo(ASCENT);
      }
      break;

    case ASCENT:
      if (detectApogee(_vz, _az)) {
        _apogeeDetected = true;
        transitionTo(DESCENT);   // apogeu = topo
      }
      break;

    case DESCENT:
      // detectParachute() (Option A) é consultado pela FlightControlTask;
      // aqui apenas transições de estado:
      if (detectLanded(_vz, _height)) {
        transitionTo(LANDED);
      }
      break;

    case LANDED:
      // Estado final
      break;
  }
}
```


**Validação:**
- [ ] Todas as 3 transições funcionam (IDLE→ASCENT→DESCENT→LANDED)
- [ ] Validações de segurança (NaN/Inf) impedem crashes
- [ ] Guards mínimos evitam false positives
- [ ] Simulação com dados do CSV (13_30_11-Dados.csv)
- [ ] Logs de transição claros com timestamps

---

### FASE 7: FreeRTOS Tasks ⏱️ 4h

**Status:** ✅ **COMPLETA** (Tasks 1, 2 e 3 implementadas)

**Objetivos:**
- [x] Implementar Task 1 (FlightControl - 50Hz)
- [x] Implementar Task 2 (Telemetry - 5Hz)
- [x] Implementar Task 3 (Logger - low priority)
- [x] Criar Queues de comunicação (sensorDataQueue 25 slots, logQueue 50 slots)
- [x] Configurar Watchdog (5s, apenas Task FlightControl)

**Arquivos:**
- `flight/FlightControlTask.h/cpp`
- `flight/TelemetryTask.h/cpp`
- `flight/LoggerTask.h/cpp`

**Task 1 - FlightControl:**
```cpp
void taskFlightControl(void* parameter) {
  esp_task_wdt_add(NULL);  // Watchdog
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  while(1) {
    esp_task_wdt_reset();
    
    baroSensor->update();
    imuSensor->update();
    flightFSM->update();
    
    if (flightFSM->getState() == DESCENT && !parachute_deployed) {
      deployParachute();
      parachute_deployed = true;
      
      LogMessage log = {"[CRITICAL] Parachute deployed at DESCENT!", millis(), 1, 2};
      xQueueSend(logQueue, &log, 0);
    }
    
    SensorData data = { /* ... */ };
    xQueueSend(sensorDataQueue, &data, 0);
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
  }
}
```

**Task 2 - Telemetry:**
```cpp
void taskTelemetry(void* parameter) {
  SensorData data;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  while(1) {
    gpsModule->update();
    
    if (xQueueReceive(sensorDataQueue, &data, 0) == pdPASS) {
      String telemetry = assembleTelemetry(data);
      
      Serial.println(telemetry);
      sendLoRa(telemetry);
      appendFile(file_dir, telemetry);
    }
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
  }
}
```

**Task 3 - Logger:**
```cpp
void taskLogger(void* parameter) {
  LogMessage log;
  
  while(1) {
    if (xQueueReceive(logQueue, &log, portMAX_DELAY) == pdPASS) {
      Serial.printf("[%lu][T%d] %s\n", 
        log.timestamp, log.taskId, log.message);
      
      #ifdef LOG_TO_FILE
        appendFile("/logs/flight.log", log.message);
      #endif
    }
  }
}
```

**Configuração:**
```cpp
// Watchdog
esp_task_wdt_init(5, true);  // 5s timeout, panic on trigger

// Queues
sensorDataQueue = xQueueCreate(25, sizeof(SensorData));
logQueue = xQueueCreate(50, sizeof(LogMessage));

// Tasks
xTaskCreatePinnedToCore(taskFlightControl, "FlightCtrl", 8192, NULL, 20, NULL, 1);
xTaskCreatePinnedToCore(taskTelemetry, "Telemetry", 16384, NULL, 5, NULL, 0);
xTaskCreatePinnedToCore(taskLogger, "Logger", 4096, NULL, 1, NULL, 0);
```

**Validação:**
- [ ] 3 tasks executando em paralelo
- [ ] Queue não overflow (monitorar `uxQueueMessagesWaiting()`)
- [ ] Watchdog reseta se Task 1 travar (testar!)

---

### FASE 8: Integração firmware.ino ⏱️ 2h

**Status:** ✅ Concluída

**Objetivos (atendidos):**
- [x] Refatorar `firmware.ino` principal (apenas setup/loop + includes)
- [x] Criar tasks via `init*Task()` functions (cada task possui seus objetos, filas e watchdog)
- [x] `loop()` vazio (tasks assumem controle via FreeRTOS)
- [x] Remover globais órfãs do `.ino` (sensores/filas vivem dentro das init functions)

**Decisões de arquitetura (revisadas vs. rascunho original):**
- Os objetos (`g_baro`, `g_imu`, `g_fsm`, `g_gps`, filas) NÃO são globais no
  `.ino`; cada `init*Task()` os cria e os encapsula. O `firmware.ino` só chama
  `initFlightControlTask()` / `initTelemetryTask()` / `initLoggerTask()`.
- O **watchdog (TWDT) é armado DENTRO de `taskFlightControl`**, não no
  `setup()`. Motivo: evita um TWDT globalmente armado sem nenhuma task para
  chamar `esp_task_wdt_reset()` se o `xTaskCreatePinnedToCore` falhar.
- As filas usam constants em `config.h` (`SENSOR_DATA_QUEUE_LEN`,
  `LOG_QUEUE_LEN`), não hardcoded.
- `TelemetryTask` drena a `sensorDataQueue` mantendo só a amostra mais nova
  (a 50Hz produz, a 5Hz consome — descarta as intermediárias, não perde a
  última). Não usa `xQueueReceive(..., 0)` simples que descartaria dados.
- Stack sizes e pinagem de core vêm de `config.h` (FLIGHT_CONTROL_STACK_SIZE,
  FLIGHT_CONTROL_CORE, etc.).

**Estrutura (fiel à implementação):**
```cpp
#include <Wire.h>
#include <SPI.h>

#include "sensors/BMP585Sensor.h"
#include "sensors/LSM6DS3Sensor.h"
#include "sensors/GPSModule.h"
#include "flight/FlightStateMachine.h"
#include "flight/FlightControlTask.h"
#include "flight/TelemetryTask.h"
#include "flight/LoggerTask.h"

#include "modules/buzzer_module.h"
#include "modules/filesystem_module.h"
#include "modules/lora_module.h"
#include "modules/parachute_module.h"

// Nenhuma global de sensor/fila aqui: cada init*Task() as cria e encapsula.

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);

  if (!initFlightControlTask()) {        // cria g_baro/g_imu/g_fsm, sensorDataQueue, servo
    Serial.println("FATAL: FlightControl init failed");
    ESP.restart();
  }
  if (!initTelemetryTask()) {            // cria g_gps, consome sensorDataQueue, LoRa/file fan-out
    Serial.println("FATAL: Telemetry init failed");
    ESP.restart();
  }
  if (!initLoggerTask()) {               // cria logQueue, logger Serial/file
    Serial.println("FATAL: Logger init failed");
    ESP.restart();
  }
  // O TWDT e' armado DENTRO de taskFlightControl (apos a task existir de fato).
}

void loop() {
  vTaskDelay(portMAX_DELAY);  // Tasks controlam tudo
}
```

**Nota:** o watchdog NÃO é armado no `setup()` (o rascunho original sugeria
`esp_task_wdt_init(5, true)` global). A implementação real o arma dentro de
`taskFlightControl` para não deixar um TWDT armado sem task para resetá-lo.

**Validação:**
- [ ] Compilação OK
- [ ] Sistema inicializa sem erros
- [ ] Tasks executando corretamente

---

### FASE 9: Adaptar Módulos Dependentes ⏱️ 1.5h

**Status:** ⏳ Pendente

**Objetivos:**
- [ ] Atualizar `parachute_module.h` para usar classes
- [ ] Remover código antigo (bmp280_sensor.h, mpu6050_sensor.h, gps_module.h)
- [ ] Validação final

**Módulos a Adaptar:**

**`modules/parachute_module.h`:**
- Antes: `if (altitude < max_altitude - DESCENT_THRESHOLD)`
- Depois: `void handleParachute(BMP585Sensor* baro)`

**Deletar:**
- ❌ `bmp280_sensor.h`
- ❌ `mpu6050_sensor.h`
- ❌ `gps_module.h`

**Validação Final:**
- [ ] Nenhum módulo acessa variáveis globais de sensores
- [ ] Compilação limpa (sem warnings)
- [ ] Código antigo removido

---

### FASE 10: Comunicação com o Receiver ⏱️ 2h

**Status:** ✅ Concluída

**Contexto:**
O flight computer transmite telemetria via LoRa para o receiver
(`recovery-webui/components/receiver-lora/firmware/`). O receiver é um ESP32
separado que recebe pacotes CSV, faz parse dos campos, adiciona hora/data do
GPS local, e retransmite para o WebUI. Qualquer mudança no formato de
telemetria do flight computer DEVE ser refletida no receiver para manter
compatibilidade — foi feito na Opção B (formato v2.0 conjunto, 22 campos no
satellite / 24 no protocolo do receiver).

**Decisões da Fase 10:**
- Formato v2.0 definido campo a campo em `docs/telemetry-format.md`
  (referência única). O rascunho antigo desta seção estava dessincronizado
  (`p` em Pa; `packetQuality` no índice 20; caminho errado do receiver) — a
  fonte de verdade agora é o `telemetry-format.md`.
- Pressão (`p`) emitida em **hPa** (BMP585), não Pa.
- `rssi` do satellite é placeholder `0`; o receiver substitui pelo RSSI real
  do link descendente (`LoRa.packetRssi()`).
- `umi` (umidade) é `0` fixo — ainda não há sensor de umidade.
- Rádio alinhado ao receiver: `LORA_FREQ=915E6`, `SYNC_WORD=0xF3`,
  `LORA_SF=7`, `LORA_BW=125E3`, `LORA_CR=5`, `LORA_TX_POWER=17`, CRC on.
  (Antes o flight usava 868E6 — nem conectava com o receiver em 915E6.)

**Objetivos (atendidos):**
- [x] Definir formato de telemetria final da v2.0 (campo a campo)
- [x] Escrever formato em `docs/telemetry-format.md` (referência única)
- [x] `TelemetryTask::assembleTelemetry` monta o pacote no formato acordado (22 campos)
- [x] Parser do receiver atualizado (`parseSatellitePacket` → 22 campos na ordem v2.0)
- [x] `rssi` real: receiver usa `LoRa.packetRssi()` no protocolPacket
- [x] Validar ponta-a-ponta: `extras/validate_telemetry_format.py` (PASS)

**Arquivos alterados:**
- `firmware/config.h` — frequência 915E6 + SF/BW/CR/TX_POWER
- `firmware/modules/lora_module.h` — `setupLoRa()` aplica os parâmetros + CRC
- `firmware/flight/TelemetryTask.cpp` — `assembleTelemetry` reordenado (22 campos)
- `recovery-webui/.../firmware/src/main.cpp` — `parseSatellitePacket` 22 campos + `buildProtocolPacket`
- `recovery-webui/.../firmware/include/payload.h` — protocolo 24 campos (v2.0)
- `recovery-webui/.../firmware/arduino/receiver-lora/receiver-lora.ino` — mirror legado alinhado

**Referências:**
- `docs/telemetry-format.md` — formato v2.0 (fonte única)
- `recovery-webui/components/receiver-lora/firmware/src/main.cpp` — Parser CSV do receiver
- `extras/validate_telemetry_format.py` — validação E2E do formato

### Checklist de Funcionalidades

**Sensores:**
- [ ] BMP585 lê altitude/pressão/temperatura
- [ ] LSM6DS3 lê aceleração/giroscópio (6 eixos)
- [ ] GPS recebe fix e coordenadas
- [ ] Calibração de base_pressure funciona

**FSM (4 estados):**
- [ ] Transição IDLE → ASCENT (totalAccel > limiar)
- [ ] Transição ASCENT → DESCENT (vz < 0 persistente)
- [ ] Transição DESCENT → LANDED (vz ~ 0 e altitude estável)
- [ ] Validações de segurança (NaN/Inf) funcionando
- [ ] Guards mínimos evitando false positives

**FreeRTOS:**
- [ ] Task FSM roda a 50Hz preciso
- [ ] Task Telemetry roda a 5Hz
- [ ] Watchdog reseta se Task FSM travar
- [ ] Queue não overflow durante operação normal

**Telemetria:**
- [ ] Dados transmitidos via Serial
- [ ] Dados transmitidos via LoRa
- [ ] Dados gravados em `/data/HHMMSS-data.csv`
- [ ] Logs gravados em `/logs/HHMMSS-log.txt` (se habilitado)

**Paraquedas:**
- [ ] Deploy aciona no estado DESCENT
- [ ] Servo move para posição correta
- [ ] Buzzer toca sinal "Activated"

**Receiver (integração):**
- [ ] Formato de telemetria documentado em `docs/telemetry-format.md`
- [ ] Parser do receiver compatível com formato v2.0
- [ ] `rssi` real transmitido (não hardcoded -1)
- [ ] Validação ponta-a-ponta (flight computer -> receiver -> WebUI)


### Testes de Estresse

**Simulações:**
- [ ] Atraso Task Telemetry (500ms) - Task FSM continua?
- [ ] Sensor travado - Watchdog reseta sistema?
- [ ] Queue cheia - Sistema continua operando?
- [ ] Voo completo simulado (IDLE → LANDED)

**Hardware:**
- [ ] Teste de bancada (sensores reais)
- [ ] Teste de queda livre (simular apogee)
- [ ] Teste de vibração (simular liftoff)
- [ ] Validação de dados CSV gerados

---

## 📊 Uso de Recursos

### Memória Estimada

| Componente | RAM | Flash |
|------------|-----|-------|
| Código base | ~50 KB | ~200 KB |
| Bibliotecas Adafruit | ~10 KB | ~50 KB |
| FreeRTOS overhead | ~2 KB | ~20 KB |
| Task stacks (3 × 8KB avg) | 28 KB | - |
| Queues (25+50 slots) | ~8 KB | - |
| Objetos (sensors, FSM) | ~5 KB | - |
| **TOTAL** | **~103 KB** | **~270 KB** |

**Disponível:**
- RAM: 512 KB → Uso: ~20% ✅
- Flash: 8 MB → Uso: ~3% ✅

### Performance

| Métrica | Valor | Limite |
|---------|-------|--------|
| Task FSM cycle time | ~5-10ms | 20ms (50Hz) |
| Task Telemetry cycle | ~50-100ms | 200ms (5Hz) |
| Queue send latency | <1ms | - |
| Watchdog timeout | 5000ms | - |

---

## ⚠️ Riscos e Mitigações

| Risco | Prob. | Impacto | Mitigação |
|-------|-------|---------|-----------|
| API Adafruit_BMP5xx diferente | Média | Alto | Testar biblioteca ANTES (Fase 3) |
| Deadlock entre tasks | Baixa | Alto | Nunca usar `portMAX_DELAY` em Task FSM |
| Queue overflow | Média | Médio | Monitorar `uxQueueMessagesWaiting()` |
| Watchdog falso positivo | Baixa | Alto | Testar com timeout maior (10s) primeiro |
| Watchdog reset no meio do voo | Baixa | **Crítico** | **RESOLVIDO**: FSM persiste estado + base_pressure em NVS (`flight/fsm`); no boot, `restoreFromNVS()` retoma ASCENT/DESCENT e o paraquedas abre no apogeu mesmo após reset (validado: `extras/FSM_tester/validate_watchdog_reboot.py`) |
| FSM presa em estado errado (bug/edge case) | Média | **Crítico** | **RESOLVIDO**: backstop de queda livre independente da FSM no FlightControlTask — acc < 3 m/s² por 1s + vz < -5 m/s + h > 50m abre o paraquedas mesmo com a FSM presa em IDLE/ASCENT (validado: `extras/FSM_tester/validate_freefall_backstop.py`) |
| Deploy do paraquedas na subida (falso positivo) | Baixa | Crítico | Guard `vz < -5 m/s` no backstop separa queda real de burnout/coasting (accel ~0 logo após o motor parar); validado em dados reais |
| GPS sem fix indoor | Alta | Baixo | Usar fallback "NOFIX" para filename |
| Filesystem lento | Média | Baixo | OK, Task 2 pode atrasar |

### Recuperação pós-reset (Watchdog) — implementada em v2.0

Comportamento após um reset por watchdog **no meio do voo**:

1. `FlightStateMachine::begin()` chama `restoreFromNVS()`:
   - Sem snapshot válido (1º boot, ou voo anterior terminou em LANDED → snapshot limpo) → FSM começa fresca em IDLE.
   - Snapshot válido em ASCENT/DESCENT → FSM **retoma do estado salvo** (flags + contador de confirmação), e a `base_pressure` do local de lançamento é restaurada no BMP585 → a altitude continua **absoluta ao solo de lançamento** (sem o re-nivelamento que fazia o firmware "esquecer" a altura).
2. Snapshot é gravado em toda transição de estado, no deploy do paraquedas e uma vez no boot. `reset()` limpa o snapshot.
3. `setupServo(keepOpen)`: se o snapshot diz que o paraquedas já foi acionado, o servo **não é fechado** no boot (fechar o compartimento com o paraquedas aberto em voo o soltaria).

Validação: `extras/FSM_tester/validate_watchdog_reboot.py` — reboots em 5 fases do voo simulado (queima, coasting, pré-apogeu, pós-apogeu, pós-deploy) + 4 fases do voo real: **sem o fix o paraquedas nunca abre** (FSM presa em IDLE); **com o fix abre no apogeu em todos os cenários** (949.5 m simulado, 268 m real).

> ⚠️ Limitações conhecidas: um reboot durante a queima só rearma se a aceleração > 15 m/s² persistir após o boot; snapshot "stale" (aborte antes de LANDED) pode restaurar ASCENT no solo — a guarda `PARACHUTE_MIN_ALTITUDE` impede deploy falso em solo.

### Backstop de queda livre (independente da FSM) — implementado em v2.0

Segunda camada de segurança, complementar ao NVS: cobre o caso em que a FSM está **viva mas presa no estado errado** (ex.: snapshot NVS restaurado em ASCENT com o foguete caindo de verdade; FSM em IDLE sem snapshot válido). O NVS não cobre esse caso — o backstop sim.

No loop do FlightControlTask (50Hz), `checkFreefallBackstop()`:

```
totalAccel (IIR α=0.2, filtro próprio) < 3.0 m/s²   por 50 ciclos consecutivos (1.0s)
        AND vz < -5.0 m/s                            (descendo de verdade)
        AND altitude > 50 m                          (guarda de solo)
        → deployParachute()  (idempotente via g_parachuteActuated)
```

Decisões de projeto (fundamentadas em dados reais):
- **`vz < -5 m/s` é obrigatório**: o accel cai para ~0g logo após o burnout (foguete ainda subindo) — no voo real a janela zero-g começa 4s ANTES do apogeu. Sem a condição de velocidade, o paraquedas abriria na subida a ~190 m.
- **Janela de 1.0s**: janelas zero-g reais duram 8–131s (sobra tempo), spikes de vibração em solo são transitórios (22–122 m/s²) e são rejeitados pela janela.
- **Filtro IIR próprio** (não usa estado da FSM): funciona mesmo com a FSM corrompida.
- Em voo normal o backstop dispara 1–3s APÓS o deploy da FSM (nunca antes); com a FSM presa, abre a 922 m (simulado) / 255 m (real) — sempre acima do piso.

Validação: `extras/FSM_tester/validate_freefall_backstop.py` — cenários A (FSM ok: backstop inofensivo), B (FSM presa em IDLE: backstop abre no apogeu), C (sem falso positivo no burnout), D (bancada: nunca dispara). Todos PASS nos 2 datasets de voo + bancada.

---

## 🐛 Troubleshooting

### Problemas Comuns

**"BMP585 initialization failed"**
- Verificar conexões I2C (SDA/SCL)
- Testar com I2C scanner
- Verificar endereço I2C (0x76 ou 0x77)

**"Task watchdog got triggered"**
- Task FSM travou (sensor não responde?)
- Aumentar timeout temporariamente
- Adicionar logs antes de operações bloqueantes

**"Queue full" ou dados perdidos**
- Task Telemetry muito lenta (filesystem?)
- Aumentar queue size (25 → 50)
- Otimizar Task 2 (remover delays desnecessários)

**FSM não transita de estado**
- Verificar thresholds em `config.h`
- Adicionar logs de debug nas condições
- Testar com valores simulados

---

## 📚 Referências

### Documentação
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [FreeRTOS API Reference](https://www.freertos.org/a00106.html)
- [Adafruit BMP5xx Guide](https://learn.adafruit.com/adafruit-bmp5xx)
- [Adafruit LSM6DS Guide](https://learn.adafruit.com/lsm6ds)
- [TinyGPS++ Documentation](http://arduiniana.org/libraries/tinygpsplus/)

### Código de Referência
- [firmware.ino original](firmware.ino) - Código procedural (EOL)
- [MODULOS.md](MODULOS.md) - Documentação módulos antigos
- **[extras/FSM_tester/FSM_Tester.py](../extras/FSM_tester/FSM_Tester.py)** - ⚠️ **CRÍTICO:** FSM validado em Python
- **[extras/FSM_tester/explicacao.md](../extras/FSM_tester/explicacao.md)** - Documentação completa do FSM
- **[extras/FSM_tester/13_30_11-Dados.csv](../extras/FSM_tester/13_30_11-Dados.csv)** - Dataset de validação (1,873 pontos)

---

## 📝 Changelog

### 2026-08-04 - Backstop de queda livre (independente da FSM)
- ✅ `checkFreefallBackstop()` no FlightControlTask: acc < 3 m/s² (IIR próprio) por 1.0s + vz < -5 m/s + h > 50 m → deploy idempotente
- ✅ Cobre FSM viva mas presa em estado errado (gap do NVS); nunca dispara na subida (guard vz) nem em solo (janela + piso)
- ✅ Validado em Python antes do C++: `extras/FSM_tester/validate_freefall_backstop.py` (A: inofensivo em voo normal, B: abre com FSM presa, C: sem falso positivo no burnout, D: bancada) — PASS em todos
- ✅ Thresholds em `config.h` (`FREEFALL_BACKSTOP_*`) dimensionados com dados reais

### 2026-08-04 - Recuperação pós-reset por Watchdog (NVS)
- ✅ FSM persiste snapshot em NVS (`Preferences`, namespace `flight`/chave `fsm`): estado, flags, contador de confirmação, `base_pressure` do lançamento e `maxAltitude`
- ✅ `restoreFromNVS()` em `begin()`: retoma ASCENT/DESCENT após reboot no meio do voo; snapshot LANDED é limpo (boot fresco); sem snapshot → IDLE
- ✅ `BMP585Sensor::setBasePressure()`/`setMaxAltitude()`: altitude continua absoluta ao solo de lançamento após o reset (sem re-nivelamento no ponto do reboot)
- ✅ `setupServo(keepOpen)`: servo não é fechado no boot se o paraquedas já foi acionado
- ✅ Novo validador `extras/FSM_tester/validate_watchdog_reboot.py` (PASS em todos os cenários: 5 fases do voo simulado + 4 do voo real)

### 2026-08-05 - Revisão de riscos de voo real (5 riscos, batch de subagentes)
- ✅ **Risco 1 (barômetro congela em voo) — resolvido**: `BMP585Sensor::getLastReadingAgeMs()` + contingência IMU-only `checkBaroStaleContingency()` no FlightControlTask: barômetro sem leitura válida por >2s + liftoff já visto (acc > 15 m/s²) + maxAltitude (último valor bom) > 50 m + acc filtrado < 3 m/s² por 2.5s → deploy. Validado em Python antes do C++ (`validate_baro_stale.py`, cenários A-D — PASS; contingência abre em t=19.6s/12.4s acima do piso, inofensiva em voo normal e em bancada)
- ✅ **Risco 3 (apogeu perdido por ruído de vz a 50Hz) — NÃO confirmado**: novo `validate_50hz_noise.py` re-amostra os 2 datasets a 50Hz, injeta ruído de quantização do barômetro (até 20x o real) e mede: apogeu detectado em 100% dos runs, 0 deploys prematuros, 0 deploys >5s após o apogeu → nenhuma mudança de firmware necessária
- ✅ **Risco 5 (bamboleio pós-burnout impede apogeu) — resolvido**: análise `analyze_apogee_robustness.py` confirmou o risco no voo real (gate de az tinha margem de apenas ~0.81 m/s²; perdia o apogeu com >=1.1 m/s² de bamboleio pendular). **Gate de az removido**: `detectApogee()` agora é só `|vz| < 1.0` (config.h `APOGEE_MAX_VZ`) — vz vem do barômetro, imune ao bamboleio por construção. Portas Python dos 3 validadores atualizadas; varredura pós-mudança: imune até >20 m/s² em todas as frequências. Follow-up: investigar calibração do acelerômetro do voo real (az em repouso +2.81 m/s²)
- ✅ Subagentes deixaram diffs sem commit; revisão, validação e commits centralizados

### 2026-03-18 - Planejamento Completo (v1.0)
- ✅ Arquitetura definida (POO + FreeRTOS + FSM)
- ✅ Decisões técnicas tomadas
- ✅ Plano de 9 fases criado
- ✅ Estimativas de tempo e recursos

### 2026-03-18 - Atualização v2.0 (FSM Simplificada)
- ✅ FSM alinhado para **4 estados** (IDLE → ASCENT → DESCENT → LANDED)
- ✅ Thresholds validados com dados reais (1,873 pontos de telemetria)
- ✅ Referências linha-por-linha ao código Python
- ✅ Validações de segurança obrigatórias documentadas
- ✅ Cálculos de Vz e totalAccel especificados
- ✅ Tempo total ajustado: 16.25h → 17.25h (Fase 6 aumentada)

### 2026-06-24 - Fase 6 Concluída (FSM 4 estados)
- ✅ `FlightStateMachine` classe implementada (PR #16)
- ✅ 4 estados: IDLE → ASCENT → DESCENT → LANDED
- ✅ Sub-eventos: liftoff, burnout, apogee, freefall, parachute
- ✅ Thresholds validados com dados reais (1,873 pontos)
- ✅ Validações NaN/Inf em todas as entradas
- ✅ Vz corrigido para m/s (divisão por 1000.0F em BMP585Sensor)
- ✅ IIR filter (alpha=0.2) com seed na primeira leitura

### Próximas Etapas
- ✅ Fase 7 concluída (FreeRTOS Tasks: FlightControl @50Hz, Telemetry @5Hz, Logger)
- ✅ Fase 8 concluída (Integração firmware.ino — setup() orquestra init*Task())
- ✅ Fase 9 concluída (Módulos adaptados: parachute, lora, filesystem, buzzer)
- ✅ Fase 10 concluída (Formato de telemetria 22 campos alinhado com receiver)

Todas as 10 fases da v2.0 foram concluídas. O firmware está migrado para
OOP + FreeRTOS + FSM. Consulte `CHANGELOG.md` para o histórico completo.

---

## 🤝 Contribuindo

### Para Desenvolvedores

**Antes de começar uma fase:**
1. Ler esta documentação completa
2. Marcar fase como "Em Progresso" (⏳ → 🔄)
3. Criar branch específica: `git checkout -b fase-N-descricao`

**Durante implementação:**
1. Seguir estrutura proposta
2. Fazer commits pequenos e frequentes
3. Testar cada componente isoladamente

**Ao concluir fase:**
1. Marcar como "Concluída" (🔄 → ✅)
2. Validar checklist da fase
3. Fazer PR para branch principal
4. Atualizar este documento

### Para Agentes de Código

**Você pode usar este documento para:**
- Entender arquitetura do sistema
- Consultar decisões técnicas
- Verificar progresso das fases
- Tirar dúvidas sobre implementação
- Encontrar referências e exemplos

**Ao fazer modificações:**
- Sempre atualizar este documento
- Manter coerência com decisões tomadas
- Seguir estrutura de pastas definida
- Respeitar convenções de nomenclatura

---

## 📞 Contato

**#11 Avionics**  
**Projeto:** Flight Computer  
**Repositório:** `/home/vinicius/Documentos/Projetos/flight-computer/`

---

**Última atualização:** 2026-06-24  
**Versão do documento:** 2.3  
**Status geral:** 🚀 Todas as 10 fases concluídas — v2.0 operacional

---

## ⚠️ ATUALIZAÇÃO v2.0: FSM Simplificada (4 Estados)

**IMPORTANTE:** Este documento foi atualizado para refletir a **FSM de 4 estados** adotada na v2.0.

**Fonte de Validação:**
- Arquivo: `extras/FSM_tester/FSM_Tester.py` (implementação Python completa)
- Dataset: `extras/FSM_tester/13_30_11-Dados.csv` (1,873 pontos de telemetria)
- Documentação: `extras/FSM_tester/explicacao.md` (lógica completa com referências de linha)

**Mudanças principais:**
1. FSM agora possui **4 estados**: `IDLE → ASCENT → DESCENT → LANDED`
2. Estados intermediários do modelo anterior de 7 estados foram consolidados
3. Validações de segurança obrigatórias permanecem (NaN/Inf checks, guards mínimos)
4. Lógica foi simplificada para facilitar integração na arquitetura atual
