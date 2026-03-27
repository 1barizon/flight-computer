# Diferenças entre testes funcionais e planejamento firmware/extras

**Session ID:** ses_2cfc032b2ffermYLHw8LsIIiYC
**Created:** 3/27/2026, 2:03:00 PM
**Updated:** 3/27/2026, 2:29:34 PM

---

## User

Veja que agora temos em /tests uma versão funcional e aprovada para a máquina de estados. compare com o planejamento em /firmware e /extras e me aponte as principais diferenças.

---

## Assistant (Plan · mimo-v2-pro-free · 190.2s)

_Thinking:_

The user wants me to compare the functional state machine implementation in /tests with the planning in /firmware and /extras, and point out the main differences. Let me first explore the repository structure to understand what we're working with.

**Tool: task**

**Input:**
```json
{
  "description": "Explore repo structure",
  "prompt": "Explore the repository at /home/viniciusmonnerat/Documentos/Projetos/flight-computer to understand the overall structure. List all top-level directories and files, then explore /tests, /firmware, and /extras directories in detail. Return:\n1. Full directory tree of /tests\n2. Full directory tree of /firmware\n3. Full directory tree of /extras\n4. Key files in each directory with brief descriptions",
  "subagent_type": "explore"
}
```

**Output:**
```
task_id: ses_2cfc0190effeXYxLOCiFHgrgQf (for resuming to continue this task if needed)

<task_result>
## Repository Structure Overview

The repository is a **flight computer** project for Serra Rocketry (Team #100). It's a V2 development of an avionics system built on ESP32, featuring real-time sensor data collection, parachute deployment, telemetry via LoRa, and data logging.

### Top-Level Contents
```
/home/viniciusmonnerat/Documentos/Projetos/flight-computer/
├── .git/                  # Git repository metadata
├── .gitignore             # Git ignore rules
├── CHANGELOG.md           # Project changelog (Keep a Changelog format)
├── CONTRIBUTING.md        # Contribution guidelines
├── README.md              # Project overview and design goals
├── docs/                  # Documentation (diagrams, hardware/software docs)
├── extras/                # Extra tools and test data
├── firmware/              # Main flight computer firmware (Arduino/ESP32)
├── hardware/              # Hardware files (currently empty with .gitkeep)
└── test/                  # Test sketches (note: directory is "test", not "tests")
```

---

## 1. Full Directory Tree of `/test`

```
/test/
├── .gitkeep                        # Placeholder to keep directory in git
└── FSM/
    └── FSM.ino                     # Arduino sketch for testing Finite State Machine logic
```

---

## 2. Full Directory Tree of `/firmware`

```
/firmware/
├── .gitkeep                        # Placeholder to keep directory in git
├── bmp280_sensor.h                 # BMP280 barometric pressure sensor module
├── buzzer_module.h                 # Piezoeuzzer feedback module
├── config.h                        # Global configuration (pins, thresholds, WiFi, etc.)
├── filesystem_module.h             # LittleFS filesystem management module
├── firmware.ino                    # Main Arduino sketch entry point
├── flight/                         # [Empty] Planned: FSM + FreeRTOS tasks (future refactoring)
├── gps_module.h                    # GPS module control and data acquisition
├── lora_module.h                   # LoRa wireless communication module
├── MODULOS.md                      # Documentation of modular firmware structure (Portuguese)
├── mpu6050_sensor.h                # MPU6050 IMU sensor (accelerometer + gyroscope) module
├── parachute_module.h              # Parachute deployment control module
├── REFACTORING_PLAN.md             # Detailed 9-phase refactoring plan (POO + FreeRTOS + FSM)
├── sensors/                        # [Empty] Planned: Sensor abstraction classes (future refactoring)
├── server_module.h                 # WiFi AP and web server for data access
└── telemetry_module.h              # Telemetry data aggregation, logging, and transmission
```

---

## 3. Full Directory Tree of `/extras`

```
/extras/
├── .gitkeep                        # Placeholder to keep directory in git
└── FSM_tester/
    ├── 13_30_11-Dados.csv          # Real flight telemetry data (1,873 data points)
    ├── dados_filtrados.csv         # Filtered real flight data (61 points, decimated)
    ├── dados_simulados.csv         # Simulated flight data (1,510 points, ideal trajectory)
    ├── explicacao.md               # Comprehensive documentation of FSM logic and thresholds
    ├── flight_inserter.py          # Python script to send CSV data over serial to ESP32 for testing
    ├── FSM_Tester.py               # Python FSM simulator for validating flight state detection logic
    └── OpenCSV.ipynb               # Jupyter notebook for CSV data analysis
```

---

## 4. Key Files in Each Directory with Brief Descriptions

### `/test/` - Test Sketches

| File | Description |
|------|-------------|
| `/test/FSM/FSM.ino` | Arduino sketch implementing a **4-state FSM** (IDLE, ASCENT, DESCENT, LANDED) that receives sensor data via Serial (CSV format: `millis,altp,ax,ay,az`), processes it with filtering, detects flight events (liftoff, burnout, apogee, freefall, parachute), and prints status. Includes automatic reset capability for multiple test flights. |

### `/firmware/` - Main Flight Computer Firmware

| File | Description |
|------|-------------|
| `/firmware/firmware.ino` | **Main entry point**. Initializes all modules (I2C, SPI, sensors, LoRa, filesystem, WiFi server) in `setup()` and runs a 5Hz data logging loop in `loop()` that reads altitude, calculates velocity, logs telemetry, and handles parachute deployment. |
| `/firmware/config.h` | **Global configuration file**. Defines pins (LoRa SS/RST/DIO0, servo, buzzer, GPS RX/TX), timing interval (200ms), LoRa frequency (868MHz), parachute thresholds (altitude drop, altitude, velocity), team ID ("#100"), WiFi credentials, and filesystem paths. |
| `/firmware/bmp280_sensor.h` | **BMP280 barometric sensor module**. Provides `setupBMP()` for initialization/calibration, `BMPData()` for formatted CSV output (altitude, temperature, humidity placeholder, pressure), and `checkHighest()` for tracking maximum altitude (apogee detection). |
| `/firmware/mpu6050_sensor.h` | **MPU6050 IMU module**. Provides `setupMPU()` for initialization and `MPUData()` for 6-axis CSV output (gyro pitch/roll/yaw + accelerometer pitch/roll/yaw). |
| `/firmware/gps_module.h` | **GPS module**. Provides `setupGPS()` for Serial1 initialization, `getGPSTimeString()` for filename generation, and `GPSData()` for formatted GPS data (time, date, altitude, lat/lon, satellites). Uses TinyGPS++ library. |
| `/firmware/lora_module.h` | **LoRa communication module**. Provides `setupLoRa()` for initialization with sync word and `sendLoRa()` for packet transmission. |
| `/firmware/filesystem_module.h` | **LittleFS filesystem module**. Provides `setupLittleFS()` for mounting, `writeFile()` for creating files, and `appendFile()` for adding data to existing files. Used for CSV telemetry logging. |
| `/firmware/parachute_module.h` | **Parachute deployment control**. Provides `setupServo()` to initialize servo in locked position and `handleParachute()` implementing multi-condition deployment logic (descent detection, altitude/velocity thresholds, servo verification). |
| `/firmware/buzzer_module.h` | **Audio feedback module**. Provides `buzzSignal()` generating different tone patterns: "Alert" (5 beeps), "Success" (3 beeps), "Activated" (long beep), "Beep" (short beep). |
| `/firmware/server_module.h` | **WiFi AP and web server**. Creates WiFi access point and serves web interface with REST API endpoints: `GET /api/files` (list files), `GET /api/file?filename=X` (retrieve/download file), `DELETE /api/file?filename=X` (delete file). |
| `/firmware/telemetry_module.h` | **Telemetry aggregation and logging**. Provides `getDataString()` combining all sensor data, `printBoth()` for dual Serial+LoRa output, and `logData()` for complete telemetry packet assembly, transmission, and storage. |
| `/firmware/MODULOS.md` | **Module documentation** (Portuguese). Describes each of the 11 modular firmware files, their functions, and advantages of the modular architecture. |
| `/firmware/REFACTORING_PLAN.md` | **Comprehensive 9-phase refactoring plan** for migrating from procedural code to OOP (selective), adding FreeRTOS tasks (3 tasks: FlightControl 50Hz, Telemetry 5Hz, Logger), implementing a 7-state FSM validated with real flight data, and upgrading sensors (BMP280→BMP585, MPU6050→LSM6DS3). Includes detailed FSM state diagrams, threshold values validated with 1,873 data points, task architecture diagrams, and phase-by-phase implementation checklist. |

### `/extras/` - Extra Tools and Test Data

| File | Description |
|------|-------------|
| `/extras/FSM_tester/FSM_Tester.py` | **Python FSM simulator**. Loads CSV flight data, calculates vertical velocity via numerical differentiation, applies acceleration filtering, and runs a sequential state machine detecting: liftoff (totalAccel > 15 m/s²), burnout (az < -8 OR totalAccel < 2), apogee (|vz| < 1 AND az < -0.1), freefall (totalAccel < 11.5 AND vz < -5), and parachute deployment (altitude ≤ 100m AND descending). Generates matplotlib visualization of detected events. |
| `/extras/FSM_tester/explicacao.md` | **Detailed FSM documentation**. Explains the purpose, architecture, detection functions, safety features (NaN/Inf validation, state guards), tuning parameters, and translation guide from Python to ESP32 C++. |
| `/extras/FSM_tester/flight_inserter.py` | **Serial data sender utility**. Reads CSV files and transmits rows over serial to ESP32 for hardware-in-the-loop testing of the flight state machine. Supports configurable delay, baud rate, and port selection. |
| `/extras/FSM_tester/13_30_11-Dados.csv` | **Real flight telemetry dataset** (1,873 points). Contains columns: millis, lat, lon, sat, alt, data, hora, altp, p, ax, ay, az, gx, gy, gz, pqd. Used for FSM validation. |
| `/extras/FSM_tester/dados_filtrados.csv` | **Filtered flight data** (61 points). Decimated version of real flight data for faster testing. |
| `/extras/FSM_tester/dados_simulados.csv` | **Simulated flight data** (1,510 points). Idealized trajectory without noise for initial logic validation. |
| `/extras/FSM_tester/OpenCSV.ipynb` | **Jupyter notebook** for interactive CSV data analysis and visualization. |
</task_result>
```

---

## Assistant (Plan · mimo-v2-pro-free · 8.2s)

_Thinking:_

Now I have a good overview. The directory is actually `/test` not `/tests`. Let me read the key files in detail to compare the FSM implementations:

1. `/test/FSM/FSM.ino` - the "approved" functional version
2. `/extras/FSM_tester/FSM_Tester.py` - the Python FSM tester
3. `/extras/FSM_tester/explicacao.md` - FSM documentation
4. `/firmware/REFACTORING_PLAN.md` - the refactoring plan with FSM design
5. `/firmware/firmware.ino` and related files for the current firmware implementation


6. I'll start by examining the FSM implementations to understand their structure and state transitions.

Now let me read the key files to do a detailed comparison.

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/test/FSM/FSM.ino"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/test/FSM/FSM.ino</path>
<type>file</type>
<content>1: // ================================
2: // 🚀 FMS Completa - Com Reset Automático
3: // ================================
4: 
5: #include <math.h>
6: 
7: // Constantes de configuração
8: const float PARACHUTE_ALTITUDE = 100.0;
9: const float GRAVITY = 9.81;
10: 
11: 
12: 
13: // Estados da máquina
14: enum FlightState { 
15:   IDLE = 0, 
16:   ASCENT = 1, 
17:   DESCENT = 2, 
18:   LANDED = 3 
19: };
20: 
21: // Variáveis globais
22: FlightState current_state = IDLE;
23: float prev_altp = 0.0;
24: float prev_millis = 0.0;
25: float prev_filtered_ax = 0.0;
26: float prev_filtered_ay = 0.0;
27: float prev_filtered_az = 0.0;
28: 
29: bool first_reading = true;
30: float prev_vz = 0.0;
31: 
32: // Flags de eventos
33: bool liftoff_detected = false;
34: bool burnout_detected = false;
35: bool apogee_detected = false;
36: bool freefall_detected = false;
37: bool parachute_deployed = false;
38: 
39: // Controle de reset automático
40: unsigned long last_data_time = 0;
41: bool waiting_for_new_flight = false;
42: const unsigned long TIMEOUT_MS = 500;  // 5 segundos sem dados = reset
43: 
44: // Filtro
45: float prev_acc = 0.0;
46: const float ALPHA = 0.2;
47: 
48: // ================================
49: // Função de Reset do Sistema
50: // ================================
51: void reset_flight_state() {
52:   Serial.println("\n🔄 RESETANDO MÁQUINA DE ESTADOS PARA NOVO VOO\n");
53:   
54:   current_state = IDLE;
55:   prev_altp = 0.0;
56:   prev_millis = 0.0;
57:   prev_filtered_ax = 0.0;
58:   prev_filtered_ay = 0.0;
59:   prev_filtered_az = 0.0;
60:   first_reading = true;
61:   prev_vz = 0.0;
62:   liftoff_detected = false;
63:   burnout_detected = false;
64:   apogee_detected = false;
65:   freefall_detected = false;
66:   parachute_deployed = false;
67:   prev_acc = 0.0;
68:   waiting_for_new_flight = false;
69:   
70: }
71: 
72: // ================================
73: // Funções de detecção (mesmas do código anterior)
74: // ================================
75: 
76: float compute_total_acceleration(float ax, float ay, float az) {
77:   return sqrt(ax*ax + ay*ay + az*az);
78: }
79: 
80: float smooth(float value, float prev_value, float alpha) {
81:   if (prev_value == 0.0 && !first_reading) return value;
82:   return alpha * value + (1.0 - alpha) * prev_value;
83: }
84: 
85: bool detect_liftoff(float ax, float ay, float az, float height) {
86:   float total_acc = compute_total_acceleration(ax, ay, az);
87:   return (total_acc > 15.0);
88: }
89: 
90: bool detect_burnout(float ax, float ay, float az, float height, float vz) {
91:   float total_acc = compute_total_acceleration(ax, ay, az);
92:   
93:   if (height < 5.0 || vz <= 0.5) {
94:     return false;
95:   }
96:   
97:   return (az < -8.0 || total_acc < 2.0);
98: }
99: 
100: bool detect_apogee(float vz, float az) {
101:   return (abs(vz) < 1.0 && az < -0.1);
102: }
103: 
104: bool detect_freefall(float vz, float az, float height, float total_acc) {
105:   if (height < 5.0 || vz >= -5.0) {
106:     return false;
107:   }
108:   return (total_acc < 11.5);
109: }
110: 
111: bool detect_parachute(float height, float vz) {
112:   return (height <= PARACHUTE_ALTITUDE && vz < 0);
113: }
114: 
115: bool detect_landed(float vz, float height) {
116:   return (abs(vz) < 0.5 && height < 2.0);
117: }
118: 
119: // ================================
120: // Funções de impressão
121: // ================================
122: 
123: const char* state_name(FlightState state) {
124:   switch(state) {
125:     case IDLE: return "IDLE";
126:     case ASCENT: return "ASCENT";
127:     case DESCENT: return "DESCENT";
128:     case LANDED: return "LANDED";
129:     default: return "UNKNOWN";
130:   }
131: }
132: 
133: void print_event(const char* event, float t, float h, float vz, float ax, float ay, float az) {
134:   float total_acc = compute_total_acceleration(ax, ay, az);
135:   
136:   Serial.print(event);
137:   Serial.print(" @ ");
138:   Serial.print(t, 2);
139:   Serial.print("s | h=");
140:   Serial.print(h, 1);
141:   Serial.print(" | vz=");
142:   Serial.print(vz, 2);
143:   Serial.print(" | ax=");
144:   Serial.print(ax, 2);
145:   Serial.print(" | ay=");
146:   Serial.print(ay, 2);
147:   Serial.print(" | az=");
148:   Serial.print(az, 2);
149:   Serial.print(" | acc=");
150:   Serial.print(total_acc, 2);
151:   Serial.print(" | state=State.");
152:   Serial.println(state_name(current_state));
153: }
154: 
155: void print_status(float t, float h, float vz, float ax, float ay, float az) {
156:   float total_acc = compute_total_acceleration(ax, ay, az);
157:   
158:   Serial.print("📊 Status @ ");
159:   Serial.print(t, 2);
160:   Serial.print("s | h=");
161:   Serial.print(h, 1);
162:   Serial.print(" | vz=");
163:   Serial.print(vz, 2);
164:   Serial.print(" | acc=");
165:   Serial.print(total_acc, 2);
166:   Serial.print(" | state=State.");
167:   Serial.println(state_name(current_state));
168: }
169: 
170: // ================================
171: // Processamento principal
172: // ================================
173: 
174: void process_sensor_data(float millis, float altp, float ax, float ay, float az) {
175:   float vz = 0.0;
176:   float dt = 0.0;
177:   
178:   // Reset se houver gap de dados (nova execução)
179:   static float last_millis = -1;
180:   if (last_millis > 0 && millis < last_millis) {
181:     // Se o tempo reiniciou, é uma nova execução
182:     reset_flight_state();
183:   }
184:   last_millis = millis;
185:   
186:   // Calcular velocidade vertical
187:   if (!first_reading) {
188:     dt = millis - prev_millis;
189:     if (dt > 0) {
190:       vz = (altp - prev_altp) / dt;
191:       vz = constrain(vz, -200.0, 200.0);
192:     }
193:   } else {
194:     first_reading = false;
195:   }
196:   
197:   // Aplicar filtro
198:   float filtered_ax = smooth(ax, prev_filtered_ax, ALPHA);
199:   float filtered_ay = smooth(ay, prev_filtered_ay, ALPHA);
200:   float filtered_az = smooth(az, prev_filtered_az, ALPHA);
201:   prev_filtered_ax = filtered_ax;
202:   prev_filtered_ay = filtered_ay;
203:   prev_filtered_az = filtered_az;
204: 
205:   float total_acc = compute_total_acceleration(filtered_ax, filtered_ay, filtered_az);
206:   
207:   // ===== MÁQUINA DE ESTADOS =====
208:   
209:   switch(current_state) {
210:     case IDLE:
211:       if (!liftoff_detected && detect_liftoff(filtered_ax, filtered_ay, filtered_az, altp)) {
212:         current_state = ASCENT;
213:         liftoff_detected = true;
214:         print_event("🚀 LIFTOFF", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
215:       }
216:       break;
217:       
218:     case ASCENT:
219:       if (!burnout_detected && detect_burnout(filtered_ax, filtered_ay, filtered_az, altp, vz)) {
220:         burnout_detected = true;
221:         print_event("🔥 BURNOUT", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
222:       }
223:       
224:       if (!apogee_detected && detect_apogee(vz, filtered_az)) {
225:         current_state = DESCENT;
226:         apogee_detected = true;
227:         print_event("⛰️ APOGEE", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
228:       }
229:       break;
230:       
231:     case DESCENT:
232:       if (!freefall_detected && detect_freefall(vz, filtered_az, altp, total_acc)) {
233:         freefall_detected = true;
234:         print_event("🪂 FREEFALL", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
235:       }
236:       
237:       if (!parachute_deployed && detect_parachute(altp, vz)) {
238:         parachute_deployed = true;
239:         print_event("🪂 PARACHUTE DEPLOYED", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
240:       }
241:       
242:       if (detect_landed(vz, altp)) {
243:         current_state = LANDED;
244:         print_event("🏁 LANDED", millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
245:       }
246:       break;
247:       
248:     case LANDED:
249:       // Estado final, aguarda reset
250:       break;
251:   }
252:   
253:   // Status periódico
254:   static int counter = 0;
255:   counter++;
256:   if (counter >= 50 && current_state != LANDED) {
257:     counter = 0;
258:     print_status(millis, altp, vz, filtered_ax, filtered_ay, filtered_az);
259:   }
260:   
261:   // Atualizar variáveis
262:   prev_altp = altp;
263:   prev_millis = millis;
264:   prev_vz = vz;
265:   prev_acc = total_acc;
266: }
267: 
268: // ================================
269: // Setup e Loop
270: // ================================
271: 
272: void setup() {
273:   Serial.begin(115200);
274:   delay(2000);
275:   
276:   Serial.println("=========================================");
277:   Serial.println("🚀 FMS Completa - Ready to receive data");
278:   Serial.println("=========================================");
279:   Serial.println("Formato esperado: millis,altp,ax,ay,az");
280:   Serial.println("Sistema irá resetar automaticamente para cada novo voo");
281:   Serial.println("=========================================\n");
282: }
283: 
284: void loop() {
285:   if (Serial.available() > 0) {
286:     String data = Serial.readStringUntil('\n');
287:     data.trim();
288: 
289: 
290: 
291:     // Comando de reset
292:     if (data == "RESET") {
293:       reset_flight_state();
294:       Serial.println("✅ Sistema resetado para novo voo!");
295:       return;
296:     }
297: 
298:   
299:     
300:     // Parse CSV: millis,altp,ax,ay,az
301:     int i1 = data.indexOf(',');
302:     int i2 = data.indexOf(',', i1 + 1);
303:     int i3 = data.indexOf(',', i2 + 1);
304:     int i4 = data.indexOf(',', i3 + 1);
305:     
306:     if (i1 != -1 && i2 != -1 && i3 != -1 && i4 != -1) {
307:       float millis = data.substring(0, i1).toFloat();
308:       float altp   = data.substring(i1 + 1, i2).toFloat();
309:       float ax     = data.substring(i2 + 1, i3).toFloat();
310:       float ay     = data.substring(i3 + 1, i4).toFloat();
311:       float az     = data.substring(i4 + 1).toFloat();
312:       
313:       // Validar dados
314:       if (isnan(millis) || isnan(altp) || isnan(ax) || isnan(ay) || isnan(az)) {
315:         Serial.println("⚠️ Dados inválidos recebidos!");
316:         return;
317:       }
318:       
319:       process_sensor_data(millis, altp, ax, ay, az);
320:     }
321:   }
322: }

(End of file - total 322 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/extras/FSM_tester/FSM_Tester.py"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/extras/FSM_tester/FSM_Tester.py</path>
<type>file</type>
<content>1: import numpy as np
2: import matplotlib.pyplot as plt
3: import pandas as pd
4: 
5: # file = "Open csv file/dados_simulados.csv"
6: file = "extras/FSM_tester/dados_filtrados.csv"
7: # file = "Open csv file/dados_filtrados.csv"
8: 
9: df = pd.read_csv(file)
10: # from "# Time (s), Z (m), Ax (m/s²), Ay (m/s²), Az (m/s²)" to "millis,altp,ax,ay,az"
11: 
12: if file == "extras/FSM_tester/dados_simulados.csv": 
13:     df["altp"] = df["altp"] - df["altp"].iloc[0]   # Convert to altitude above ground level
14: 
15: if file == "extras/FSM_tester/13_30_11-Dados.csv":
16:     df["millis"] = df["millis"] / 1000.0  # Convert milliseconds to seconds
17: 
18: def detect_motor_burnout(_pressure, height, state_vector, u_dot):
19:     """Detect motor burnout by sudden drop in acceleration.
20: 
21:     Returns True when vertical acceleration becomes significantly negative
22:     (indicating end of propulsion phase) OR when total acceleration drops below
23:     a threshold indicating coasting/free-fall has begun.
24:     """
25:     try:
26:         if u_dot is None or len(u_dot) < 6:
27:             return False
28: 
29:         ax = float(u_dot[3])
30:         ay = float(u_dot[4])
31:         az = float(u_dot[5])
32: 
33:         # Defensive checks for NaN/Inf
34:         if not all(np.isfinite([ax, ay, az])):
35:             return False
36: 
37:         total_acc = np.sqrt(ax * ax + ay * ay + az * az)
38:         if not np.isfinite(total_acc):
39:             return False
40: 
41:         # Additional safety: ignore spurious low-accel readings at t~0 by
42:         # requiring the rocket to be above a small altitude and still ascending
43:         vz = float(state_vector[5]) if len(state_vector) > 5 else 0
44:         if not np.isfinite(vz):
45:             return False
46: 
47:         if height < 5.0 or vz <= 0.5:
48:             return False
49: 
50:         # Burnout detected when:
51:         # 1. Vertical acceleration becomes very negative (end of thrust phase)
52:         # 2. OR total acceleration drops below 2.0 m/s² (coasting detected)
53:         return az < -8.0 or total_acc < 2.0
54:     except (ValueError, TypeError, IndexError):
55:         return False
56: 
57: 
58: def detect_apogee_acceleration(_pressure, _height, state_vector, u_dot):
59:     """Detect apogee using near-zero vertical velocity and negative vertical accel.
60: 
61:     Apogee occurs when the rocket reaches its highest point, characterized by
62:     vertical velocity approaching zero and negative (downward) acceleration.
63:     """
64:     try:
65:         if state_vector is None or u_dot is None:
66:             return False
67:         if len(state_vector) < 6 or len(u_dot) < 6:
68:             return False
69: 
70:         vz = float(state_vector[5])
71:         az = float(u_dot[5])
72:         if not all(np.isfinite([vz, az])):
73:             return False
74: 
75:         # Slightly more permissive thresholds to avoid spurious misses
76:         return abs(vz) < 1.0 and az < -0.1
77:     except (ValueError, TypeError, IndexError):
78:         return False
79: 
80: 
81: def detect_freefall(_pressure, height, state_vector, u_dot):
82:     """Detect free-fall when total acceleration magnitude is low.
83: 
84:     Free-fall is characterized by acceleration magnitude close to gravitational
85:     acceleration (approximately -g in the vertical direction), or when the rocket
86:     is in a ballistic coasting phase with minimal thrust or drag effects.
87:     """
88:     try:
89:         if u_dot is None or len(u_dot) < 6:
90:             return False
91: 
92:         ax = float(u_dot[3])
93:         ay = float(u_dot[4])
94:         az = float(u_dot[5])
95:         if not all(np.isfinite([ax, ay, az])):
96:             return False
97: 
98:         total_acc = np.sqrt(ax * ax + ay * ay + az * az)
99:         if not np.isfinite(total_acc):
100:             return False
101: 
102:         # Require the rocket to be descending and above a small altitude to
103:         # avoid false positives before launch.
104:         vz = float(state_vector[5]) if len(state_vector) > 5 else 0
105:         if not np.isfinite(vz):
106:             return False
107: 
108:         if height < 5.0 or vz >= -0.2:
109:             return False
110: 
111:         # More sensitive threshold: detect free-fall at lower acceleration
112:         return total_acc < 11.5 and vz < -5
113:     except (ValueError, TypeError, IndexError):
114:         return False
115: 
116: 
117: def detect_liftoff(_pressure, _height, _state_vector, u_dot):
118:     """Detect liftoff by high total acceleration.
119: 
120:     Liftoff is characterized by a sudden increase in acceleration as the motor
121:     ignites and begins producing thrust.
122:     """
123:     try:
124:         if u_dot is None or len(u_dot) < 6:
125:             return False
126: 
127:         ax = float(u_dot[3])
128:         ay = float(u_dot[4])
129:         az = float(u_dot[5])
130:         if not all(np.isfinite([ax, ay, az])):
131:             return False
132: 
133:         total_acc = np.sqrt(ax * ax + ay * ay + az * az)
134:         if not np.isfinite(total_acc):
135:             return False
136: 
137:         return total_acc > 15.0
138:     except (ValueError, TypeError, IndexError):
139:         return False
140: 
141: 
142: def altitude_trigger_factory(target_altitude, require_descent=True):
143:     """Return a trigger that deploys when altitude <= target_altitude.
144: 
145:     If require_descent is True, also require vertical velocity negative
146:     (descending) to avoid firing during ascent.
147:     """
148: 
149:     def trigger(_pressure, height, state_vector, _u_dot=None):
150:         vz = float(state_vector[5])
151:         if require_descent:
152:             return (height <= target_altitude) and (vz < 0)
153:         return height <= target_altitude
154: 
155:     return trigger
156: 
157: 
158: PARACHUTE_ALTITUDE = 100
159: parachute_trigger = altitude_trigger_factory(PARACHUTE_ALTITUDE)
160: 
161: # Initialize variables
162: liftoff = False
163: burnout = False
164: apogee = False
165: freefall = False
166: parachute = False
167: 
168: df["vz"] = 0.0
169: df["event"] = ""
170: 
171: def smooth(value, prev_value, alpha):
172:     if prev_value is None:
173:         return value
174:     return alpha * value + (1 - alpha) * prev_value
175: 
176: # Process events
177: for i, row in df.iterrows():
178:     height = float(row["altp"])
179:     millis = float(row["millis"])
180:     height = smooth(height, df.loc[i - 1, "altp"] if i > 0 else None, alpha=0.2)
181: 
182:     ax = float(row["ax"])
183:     ay = float(row["ay"])
184:     az = float(row["az"])
185:     ax = smooth(ax, df.loc[i - 1, "ax"] if i > 0 else None, alpha=0.2)
186:     ay = smooth(ay, df.loc[i - 1, "ay"] if i > 0 else None, alpha=0.2)
187:     az = smooth(az, df.loc[i - 1, "az"] if i > 0 else None, alpha=0.2)
188: 
189:     # Estimate vertical velocity
190:     if i == 0:
191:         vz = 0.0
192:     else:
193:         prev_alt = float(df.iloc[i - 1]["altp"])
194:         prev_time = float(df.iloc[i - 1]["millis"])
195:         dt = (millis - prev_time)
196: 
197:         if dt <= 0:
198:             vz = 0.0
199:         else:
200:             vz = (height - prev_alt) / dt
201:             vz = np.clip(vz, -200, 200)
202: 
203:     df.loc[i, "vz"] = vz
204:     state_vector = [0, 0, height, 0, 0, vz]
205:     u_dot = [0, 0, 0, ax, ay, az]
206: 
207:     if not liftoff and detect_liftoff(None, height, state_vector, u_dot):
208:         df.loc[i, "event"] = "liftoff"
209:         print(
210:                 f"LIFTOFF detectado em {millis:.2f} s | altura {height:.2f} | vz {vz:.2f} | ax {ax:.2f} | ay {ay:.2f} | az {az:.2f} | acc {np.sqrt(ax*ax + ay*ay + az*az):.2f}"
211:             )
212:         liftoff = True
213:     elif not burnout and detect_motor_burnout(None, height, state_vector, u_dot) and liftoff:
214:         df.loc[i, "event"] = "burnout"
215:         print(
216:                 f"BURNOUT detectado em {millis:.2f} s | altura {height:.2f} | vz {vz:.2f} | ax {ax:.2f} | ay {ay:.2f} | az {az:.2f} | acc {np.sqrt(ax*ax + ay*ay + az*az):.2f}"
217:             )
218:         burnout = True
219:     elif not apogee and detect_apogee_acceleration(None, height, state_vector, u_dot) and burnout:
220:         df.loc[i, "event"] = "apogee"
221:         print(
222:                 f"APOGEE detectado em {millis:.2f} s | altura {height:.2f} | vz {vz:.2f} | ax {ax:.2f} | ay {ay:.2f} | az {az:.2f} | acc {np.sqrt(ax*ax + ay*ay + az*az):.2f}"
223:             )
224:         apogee = True
225:     elif not freefall and detect_freefall(None, height, state_vector, u_dot) and apogee:
226:         df.loc[i, "event"] = "freefall"
227:         print(
228:                 f"FREEFALL detectado em {millis:.2f} s | altura {height:.2f} | vz {vz:.2f} | ax {ax:.2f} | ay {ay:.2f} | az {az:.2f} | acc {np.sqrt(ax*ax + ay*ay + az*az):.2f}"
229:             )
230:         freefall = True
231:     # elif not parachute and parachute_trigger(None, height, state_vector) and freefall:
232:     #     df.loc[i, "event"] = "parachute"
233:     #     print(
234:     #             f"PARACHUTE trigger em {millis:.2f} s | altura {height:.2f} | vz {vz:.2f} | ax {ax:.2f} | ay {ay:.2f} | az {az:.2f} | acc {np.sqrt(ax*ax + ay*ay + az*az):.2f}"
235:     #         )
236:     #     parachute = True
237: 
238: 

(End of file - total 238 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/extras/FSM_tester/explicacao.md"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/extras/FSM_tester/explicacao.md</path>
<type>file</type>
<content>1: # Explicação: Simulador de Máquina de Estados Finitos (FSM) para Foguete
2: 
3: ## Visão Geral
4: 
5: Este código Python (`FSM_Tester.py`) simula uma **Máquina de Estados Finitos (FSM - Finite State Machine)** que será implementada em um **ESP32** para controle de voo de um foguete. O objetivo é testar a lógica de detecção de eventos de voo usando dados simulados ou reais antes de implementar o código no microcontrolador embarcado.
6: 
7: ## Propósito do Código
8: 
9: O simulador serve para:
10: 
11: 1. **Validar algoritmos de detecção** de fases de voo antes da implementação em hardware
12: 2. **Ajustar parâmetros** (thresholds) de detecção com segurança
13: 3. **Visualizar eventos** detectados em dados de voo reais ou simulados
14: 4. **Garantir segurança** ao testar lógica crítica sem riscos ao hardware
15: 
16: ## Arquitetura da FSM
17: 
18: A máquina de estados implementa **transições sequenciais** entre as fases de voo de um foguete:
19: 
20: ```
21: IDLE (Repouso)
22:     ↓ [Aceleração > 15 m/s²]
23: LIFTOFF (Decolagem)
24:     ↓ [Aceleração < 2 m/s² OU az < -8 m/s²]
25: BURNOUT (Fim do Motor)
26:     ↓ [Velocidade vertical ≈ 0 E aceleração negativa]
27: APOGEE (Apogeu)
28:     ↓ [Aceleração total < 11.5 m/s² E descendo]
29: FREEFALL (Queda Livre)
30:     ↓ [Altitude ≤ 100m E descendo]
31: PARACHUTE (Paraquedas Aberto)
32:     ↓
33: LANDED (Pouso)
34: ```
35: 
36: ## Componentes Principais
37: 
38: ### 1. Funções de Detecção de Estados
39: 
40: #### `detect_liftoff()` - Detecção de Decolagem
41: **Linha:** 117-139
42: 
43: **Critério:** Aceleração total > 15.0 m/s²
44: 
45: **Funcionamento:**
46: - Calcula aceleração total: `√(ax² + ay² + az²)`
47: - Ignição do motor causa pico súbito de aceleração
48: - Validações: verifica NaN/Inf para evitar falsos positivos
49: 
50: **Threshold ESP32:** `> 15.0 m/s²`
51: 
52: ---
53: 
54: #### `detect_motor_burnout()` - Detecção de Fim do Motor
55: **Linha:** 18-55
56: 
57: **Critérios:**
58: 1. Aceleração vertical < -8.0 m/s² (gravidade domina)
59: 2. OU aceleração total < 2.0 m/s² (coasting)
60: 
61: **Funcionamento:**
62: - Detecta quando o motor para de gerar empuxo
63: - Requer altitude > 5m e velocidade vertical > 0.5 m/s (evita falsos positivos no solo)
64: - Fim da fase propulsada → início do voo balístico
65: 
66: **Thresholds ESP32:**
67: - `az < -8.0 m/s²` OU `total_acc < 2.0 m/s²`
68: - Altitude mínima: `5.0 m`
69: - Velocidade vertical mínima: `0.5 m/s`
70: 
71: ---
72: 
73: #### `detect_apogee_acceleration()` - Detecção de Apogeu
74: **Linha:** 58-78
75: 
76: **Critérios:**
77: 1. Velocidade vertical absoluta < 1.0 m/s (quase zero)
78: 2. E aceleração vertical < -0.1 m/s² (descendo)
79: 
80: **Funcionamento:**
81: - Ponto mais alto da trajetória
82: - Velocidade vertical inverte de positiva para negativa
83: - Momento crítico para acionamento de sistema de recuperação
84: 
85: **Thresholds ESP32:**
86: - `|vz| < 1.0 m/s`
87: - `az < -0.1 m/s²`
88: 
89: ---
90: 
91: #### `detect_freefall()` - Detecção de Queda Livre
92: **Linha:** 81-114
93: 
94: **Critérios:**
95: 1. Aceleração total < 11.5 m/s² (próxima à gravidade)
96: 2. E velocidade vertical < -5 m/s (descendo rapidamente)
97: 3. E altura > 5m
98: 
99: **Funcionamento:**
100: - Detecta fase balística após apogeu
101: - Aceleração dominada pela gravidade (~9.8 m/s²)
102: - Estado anterior ao acionamento do paraquedas
103: 
104: **Thresholds ESP32:**
105: - `total_acc < 11.5 m/s²`
106: - `vz < -5 m/s`
107: - Altitude mínima: `5.0 m`
108: 
109: ---
110: 
111: #### `altitude_trigger_factory()` - Gatilho de Altitude
112: **Linha:** 142-155
113: 
114: **Critério:** Altitude ≤ altitude_alvo (padrão: 100m)
115: 
116: **Funcionamento:**
117: - Factory function que cria gatilhos configuráveis por altitude
118: - Opção `require_descent=True` garante que só aciona durante descida
119: - Usado para abrir paraquedas em altitude segura
120: 
121: **Configuração ESP32:**
122: ```python
123: PARACHUTE_ALTITUDE = 100  # metros
124: parachute_trigger = altitude_trigger_factory(PARACHUTE_ALTITUDE)
125: ```
126: 
127: ---
128: 
129: ### 2. Processamento de Dados
130: 
131: #### Entrada de Dados (CSV)
132: **Linhas:** 5-16
133: 
134: **Formatos suportados:**
135: ```csv
136: # dados_simulados.csv
137: millis, altp, ax, ay, az
138: 
139: # 13_30_11-Dados.csv e dados_filtrados.csv
140: millis, lat, lon, sat, alt, data, hora, altp, p, ax, ay, az, gx, gy, gz, pqd
141: ```
142: 
143: **Campos utilizados:**
144: - `millis`: Timestamp em milissegundos (convertido para segundos)
145: - `altp`: Altitude barométrica em metros
146: - `ax, ay, az`: Acelerações nos eixos X, Y, Z em m/s²
147: 
148: ---
149: 
150: #### Cálculo de Velocidade Vertical
151: **Linhas:** 189-202
152: 
153: **Método:** Diferenciação numérica
154: ```python
155: vz = (altura_atual - altura_anterior) / (tempo_atual - tempo_anterior)
156: ```
157: 
158: **Limites:**
159: - Clipping: `-200 m/s` a `+200 m/s`
160: - Evita valores espúrios em dados ruidosos
161: 
162: ---
163: 
164: #### Vetor de Estado
165: **Linha:** 204
166: ```python
167: state_vector = [x, y, height, vx, vy, vz]
168: ```
169: 
170: **Componentes:**
171: - Posição: `[x, y, height]` - coordenadas espaciais (x e y não utilizados nesta versão)
172: - Velocidade: `[vx, vy, vz]` - componentes da velocidade (apenas vz utilizado)
173: 
174: ---
175: 
176: #### Vetor de Aceleração
177: **Linha:** 205
178: ```python
179: u_dot = [ax_dot, ay_dot, az_dot, ax, ay, az]
180: ```
181: 
182: **Componentes:**
183: - Derivadas: `[ax_dot, ay_dot, az_dot]` - não utilizadas nesta versão
184: - Acelerações: `[ax, ay, az]` - leituras diretas dos acelerômetros
185: 
186: ---
187: 
188: ### 3. Loop Principal de Processamento
189: **Linhas:** 177-236
190: 
191: **Algoritmo:**
192: ```
193: Para cada linha do CSV:
194:     1. Ler altitude e acelerações
195:     2. Calcular velocidade vertical (vz)
196:     3. Construir vetores de estado
197:     4. Verificar transições sequenciais:
198:        - SE não decolou E detect_liftoff() → LIFTOFF
199:        - SE decolou E não burnout E detect_motor_burnout() → BURNOUT
200:        - SE burnout E não apogeu E detect_apogee() → APOGEE
201:        - SE apogeu E não freefall E detect_freefall() → FREEFALL
202:        - SE freefall E não paraquedas E parachute_trigger() → PARACHUTE
203:     5. Registrar eventos detectados
204: ```
205: 
206: **Características:**
207: - **Transições unidirecionais:** Estados só avançam, nunca retrocedem
208: - **Dependências em cadeia:** Cada estado requer que o anterior tenha ocorrido
209: - **Logging detalhado:** Imprime tempo, altitude, velocidades e acelerações de cada evento
210: 
211: ---
212: 
213: ### 4. Visualização
214: **Linhas:** 238-253
215: 
216: **Gráfico gerado:**
217: - **Eixo X:** Tempo em segundos
218: - **Eixo Y:** Altitude barométrica em metros
219: - **Marcadores:** Linhas verticais tracejadas em cada evento detectado
220: - **Labels:** Nome do evento (liftoff, burnout, apogee, freefall)
221: 
222: ---
223: 
224: ## Dados Simulados Utilizados
225: 
226: O código pode processar três tipos de arquivos CSV:
227: 
228: ### 1. `dados_simulados.csv`
229: - Dados de simulação computacional
230: - 1.510 pontos de dados
231: - Trajetória idealizada sem ruído
232: - Melhor para validação inicial de lógica
233: 
234: ### 2. `dados_filtrados.csv`
235: - Dados reais de voo filtrados
236: - 61 pontos (dados decimados)
237: - Inclui GPS, pressão barométrica, IMU completa
238: - Campo `pqd` indica acionamento de paraquedas
239: 
240: ### 3. `13_30_11-Dados.csv`
241: - Dados de voo real completos
242: - 1.873 pontos de telemetria
243: - Sensor: IMU + GPS + barômetro
244: - Usado no código atual (linha 6)
245: 
246: ---
247: 
248: ## Tradução para ESP32
249: 
250: O código Python foi projetado para **tradução direta** para C++ no ESP32:
251: 
252: ### Equivalências de Implementação
253: 
254: | Python | ESP32 (C++/Arduino) |
255: |--------|---------------------|
256: | `detect_liftoff()` | `bool detectLiftoff(float ax, float ay, float az)` |
257: | `detect_motor_burnout()` | `bool detectMotorBurnout(float height, float vz, float* accel)` |
258: | `detect_apogee()` | `bool detectApogee(float vz, float az)` |
259: | `detect_freefall()` | `bool detectFreefall(float height, float vz, float totalAccel)` |
260: | `numpy.sqrt()` | `sqrtf()` da biblioteca `<math.h>` |
261: | `pandas DataFrame` | Arrays circulares ou buffers de dados |
262: | Loop `for i, row in df.iterrows()` | `loop()` principal do Arduino |
263: 
264: ### Exemplo de Implementação ESP32
265: 
266: ```cpp
267: // Definição de estados
268: enum FlightState {
269:     IDLE,
270:     LIFTOFF,
271:     BURNOUT,
272:     APOGEE,
273:     FREEFALL,
274:     PARACHUTE_DEPLOYED,
275:     LANDED
276: };
277: 
278: FlightState currentState = IDLE;
279: 
280: // Variáveis de sensor
281: float ax, ay, az;        // Acelerômetro
282: float altitude;          // Barômetro
283: float vz = 0.0;         // Velocidade vertical calculada
284: float prevAltitude = 0.0;
285: unsigned long prevTime = 0;
286: 
287: void loop() {
288:     // 1. Ler sensores
289:     readIMU(&ax, &ay, &az);
290:     altitude = readBarometer();
291:     
292:     // 2. Calcular velocidade vertical
293:     unsigned long currentTime = millis();
294:     float dt = (currentTime - prevTime) / 1000.0;  // segundos
295:     if (dt > 0) {
296:         vz = (altitude - prevAltitude) / dt;
297:     }
298:     
299:     // 3. Máquina de estados
300:     switch(currentState) {
301:         case IDLE:
302:             if (detectLiftoff(ax, ay, az)) {
303:                 currentState = LIFTOFF;
304:                 Serial.println("LIFTOFF");
305:             }
306:             break;
307:             
308:         case LIFTOFF:
309:             if (detectMotorBurnout(altitude, vz, ax, ay, az)) {
310:                 currentState = BURNOUT;
311:                 Serial.println("BURNOUT");
312:             }
313:             break;
314:             
315:         case BURNOUT:
316:             if (detectApogee(vz, az)) {
317:                 currentState = APOGEE;
318:                 Serial.println("APOGEE");
319:             }
320:             break;
321:             
322:         case APOGEE:
323:             if (detectFreefall(altitude, vz, ax, ay, az)) {
324:                 currentState = FREEFALL;
325:                 Serial.println("FREEFALL");
326:             }
327:             break;
328:             
329:         case FREEFALL:
330:             if (altitude <= 100 && vz < 0) {
331:                 currentState = PARACHUTE_DEPLOYED;
332:                 deployParachute();  // Ativar servo/pirotécnico
333:                 Serial.println("PARACHUTE");
334:             }
335:             break;
336:     }
337:     
338:     // 4. Atualizar histórico
339:     prevAltitude = altitude;
340:     prevTime = currentTime;
341:     
342:     delay(10);  // 100 Hz de atualização
343: }
344: 
345: bool detectLiftoff(float ax, float ay, float az) {
346:     float totalAccel = sqrtf(ax*ax + ay*ay + az*az);
347:     return totalAccel > 15.0;
348: }
349: 
350: bool detectMotorBurnout(float height, float vz, float ax, float ay, float az) {
351:     if (height < 5.0 || vz <= 0.5) return false;
352:     
353:     float totalAccel = sqrtf(ax*ax + ay*ay + az*az);
354:     return (az < -8.0) || (totalAccel < 2.0);
355: }
356: 
357: bool detectApogee(float vz, float az) {
358:     return (fabs(vz) < 1.0) && (az < -0.1);
359: }
360: 
361: bool detectFreefall(float height, float vz, float ax, float ay, float az) {
362:     if (height < 5.0 || vz >= -0.2) return false;
363:     
364:     float totalAccel = sqrtf(ax*ax + ay*ay + az*az);
365:     return (totalAccel < 11.5) && (vz < -5);
366: }
367: ```
368: 
369: ---
370: 
371: ## Funcionalidades de Segurança
372: 
373: ### 1. Validação de Dados
374: **Implementado em todas as funções de detecção:**
375: ```python
376: if not all(np.isfinite([ax, ay, az])):
377:     return False
378: ```
379: 
380: **Previne:**
381: - Leituras NaN (Not a Number)
382: - Valores infinitos
383: - Overflow/underflow numérico
384: 
385: ---
386: 
387: ### 2. Guardas de Estado
388: **Exemplo em `detect_motor_burnout()` (linhas 47-48):**
389: ```python
390: if height < 5.0 or vz <= 0.5:
391:     return False
392: ```
393: 
394: **Propósito:**
395: - Evita falsos positivos no solo (altura < 5m)
396: - Garante que o foguete está em movimento ascendente
397: - Previne acionamentos prematuros
398: 
399: ---
400: 
401: ### 3. Transições Condicionais
402: **Linha 213:**
403: ```python
404: elif not burnout and detect_motor_burnout(...) and liftoff:
405: ```
406: 
407: **Lógica:**
408: - `not burnout`: Evento ainda não ocorreu
409: - `detect_motor_burnout()`: Condição física atendida
410: - `and liftoff`: Estado anterior confirmado
411: 
412: **Resultado:** Transições só ocorrem na sequência correta
413: 
414: ---
415: 
416: ### 4. Suavização Opcional
417: **Linhas 180-187 (comentadas):**
418: ```python
419: # height = smooth(height, prev_height, alpha=0.2)
420: ```
421: 
422: **Função `smooth()` (linhas 171-174):**
423: ```python
424: def smooth(value, prev_value, alpha):
425:     return alpha * value + (1 - alpha) * prev_value
426: ```
427: 
428: **Aplicação:**
429: - Filtro passa-baixa (exponential moving average)
430: - Alpha = 0.2: 20% valor novo, 80% histórico
431: - Reduz ruído dos sensores
432: - **Comentado** para preservar resposta rápida
433: 
434: ---
435: 
436: ## Ajustes de Parâmetros (Tuning)
437: 
438: ### Thresholds Críticos
439: 
440: | Parâmetro | Valor | Localização | Efeito ao Aumentar |
441: |-----------|-------|-------------|-------------------|
442: | Aceleração de Liftoff | 15.0 m/s² | Linha 137 | Detecção mais tardia, menos falsos positivos |
443: | Aceleração de Burnout | 2.0 m/s² | Linha 53 | Detecção mais tardia |
444: | Az negativo (Burnout) | -8.0 m/s² | Linha 53 | Detecção mais sensível a gravidade |
445: | Velocidade de Apogeu | 1.0 m/s | Linha 76 | Janela de detecção mais estreita |
446: | Aceleração de Freefall | 11.5 m/s² | Linha 112 | Detecção mais próxima de ~1g |
447: | Velocidade de Freefall | -5 m/s | Linha 112 | Requer descida mais rápida |
448: | Altitude de Paraquedas | 100 m | Linha 158 | Abertura mais alta/baixa |
449: 
450: ### Recomendações para Ajuste
451: 
452: 1. **Liftoff:** Aumentar threshold se houver vibrações pré-voo
453: 2. **Burnout:** Ajustar baseado no perfil do motor (thrust curve)
454: 3. **Apogeu:** Reduzir `|vz|` para foguetes com apogeu baixo
455: 4. **Freefall:** Ajustar para arrasto aerodinâmico do foguete
456: 5. **Paraquedas:** Considerar velocidade terminal e altura mínima segura
457: 
458: ---
459: 
460: ## Limitações e Melhorias Futuras
461: 
462: ### Limitações Atuais
463: 
464: 1. **Sem Filtro de Kalman:** Velocidade calculada por diferenciação simples (ruidosa)
465: 2. **Eixos X/Y ignorados:** Não detecta instabilidade lateral
466: 3. **Sem validação de GPS:** Altitude barométrica pode ter drift
467: 4. **Hardcoded thresholds:** Valores fixos não adaptam a diferentes foguetes
468: 5. **Sem detecção de pouso:** FSM para em freefall/paraquedas
469: 
470: ### Melhorias Sugeridas para ESP32
471: 
472: 1. **Filtro de Kalman:** Fusão de sensores (barômetro + acelerômetro + GPS)
473: 2. **Watchdog timer:** Reset automático se FSM travar
474: 3. **Log em SD Card:** Gravar telemetria completa para análise pós-voo
475: 4. **Redundância:** Múltiplos critérios para eventos críticos (ex: apogeu)
476: 5. **Calibração automática:** Ajuste de thresholds baseado em pré-voo
477: 6. **Detecção de anomalias:** Tumbling, perda de sinal, falha de sensor
478: 7. **Estado de pouso:** Detectar impacto e estabilização
479: 
480: ---
481: 
482: ## Fluxo de Desenvolvimento
483: 
484: ```
485: ┌──────────────────────────────────────────────────┐
486: │  1. SIMULAÇÃO (Python - FSM_Tester.py)           │
487: │     - Testar lógica com dados históricos         │
488: │     - Ajustar thresholds                          │
489: │     - Visualizar eventos                          │
490: └────────────────────┬─────────────────────────────┘
491:                      ↓
492: ┌──────────────────────────────────────────────────┐
493: │  2. TRADUÇÃO (C++/Arduino)                       │
494: │     - Implementar funções de detecção            │
495: │     - Adaptar para leitura de sensores real-time │
496: │     - Adicionar máquina de estados switch-case   │
497: └────────────────────┬─────────────────────────────┘
498:                      ↓
499: ┌──────────────────────────────────────────────────┐
500: │  3. TESTES EM BANCADA (ESP32 + Sensores)        │
501: │     - Simular acelerações manualmente            │
502: │     - Validar leituras de sensores               │
503: │     - Testar acionamento de servo/pirotécnico    │
504: └────────────────────┬─────────────────────────────┘
505:                      ↓
506: ┌──────────────────────────────────────────────────┐
507: │  4. VOO TESTE (Foguete real)                     │
508: │     - Coletar telemetria real                    │
509: │     - Analisar eventos detectados                │
510: │     - Refinar thresholds se necessário           │
511: └────────────────────┬─────────────────────────────┘
512:                      ↓
513: ┌──────────────────────────────────────────────────┐
514: │  5. ITERAÇÃO (Retornar ao passo 1)               │
515: │     - Analisar dados do voo teste                │
516: │     - Ajustar simulação com dados reais          │
517: │     - Melhorar algoritmos                         │
518: └──────────────────────────────────────────────────┘
519: ```
520: 
521: ---
522: 
523: ## Conclusão
524: 
525: Este simulador é uma ferramenta essencial para desenvolvimento seguro de software de voo. Ele permite:
526: 
527: ✅ **Validar** lógica de detecção sem risco ao hardware  
528: ✅ **Iterar rapidamente** em ajustes de parâmetros  
529: ✅ **Visualizar** comportamento da FSM em dados reais  
530: ✅ **Traduzir facilmente** para implementação em ESP32  
531: 
532: A arquitetura modular e os critérios defensivos garantem que o código seja **robusto**, **testável** e **pronto para produção** em sistemas críticos de segurança como computadores de voo de foguetes.
533: 
534: ---
535: 
536: ## Referências
537: 
538: - Código fonte: `FSM_Tester.py`
539: - Dados de voo: `13_30_11-Dados.csv`, `dados_simulados.csv`, `dados_filtrados.csv`
540: - Notebook de análise: `OpenCSV.ipynb`
541: - Documentação ESP32: [Espressif Official Docs](https://docs.espressif.com/)

(End of file - total 541 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/REFACTORING_PLAN.md"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/REFACTORING_PLAN.md</path>
<type>file</type>
<content>1: # 🚀 Plano de Refatoração: POO + FreeRTOS + FSM
2: 
3: **Projeto:** Flight Computer - Team #100  
4: **Hardware:** ESP32-S3-DevKitC-1-N8R8 (8MB Flash, 512KB RAM)  
5: **Data Início:** 2026-03-18  
6: **Status:** 📋 Planejamento Completo - Aguardando Implementação
7: 
8: ---
9: 
10: ## 📊 Resumo Executivo
11: 
12: ### Objetivos
13: 1. ✅ Refatorar código procedural para **POO seletivo** (sensores apenas)
14: 2. ✅ Implementar **FSM** para controle de estados de voo
15: 3. ✅ Usar **FreeRTOS** para separar lógica crítica (FSM) de I/O (telemetria)
16: 4. ✅ Substituir sensores: **BMP280→BMP585**, **MPU6050→LSM6DS3**, **GPS N6M→N8M**
17: 5. ✅ Preparar arquitetura para modificações futuras
18: 
19: ### Motivação
20: - Código monolítico (584 linhas) chegou ao **EOL**
21: - Troca de sensores planejada requer abstração
22: - FSM necessária para controle de voo robusto
23: - Separação safety-critical (paraquedas) de I/O (logging)
24: 
25: ---
26: 
27: ## 🎯 Decisões Arquiteturais
28: 
29: | Aspecto | Decisão | Justificativa |
30: |---------|---------|---------------|
31: | **Abordagem POO** | Seletivo | Apenas sensores que mudam viram classes |
32: | **FSM** | Enum/Switch | Simples, direto, baixo overhead |
33: | **FreeRTOS** | 3 Tasks | Separação FSM (50Hz) / Telemetry (5Hz) / Logger |
34: | **Migração** | Big Bang | Refatoração completa de uma vez |
35: | **Testes** | Manuais/Hardware | Sem infraestrutura de unit tests |
36: | **Queue Size** | 25 (sensores), 50 (logs) | Cobre pior caso com margem |
37: | **Watchdog** | 5s, apenas Task FSM | Reset automático se travamento crítico |
38: | **Logging** | Híbrido (Queue + #ifdef) | Debug verboso em dev, seletivo em produção |
39: 
40: ---
41: 
42: ## 🏗️ Arquitetura Final
43: 
44: ### Estrutura de Diretórios
45: 
46: ```
47: firmware/
48: │
49: ├── firmware.ino                    # Main - Setup + Tasks creation
50: │
51: ├── config.h                        # Global configs + FSM thresholds
52: │
53: ├── sensors/                        # 🆕 POO - Sensor abstractions
54: │   ├── ISensor.h                   # Abstract base interface
55: │   ├── BMP585Sensor.h/cpp          # Barometer (altitude, pressure, temp)
56: │   ├── LSM6DS3Sensor.h/cpp         # IMU (accelerometer + gyroscope)
57: │   └── GPSModule.h/cpp             # GPS positioning and time
58: │
59: ├── flight/                         # 🆕 FSM + FreeRTOS Tasks
60: │   ├── SensorData.h                # Shared structs (SensorData, LogMessage)
61: │   ├── FlightStateMachine.h/cpp    # FSM - 7 states + transitions (VALIDADO)
62: │   ├── FlightControlTask.h/cpp     # Task 1 - FSM + Safety (50Hz, Core 1)
63: │   ├── TelemetryTask.h/cpp         # Task 2 - Logging + TX (5Hz, Core 0)
64: │   └── LoggerTask.h/cpp            # Task 3 - Debug logger (low priority)
65: │
66: └── modules/                        # Procedural modules (unchanged)
67:     ├── buzzer_module.h             # Audio feedback
68:     ├── filesystem_module.h         # LittleFS operations
69:     ├── lora_module.h               # LoRa transmission
70:     ├── parachute_module.h          # Servo + deployment logic
71:     └── server_module.h             # WiFi AP + Web interface
72: ```
73: 
74: ### Diagrama de Tasks FreeRTOS
75: 
76: ```
77: ┌────────────────────────────────────────────────────────────┐
78: │               ESP32-S3 Dual Core Architecture              │
79: ├────────────────────────────────────────────────────────────┤
80: │                                                            │
81: │  CORE 1 (pinned)             CORE 0 (shared)              │
82: │  ┌──────────────────────┐    ┌──────────────────────┐     │
83: │  │ Task 1: FlightCtrl   │    │ Task 2: Telemetry    │     │
84: │  │ Priority: 20 (HIGH)  │    │ Priority: 5 (LOW)    │     │
85: │  │ Freq: 50Hz (20ms)    │    │ Freq: 5Hz (200ms)    │     │
86: │  ├──────────────────────┤    ├──────────────────────┤     │
87: │  │ • BMP585->update()   │    │ • GPS->update()      │     │
88: │  │ • LSM6DS3->update()  │    │ • Receive Queue      │     │
89: │  │ • FSM->update()      │    │ • Serial.println()   │     │
90: │  │ • Parachute deploy   │    │ • sendLoRa()         │     │
91: │  │ • Send to Queue      │    │ • appendFile()       │     │
92: │  │ • Watchdog reset     │    │ • buzzSignal()       │     │
93: │  └──────────────────────┘    └──────────────────────┘     │
94: │           │                            │                   │
95: │           └──── sensorDataQueue ───────┘                   │
96: │                (25 slots, ~1.25KB)                         │
97: │                                                            │
98: │  ┌────────────────────────────────────────────────────┐   │
99: │  │ Task 3: Logger (CORE 0, Priority: 1, IDLE+)       │   │
100: │  │ • Receive from logQueue                           │   │
101: │  │ • Serial.printf() thread-safe                     │   │
102: │  │ • Append to /logs/flight.log (if DEBUG_MODE)      │   │
103: │  └────────────────────────────────────────────────────┘   │
104: │                      │                                     │
105: │                 logQueue (50 slots, ~6KB)                  │
106: │                      △                                     │
107: │          ┌───────────┴────────────┐                        │
108: │     Task 1 logs            Task 2 logs                     │
109: │                                                            │
110: └────────────────────────────────────────────────────────────┘
111: ```
112: 
113: ### FSM - Estados de Voo (⚠️ VALIDADO COM DADOS REAIS)
114: 
115: **CRÍTICO:** Esta FSM foi validada com 1,873 pontos de telemetria real. **NÃO ALTERE thresholds** sem consultar `extras/FSM_tester/FSM_Tester.py`.
116: 
117: ```
118: ┌─────────┐
119: │  IDLE   │  Pré-lançamento, aguardando no solo
120: └────┬────┘
121:      │ Trigger: totalAccel > 15.0 m/s²
122:      ▼
123: ┌──────────┐
124: │ LIFTOFF  │  Subida motorizada, alta aceleração
125: └────┬─────┘
126:      │ Trigger: (az < -8.0 m/s²) OR (totalAccel < 2.0 m/s²)
127:      │          AND (height > 5m) AND (vz > 0.5 m/s)
128:      ▼
129: ┌──────────┐
130: │ BURNOUT  │  Costa balística (sem motor)
131: └────┬─────┘
132:      │ Trigger: |vz| < 1.0 m/s AND az < -0.1 m/s²
133:      ▼
134: ┌──────────┐
135: │  APOGEE  │  Altitude máxima atingida
136: └────┬─────┘  🪂 DEPLOY PARACHUTE! (estado crítico)
137:      │ Trigger: totalAccel < 11.5 m/s² AND vz < -5 m/s
138:      │          AND height > 5m
139:      ▼
140: ┌───────────┐
141: │ FREEFALL  │  Queda rápida pós-apogeu
142: └────┬──────┘
143:      │ Trigger: altitude ≤ 100m AND vz < 0
144:      ▼
145: ┌────────────┐
146: │ PARACHUTE  │  Descida controlada com paraquedas
147: └────┬───────┘
148:      │ Trigger: [Implementação futura]
149:      ▼
150: ┌──────────┐
151: │  LANDED  │  Pouso detectado, fim do voo
152: └──────────┘
153: ```
154: 
155: **Thresholds Validados (config.h):**
156: ```cpp
157: // ⚠️ ATENÇÃO: Valores validados com dados reais de voo
158: // Referência: extras/FSM_tester/FSM_Tester.py
159: 
160: // Estado LIFTOFF (linha 137 do Python)
161: #define LIFTOFF_TOTAL_ACCEL_THRESHOLD  15.0   // m/s²
162: 
163: // Estado BURNOUT (linhas 47-59 do Python)
164: #define BURNOUT_AZ_THRESHOLD          -8.0    // m/s²
165: #define BURNOUT_TOTAL_ACCEL_MIN        2.0    // m/s²
166: #define BURNOUT_MIN_HEIGHT             5.0    // m
167: #define BURNOUT_MIN_VZ                 0.5    // m/s
168: 
169: // Estado APOGEE (linhas 72-79 do Python)
170: #define APOGEE_VZ_THRESHOLD            1.0    // m/s (absoluto)
171: #define APOGEE_AZ_THRESHOLD           -0.1    // m/s²
172: 
173: // Estado FREEFALL (linhas 95-113 do Python)
174: #define FREEFALL_TOTAL_ACCEL_MAX      11.5    // m/s²
175: #define FREEFALL_VZ_THRESHOLD         -5.0    // m/s
176: #define FREEFALL_MIN_HEIGHT            5.0    // m
177: 
178: // Estado PARACHUTE (linhas 130-153 do Python)
179: #define PARACHUTE_ALTITUDE_THRESHOLD  100.0   // m
180: #define PARACHUTE_VZ_NEGATIVE          0.0    // m/s (vz < 0)
181: 
182: // Validações de Segurança
183: #define SAFETY_CHECK_NAN_INF    true  // Obrigatório em todas detecções
184: #define SAFETY_MIN_ALTITUDE     0.0   // m (guard contra false positives)
185: ```
186: 
187: **Validações de Segurança OBRIGATÓRIAS:**
188: ```cpp
189: // CRÍTICO: Todas as funções de detecção devem incluir:
190: // 1. Verificação NaN/Inf (linhas 34-35, 72-73, 95-96, 130-131 do Python)
191: if (!std::isfinite(vz) || !std::isfinite(az) || !std::isfinite(totalAccel)) {
192:     return false;  // Dados inválidos
193: }
194: 
195: // 2. State Guards - condições mínimas para evitar false positives
196: // Exemplo BURNOUT (linhas 47-48):
197: if (height <= BURNOUT_MIN_HEIGHT || vz <= BURNOUT_MIN_VZ) {
198:     return false;  // Muito baixo/lento para ser burnout
199: }
200: 
201: // 3. Clipping de Velocidade Vertical (linha 201 do Python)
202: vz = constrain(vz, -200.0, 200.0);
203: ```
204: 
205: ---
206: 
207: ## 📦 Bibliotecas e Dependências
208: 
209: ### Sensores
210: 
211: | Sensor Antigo | Sensor Novo | Biblioteca | Instalação |
212: |---------------|-------------|------------|------------|
213: | BMP280 | **BMP585** | `Adafruit_BMP5xx` | Library Manager |
214: | MPU6050 | **LSM6DS3** | `Adafruit_LSM6DS` | Library Manager |
215: | GPS N6M | **GPS N8M** | `TinyGPSPlus` (mantém) | Já instalado |
216: 
217: ### Dependências
218: - `Adafruit_Sensor` (unified sensor interface)
219: - `Adafruit_BusIO` (I2C/SPI abstraction)
220: - `ESPAsyncWebServer` (web interface)
221: - `ArduinoJson` (telemetry formatting)
222: - `LoRa` (wireless communication)
223: 
224: ### Instalação
225: ```bash
226: # Via Arduino Library Manager:
227: Adafruit BMP5xx
228: Adafruit LSM6DS
229: Adafruit Unified Sensor
230: Adafruit BusIO
231: TinyGPSPlus
232: ESPAsyncWebServer
233: ArduinoJson
234: LoRa
235: ```
236: 
237: ---
238: 
239: ## 🗂️ Estruturas de Dados
240: 
241: ### SensorData (Queue: Task 1 → Task 2)
242: 
243: ```cpp
244: struct SensorData {
245:   // Timestamp
246:   unsigned long timestamp;
247:   uint16_t packet_count;
248:   
249:   // BMP585
250:   float altitude;
251:   float pressure;
252:   float temperature;
253:   float verticalVelocity;
254:   float maxAltitude;
255:   
256:   // LSM6DS3
257:   float accelX, accelY, accelZ;
258:   float gyroX, gyroY, gyroZ;
259:   float totalAccel;  // 🆕 Calculado pelo LSM6DS3Sensor
260:   
261:   // GPS (apenas se fix disponível)
262:   double latitude, longitude;
263:   float gpsAltitude;
264:   uint8_t satellites;
265:   bool gps_valid;
266:   
267:   // FSM
268:   FlightState state;
269:   bool parachute_deployed;
270: };
271: 
272: // Queue: 25 slots × 64 bytes = ~1.6KB RAM
273: QueueHandle_t sensorDataQueue;
274: ```
275: 
276: ### LogMessage (Queue: Tasks 1,2 → Task 3)
277: 
278: ```cpp
279: struct LogMessage {
280:   char message[128];
281:   unsigned long timestamp;
282:   uint8_t taskId;    // 1=FSM, 2=Telemetry, 3=Logger
283:   uint8_t level;     // 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
284: };
285: 
286: // Queue: 50 slots × 140 bytes = ~7KB RAM
287: QueueHandle_t logQueue;
288: ```
289: 
290: ---
291: 
292: ## 📝 Plano de Implementação (9 Fases)
293: 
294: ### Checklist Geral
295: 
296: | Fase | Descrição | Tempo | Status |
297: |------|-----------|-------|--------|
298: | 1 | Setup e preparação | 30 min | ⏳ Pendente |
299: | 2 | Interface base + structs | 45 min | ⏳ Pendente |
300: | 3 | BMP585Sensor (classe) | 2 h | ⏳ Pendente |
301: | 4 | LSM6DS3Sensor (classe) | 1.5 h | ⏳ Pendente |
302: | 5 | GPSModule (classe) | 1 h | ⏳ Pendente |
303: | 6 | FSM - Máquina de estados | **4 h** | ⏳ Pendente |
304: | 7 | FreeRTOS Tasks | 4 h | ⏳ Pendente |
305: | 8 | Integração firmware.ino | 2 h | ⏳ Pendente |
306: | 9 | Adaptar módulos dependentes | 1.5 h | ⏳ Pendente |
307: | **TOTAL** | | **17.25h** | **0%** |
308: 
309: ---
310: 
311: ### FASE 1: Setup e Preparação ⏱️ 30min
312: 
313: **Status:** ⏳ Pendente
314: 
315: **Objetivos:**
316: - [ ] Criar estrutura de diretórios (`sensors/`, `flight/`)
317: - [ ] Reorganizar módulos existentes em `modules/`
318: - [ ] Instalar bibliotecas via Arduino Library Manager
319: - [ ] Criar branch `feature/oop-freertos-refactor`
320: 
321: **Ações:**
322: ```bash
323: # 1. Criar branch
324: git checkout -b feature/oop-freertos-refactor
325: 
326: # 2. Criar estrutura de pastas
327: mkdir -p sensors flight modules
328: 
329: # 3. Mover módulos existentes
330: mv buzzer_module.h modules/
331: mv filesystem_module.h modules/
332: mv lora_module.h modules/
333: mv parachute_module.h modules/
334: mv server_module.h modules/
335: 
336: # 4. Instalar bibliotecas (via Arduino IDE Library Manager)
337: # - Adafruit_BMP5xx
338: # - Adafruit_LSM6DS
339: # - Adafruit_Sensor
340: # - Adafruit_BusIO
341: ```
342: 
343: **Entregável:**
344: - ✅ Estrutura de pastas criada
345: - ✅ Bibliotecas instaladas
346: - ✅ Branch criado
347: 
348: **Notas:**
349: - Manter código antigo intacto até validação completa
350: - Testar bibliotecas com sketches simples antes de integrar
351: 
352: ---
353: 
354: ### FASE 2: Interface Base + Structs ⏱️ 45min
355: 
356: **Status:** ⏳ Pendente
357: 
358: **Objetivos:**
359: - [ ] Criar interface abstrata `ISensor`
360: - [ ] Criar structs de comunicação (`SensorData`, `LogMessage`)
361: - [ ] Definir enum `FlightState`
362: 
363: **Arquivos a Criar:**
364: 
365: **`sensors/ISensor.h`**
366: ```cpp
367: #ifndef ISENSOR_H
368: #define ISENSOR_H
369: 
370: #include <Arduino.h>
371: 
372: /**
373:  * @brief Interface abstrata para sensores do flight computer
374:  */
375: class ISensor {
376: public:
377:   virtual ~ISensor() = default;
378:   
379:   virtual bool begin() = 0;
380:   virtual void update() = 0;
381:   virtual String getData() = 0;
382:   virtual bool isReady() = 0;
383: };
384: 
385: #endif
386: ```
387: 
388: **`flight/SensorData.h`**
389: ```cpp
390: #ifndef SENSOR_DATA_H
391: #define SENSOR_DATA_H
392: 
393: #include <Arduino.h>
394: 
395: enum FlightState {
396:   IDLE = 0,
397:   LIFTOFF = 1,
398:   BURNOUT = 2,
399:   APOGEE = 3,
400:   FREEFALL = 4,
401:   PARACHUTE = 5,
402:   LANDED = 6
403: };
404: 
405: struct SensorData {
406:   unsigned long timestamp;
407:   uint16_t packet_count;
408:   
409:   float altitude;
410:   float pressure;
411:   float temperature;
412:   float verticalVelocity;
413:   float maxAltitude;
414:   
415:   float accelX, accelY, accelZ;
416:   float gyroX, gyroY, gyroZ;
417:   
418:   FlightState state;
419:   bool parachute_deployed;
420: };
421: 
422: struct LogMessage {
423:   char message[128];
424:   unsigned long timestamp;
425:   uint8_t taskId;
426:   uint8_t level;
427: };
428: 
429: #endif
430: ```
431: 
432: **Validação:**
433: - [ ] Compilação OK (headers apenas)
434: - [ ] Sem erros de sintaxe
435: 
436: ---
437: 
438: ### FASE 3: BMP585Sensor (Classe) ⏱️ 2h
439: 
440: **Status:** ⏳ Pendente
441: 
442: **Objetivos:**
443: - [ ] Criar classe do barômetro BMP585
444: - [ ] Migrar lógica de `bmp280_sensor.h`
445: - [ ] **IMPLEMENTAR cálculo de velocidade vertical (Vz)** - CRÍTICO para FSM
446: - [ ] Testar leitura de altitude/pressão/temperatura
447: 
448: **Arquivos:** `sensors/BMP585Sensor.h` + `sensors/BMP585Sensor.cpp`
449: 
450: **API Pública:**
451: ```cpp
452: class BMP585Sensor : public ISensor {
453: public:
454:   BMP585Sensor();
455:   
456:   // Interface ISensor
457:   bool begin() override;
458:   void update() override;
459:   String getData() override;
460:   bool isReady() override;
461:   
462:   // Específicos do barômetro
463:   float getAltitude() const;
464:   float getMaxAltitude() const;
465:   float getVerticalVelocity() const;  // ⚠️ CRÍTICO: Calculado via diferenciação numérica
466:   void checkHighest();
467:   
468: private:
469:   Adafruit_BMP5XX bmp;
470:   float base_pressure;
471:   float max_altitude;
472:   float prev_altitude;
473:   unsigned long prev_time;
474:   float vertical_velocity;  // Vz = (altitude_current - altitude_previous) / dt
475: };
476: ```
477: 
478: **Migração de `bmp280_sensor.h`:**
479: - ❌ Remover: variáveis globais `BMP`, `max_altitude`, `base_pressure`, etc.
480: - ✅ Transformar em: membros privados da classe
481: - ✅ Adaptar: `Adafruit_BMP280` → `Adafruit_BMP5XX`
482: - ✅ **IMPLEMENTAR cálculo de Vz** (referência: linha 201 do `FSM_Tester.py`)
483: 
484: **Cálculo de Velocidade Vertical (CRÍTICO):**
485: ```cpp
486: // Em BMP585Sensor::update()
487: void BMP585Sensor::update() {
488:   sensors_event_t temp_event, pressure_event;
489:   bmp.getEvent(&pressure_event, &temp_event);
490:   
491:   float altitude_current = bmp.readAltitude(base_pressure);
492:   unsigned long time_current = millis();
493:   
494:   // Diferenciação numérica (Python linha 201)
495:   float dt = (time_current - prev_time) / 1000.0;  // segundos
496:   if (dt > 0.001) {  // Evitar divisão por zero
497:     vertical_velocity = (altitude_current - prev_altitude) / dt;
498:     vertical_velocity = constrain(vertical_velocity, -200.0, 200.0);  // Clipping
499:     
500:     prev_altitude = altitude_current;
501:     prev_time = time_current;
502:   }
503: }
504: ```
505: 
506: **Teste Simples:**
507: ```cpp
508: // test_bmp585.ino
509: #include "sensors/BMP585Sensor.h"
510: 
511: BMP585Sensor baro;
512: 
513: void setup() {
514:   Serial.begin(115200);
515:   Wire.begin();
516:   
517:   if (baro.begin()) {
518:     Serial.println("BMP585 OK!");
519:   }
520: }
521: 
522: void loop() {
523:   baro.update();
524:   Serial.println(baro.getData());
525:   delay(1000);
526: }
527: ```
528: 
529: **Validação:**
530: - [ ] Compilação OK
531: - [ ] `begin()` retorna true
532: - [ ] `getData()` retorna CSV válido
533: - [ ] `getAltitude()` retorna valor razoável
534: 
535: ---
536: 
537: ### FASE 4: LSM6DS3Sensor (Classe) ⏱️ 1.5h
538: 
539: **Status:** ⏳ Pendente
540: 
541: **Objetivos:**
542: - [ ] Criar classe do IMU LSM6DS3
543: - [ ] Migrar lógica de `mpu6050_sensor.h`
544: - [ ] **IMPLEMENTAR cálculo de aceleração total** - CRÍTICO para FSM
545: - [ ] Testar leitura de aceleração/giroscópio
546: 
547: **Arquivos:** `sensors/LSM6DS3Sensor.h` + `sensors/LSM6DS3Sensor.cpp`
548: 
549: **API Pública:**
550: ```cpp
551: class LSM6DS3Sensor : public ISensor {
552: public:
553:   LSM6DS3Sensor();
554:   
555:   // Interface ISensor
556:   bool begin() override;
557:   void update() override;
558:   String getData() override;
559:   bool isReady() override;
560:   
561:   // Específicos do IMU
562:   float getAccelZ() const;
563:   float getTotalAccel() const;  // ⚠️ CRÍTICO: sqrt(ax² + ay² + az²)
564:   
565: private:
566:   Adafruit_LSM6DS3 lsm;
567:   float accelX, accelY, accelZ;
568:   float gyroX, gyroY, gyroZ;
569:   float total_accel;  // Calculado em update()
570: };
571: ```
572: 
573: **Migração:**
574: - ✅ API Adafruit_MPU6050 → Adafruit_LSM6DS3 (quase idêntica!)
575: - ✅ Manter uso de `sensors_event_t` (compatível)
576: - ✅ **IMPLEMENTAR cálculo de totalAccel** (referência: linha 37 do `FSM_Tester.py`)
577: 
578: **Cálculo de Aceleração Total (CRÍTICO):**
579: ```cpp
580: // Em LSM6DS3Sensor::update()
581: void LSM6DS3Sensor::update() {
582:   sensors_event_t accel, gyro, temp;
583:   lsm.getEvent(&accel, &gyro, &temp);
584:   
585:   accelX = accel.acceleration.x;
586:   accelY = accel.acceleration.y;
587:   accelZ = accel.acceleration.z;
588:   
589:   gyroX = gyro.gyro.x;
590:   gyroY = gyro.gyro.y;
591:   gyroZ = gyro.gyro.z;
592:   
593:   // Magnitude total da aceleração (Python linha 37)
594:   total_accel = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
595: }
596: ```
597: 
598: **Validação:**
599: - [ ] Compilação OK
600: - [ ] `begin()` retorna true
601: - [ ] `getAccelZ()` detecta gravidade (~9.8 m/s²)
602: 
603: ---
604: 
605: ### FASE 5: GPSModule (Classe) ⏱️ 1h
606: 
607: **Status:** ⏳ Pendente
608: 
609: **Objetivos:**
610: - [ ] Encapsular GPS em classe
611: - [ ] Manter `TinyGPSPlus` (sem mudança de biblioteca)
612: - [ ] Testar recepção de coordenadas
613: 
614: **Arquivos:** `sensors/GPSModule.h` + `sensors/GPSModule.cpp`
615: 
616: **API Pública:**
617: ```cpp
618: class GPSModule : public ISensor {
619: public:
620:   GPSModule(HardwareSerial* serial);
621:   
622:   bool begin() override;
623:   void update() override;
624:   String getData() override;
625:   bool isReady() override;
626:   
627:   String getTimeString() const;
628:   String getDateString() const;
629: };
630: ```
631: 
632: **Validação:**
633: - [ ] Compilação OK
634: - [ ] GPS recebe NMEA sentences
635: - [ ] `getTimeString()` retorna tempo válido após fix
636: 
637: ---
638: 
639: ### FASE 6: FSM - Máquina de Estados ⏱️ 4h
640: 
641: **Status:** ⏳ Pendente
642: 
643: **⚠️ ATENÇÃO: Esta é a fase mais crítica - tradução linha-por-linha do Python para C++**
644: 
645: **Objetivos:**
646: - [ ] Implementar FSM com **7 estados** (IDLE → LIFTOFF → BURNOUT → APOGEE → FREEFALL → PARACHUTE → LANDED)
647: - [ ] Traduzir lógica de detecção do `FSM_Tester.py` **linha por linha**
648: - [ ] Implementar validações de segurança obrigatórias (NaN/Inf, state guards)
649: - [ ] Testar transições com dados simulados
650: 
651: **Arquivos:** `flight/FlightStateMachine.h` + `flight/FlightStateMachine.cpp`
652: 
653: **Estrutura:**
654: ```cpp
655: class FlightStateMachine {
656: private:
657:   FlightState currentState;
658:   
659:   BMP585Sensor* baro;
660:   LSM6DS3Sensor* imu;
661:   
662:   // Contadores de debounce/confirmação
663:   uint8_t liftoffCounter;
664:   unsigned long apogeeDetectedTime;
665:   
666:   void checkTransitions();
667:   void transitionTo(FlightState newState);
668:   
669:   // Funções de detecção (traduzidas do Python)
670:   bool detectLiftoff();     // Python linhas 127-138
671:   bool detectBurnout();     // Python linhas 34-60
672:   bool detectApogee();      // Python linhas 68-80
673:   bool detectFreefall();    // Python linhas 86-114
674:   bool detectParachute();   // Python linhas 121-154
675:   bool detectLanded();      // [Implementação futura]
676: 
677: public:
678:   FlightStateMachine(BMP585Sensor* b, LSM6DS3Sensor* i);
679:   
680:   void begin();
681:   void update();
682:   FlightState getState() const;
683:   const char* getStateName() const;
684: };
685: ```
686: 
687: **Implementação das Funções de Detecção (Referência Completa):**
688: 
689: **`detectLiftoff()` - Python linhas 127-138:**
690: ```cpp
691: bool FlightStateMachine::detectLiftoff() {
692:   // LINHA 130-131: Validação de segurança
693:   float totalAccel = imu->getTotalAccel();
694:   if (!std::isfinite(totalAccel)) {
695:     return false;
696:   }
697:   
698:   // LINHA 137: Threshold validado
699:   if (totalAccel > LIFTOFF_TOTAL_ACCEL_THRESHOLD) {  // 15.0 m/s²
700:     liftoffCounter++;
701:     if (liftoffCounter >= 3) {  // Debounce: 3 leituras consecutivas
702:       return true;
703:     }
704:   } else {
705:     liftoffCounter = 0;
706:   }
707:   
708:   return false;
709: }
710: ```
711: 
712: **`detectBurnout()` - Python linhas 34-60:**
713: ```cpp
714: bool FlightStateMachine::detectBurnout() {
715:   // LINHA 34-35: Validação de segurança
716:   float az = imu->getAccelZ();
717:   float totalAccel = imu->getTotalAccel();
718:   float vz = baro->getVerticalVelocity();
719:   float height = baro->getAltitude();
720:   
721:   if (!std::isfinite(vz) || !std::isfinite(az) || !std::isfinite(totalAccel)) {
722:     return false;
723:   }
724:   
725:   // LINHA 47-48: State guards
726:   if (height <= BURNOUT_MIN_HEIGHT || vz <= BURNOUT_MIN_VZ) {
727:     return false;
728:   }
729:   
730:   // LINHA 53: Condição principal (OR lógico)
731:   bool condition1 = (az < BURNOUT_AZ_THRESHOLD);  // -8.0 m/s²
732:   bool condition2 = (totalAccel < BURNOUT_TOTAL_ACCEL_MIN);  // 2.0 m/s²
733:   
734:   return (condition1 || condition2);
735: }
736: ```
737: 
738: **`detectApogee()` - Python linhas 68-80:**
739: ```cpp
740: bool FlightStateMachine::detectApogee() {
741:   // LINHA 72-73: Validação de segurança
742:   float vz = baro->getVerticalVelocity();
743:   float az = imu->getAccelZ();
744:   
745:   if (!std::isfinite(vz) || !std::isfinite(az)) {
746:     return false;
747:   }
748:   
749:   // LINHA 76: Condição (valor absoluto de Vz)
750:   if (abs(vz) < APOGEE_VZ_THRESHOLD &&  // 1.0 m/s
751:       az < APOGEE_AZ_THRESHOLD) {        // -0.1 m/s²
752:     return true;
753:   }
754:   
755:   return false;
756: }
757: ```
758: 
759: **`detectFreefall()` - Python linhas 86-114:**
760: ```cpp
761: bool FlightStateMachine::detectFreefall() {
762:   // LINHA 95-96: Validação de segurança
763:   float totalAccel = imu->getTotalAccel();
764:   float vz = baro->getVerticalVelocity();
765:   float height = baro->getAltitude();
766:   
767:   if (!std::isfinite(totalAccel) || !std::isfinite(vz)) {
768:     return false;
769:   }
770:   
771:   // LINHA 108-109: State guards
772:   if (height <= FREEFALL_MIN_HEIGHT) {
773:     return false;
774:   }
775:   
776:   // LINHA 112: Condição principal (AND lógico)
777:   if (totalAccel < FREEFALL_TOTAL_ACCEL_MAX &&  // 11.5 m/s²
778:       vz < FREEFALL_VZ_THRESHOLD &&              // -5.0 m/s
779:       height > FREEFALL_MIN_HEIGHT) {            // 5.0 m
780:     return true;
781:   }
782:   
783:   return false;
784: }
785: ```
786: 
787: **`detectParachute()` - Python linhas 121-154:**
788: ```cpp
789: bool FlightStateMachine::detectParachute() {
790:   // LINHA 130-131: Validação de segurança
791:   float altitude = baro->getAltitude();
792:   float vz = baro->getVerticalVelocity();
793:   
794:   if (!std::isfinite(altitude) || !std::isfinite(vz)) {
795:     return false;
796:   }
797:   
798:   // LINHA 152: Condição principal
799:   if (altitude <= PARACHUTE_ALTITUDE_THRESHOLD &&  // 100.0 m
800:       vz < PARACHUTE_VZ_NEGATIVE) {                // vz < 0 (descendo)
801:     return true;
802:   }
803:   
804:   return false;
805: }
806: ```
807: 
808: **Lógica de Transições:**
809: 
810: | De | Para | Função | Linha Python |
811: |----|------|--------|--------------|
812: | IDLE | LIFTOFF | `detectLiftoff()` | 127-138 |
813: | LIFTOFF | BURNOUT | `detectBurnout()` | 34-60 |
814: | BURNOUT | APOGEE | `detectApogee()` | 68-80 |
815: | APOGEE | FREEFALL | `detectFreefall()` | 86-114 |
816: | FREEFALL | PARACHUTE | `detectParachute()` | 121-154 |
817: | PARACHUTE | LANDED | `detectLanded()` | [Implementação futura] |
818: 
819: **Loop Principal (`checkTransitions()`):**
820: ```cpp
821: void FlightStateMachine::checkTransitions() {
822:   switch (currentState) {
823:     case IDLE:
824:       if (detectLiftoff()) {
825:         transitionTo(LIFTOFF);
826:       }
827:       break;
828:       
829:     case LIFTOFF:
830:       if (detectBurnout()) {
831:         transitionTo(BURNOUT);
832:       }
833:       break;
834:       
835:     case BURNOUT:
836:       if (detectApogee()) {
837:         transitionTo(APOGEE);
838:       }
839:       break;
840:       
841:     case APOGEE:
842:       if (detectFreefall()) {
843:         transitionTo(FREEFALL);
844:       }
845:       break;
846:       
847:     case FREEFALL:
848:       if (detectParachute()) {
849:         transitionTo(PARACHUTE);
850:       }
851:       break;
852:       
853:     case PARACHUTE:
854:       if (detectLanded()) {
855:         transitionTo(LANDED);
856:       }
857:       break;
858:       
859:     case LANDED:
860:       // Estado final
861:       break;
862:   }
863: }
864: ```
865: 
866: **Validação:**
867: - [ ] Todas as 6 transições funcionam (IDLE→PARACHUTE validadas, LANDED pendente)
868: - [ ] Validações de segurança (NaN/Inf) impedem crashes
869: - [ ] State guards evitam false positives
870: - [ ] Simulação com dados do CSV (13_30_11-Dados.csv)
871: - [ ] Logs de transição claros com timestamps
872: 
873: ---
874: 
875: ### FASE 7: FreeRTOS Tasks ⏱️ 4h
876: 
877: **Status:** ⏳ Pendente
878: 
879: **Objetivos:**
880: - [ ] Implementar Task 1 (FlightControl - 50Hz)
881: - [ ] Implementar Task 2 (Telemetry - 5Hz)
882: - [ ] Implementar Task 3 (Logger - low priority)
883: - [ ] Criar Queues de comunicação
884: - [ ] Configurar Watchdog
885: 
886: **Arquivos:**
887: - `flight/FlightControlTask.h/cpp`
888: - `flight/TelemetryTask.h/cpp`
889: - `flight/LoggerTask.h/cpp`
890: 
891: **Task 1 - FlightControl:**
892: ```cpp
893: void taskFlightControl(void* parameter) {
894:   esp_task_wdt_add(NULL);  // Watchdog
895:   TickType_t xLastWakeTime = xTaskGetTickCount();
896:   
897:   while(1) {
898:     esp_task_wdt_reset();
899:     
900:     baroSensor->update();
901:     imuSensor->update();
902:     flightFSM->update();
903:     
904:     if (flightFSM->getState() == APOGEE && !parachute_deployed) {
905:       deployParachute();
906:       parachute_deployed = true;
907:       
908:       LogMessage log = {"[CRITICAL] Parachute deployed at APOGEE!", millis(), 1, 2};
909:       xQueueSend(logQueue, &log, 0);
910:     }
911:     
912:     SensorData data = { /* ... */ };
913:     xQueueSend(sensorDataQueue, &data, 0);
914:     
915:     vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
916:   }
917: }
918: ```
919: 
920: **Task 2 - Telemetry:**
921: ```cpp
922: void taskTelemetry(void* parameter) {
923:   SensorData data;
924:   TickType_t xLastWakeTime = xTaskGetTickCount();
925:   
926:   while(1) {
927:     gpsModule->update();
928:     
929:     if (xQueueReceive(sensorDataQueue, &data, 0) == pdPASS) {
930:       String telemetry = assembleTelemetry(data);
931:       
932:       Serial.println(telemetry);
933:       sendLoRa(telemetry);
934:       appendFile(file_dir, telemetry);
935:     }
936:     
937:     vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
938:   }
939: }
940: ```
941: 
942: **Task 3 - Logger:**
943: ```cpp
944: void taskLogger(void* parameter) {
945:   LogMessage log;
946:   
947:   while(1) {
948:     if (xQueueReceive(logQueue, &log, portMAX_DELAY) == pdPASS) {
949:       Serial.printf("[%lu][T%d] %s\n", 
950:         log.timestamp, log.taskId, log.message);
951:       
952:       #ifdef LOG_TO_FILE
953:         appendFile("/logs/flight.log", log.message);
954:       #endif
955:     }
956:   }
957: }
958: ```
959: 
960: **Configuração:**
961: ```cpp
962: // Watchdog
963: esp_task_wdt_init(5, true);  // 5s timeout, panic on trigger
964: 
965: // Queues
966: sensorDataQueue = xQueueCreate(25, sizeof(SensorData));
967: logQueue = xQueueCreate(50, sizeof(LogMessage));
968: 
969: // Tasks
970: xTaskCreatePinnedToCore(taskFlightControl, "FlightCtrl", 8192, NULL, 20, NULL, 1);
971: xTaskCreatePinnedToCore(taskTelemetry, "Telemetry", 16384, NULL, 5, NULL, 0);
972: xTaskCreatePinnedToCore(taskLogger, "Logger", 4096, NULL, 1, NULL, 0);
973: ```
974: 
975: **Validação:**
976: - [ ] 3 tasks executando em paralelo
977: - [ ] Queue não overflow (monitorar `uxQueueMessagesWaiting()`)
978: - [ ] Watchdog reseta se Task 1 travar (testar!)
979: 
980: ---
981: 
982: ### FASE 8: Integração firmware.ino ⏱️ 2h
983: 
984: **Status:** ⏳ Pendente
985: 
986: **Objetivos:**
987: - [ ] Refatorar `firmware.ino` principal
988: - [ ] Criar tasks no `setup()`
989: - [ ] `loop()` vazio (tasks assumem controle)
990: 
991: **Estrutura:**
992: ```cpp
993: #include <Wire.h>
994: #include <SPI.h>
995: 
996: #include "sensors/BMP585Sensor.h"
997: #include "sensors/LSM6DS3Sensor.h"
998: #include "sensors/GPSModule.h"
999: #include "flight/FlightStateMachine.h"
1000: #include "flight/FlightControlTask.h"
1001: #include "flight/TelemetryTask.h"
1002: #include "flight/LoggerTask.h"
1003: 
1004: #include "modules/buzzer_module.h"
1005: #include "modules/filesystem_module.h"
1006: #include "modules/lora_module.h"
1007: #include "modules/parachute_module.h"
1008: #include "modules/server_module.h"
1009: 
1010: BMP585Sensor* baroSensor;
1011: LSM6DS3Sensor* imuSensor;
1012: GPSModule* gpsModule;
1013: FlightStateMachine* flightFSM;
1014: 
1015: QueueHandle_t sensorDataQueue;
1016: QueueHandle_t logQueue;
1017: 
1018: void setup() {
1019:   Serial.begin(115200);
1020:   Wire.begin();
1021:   
1022:   baroSensor = new BMP585Sensor();
1023:   imuSensor = new LSM6DS3Sensor();
1024:   gpsModule = new GPSModule(&Serial1);
1025:   
1026:   if (!baroSensor->begin() || !imuSensor->begin() || !gpsModule->begin()) {
1027:     Serial.println("FATAL: Sensor init failed!");
1028:     while(1);
1029:   }
1030:   
1031:   flightFSM = new FlightStateMachine(baroSensor, imuSensor);
1032:   
1033:   setupServo();
1034:   setupLittleFS();
1035:   setupServer();
1036:   setupLoRa();
1037:   
1038:   sensorDataQueue = xQueueCreate(25, sizeof(SensorData));
1039:   logQueue = xQueueCreate(50, sizeof(LogMessage));
1040:   
1041:   esp_task_wdt_init(5, true);
1042:   
1043:   xTaskCreatePinnedToCore(taskFlightControl, "FlightCtrl", 8192, NULL, 20, NULL, 1);
1044:   xTaskCreatePinnedToCore(taskTelemetry, "Telemetry", 16384, NULL, 5, NULL, 0);
1045:   xTaskCreatePinnedToCore(taskLogger, "Logger", 4096, NULL, 1, NULL, 0);
1046: }
1047: 
1048: void loop() {
1049:   vTaskDelay(portMAX_DELAY);  // Tasks controlam tudo
1050: }
1051: ```
1052: 
1053: **Validação:**
1054: - [ ] Compilação OK
1055: - [ ] Sistema inicializa sem erros
1056: - [ ] Tasks executando corretamente
1057: 
1058: ---
1059: 
1060: ### FASE 9: Adaptar Módulos Dependentes ⏱️ 1.5h
1061: 
1062: **Status:** ⏳ Pendente
1063: 
1064: **Objetivos:**
1065: - [ ] Atualizar `parachute_module.h` para usar classes
1066: - [ ] Remover código antigo (bmp280_sensor.h, mpu6050_sensor.h, gps_module.h)
1067: - [ ] Validação final
1068: 
1069: **Módulos a Adaptar:**
1070: 
1071: **`modules/parachute_module.h`:**
1072: - Antes: `if (altitude < max_altitude - DESCENT_THRESHOLD)`
1073: - Depois: `void handleParachute(BMP585Sensor* baro)`
1074: 
1075: **Deletar:**
1076: - ❌ `bmp280_sensor.h`
1077: - ❌ `mpu6050_sensor.h`
1078: - ❌ `gps_module.h`
1079: 
1080: **Validação Final:**
1081: - [ ] Nenhum módulo acessa variáveis globais de sensores
1082: - [ ] Compilação limpa (sem warnings)
1083: - [ ] Código antigo removido
1084: 
1085: ---
1086: 
1087: ## 🧪 Validação e Testes
1088: 
1089: ### Checklist de Funcionalidades
1090: 
1091: **Sensores:**
1092: - [ ] BMP585 lê altitude/pressão/temperatura
1093: - [ ] LSM6DS3 lê aceleração/giroscópio (6 eixos)
1094: - [ ] GPS recebe fix e coordenadas
1095: - [ ] Calibração de base_pressure funciona
1096: 
1097: **FSM:**
1098: - [ ] Transição IDLE → LIFTOFF (totalAccel > 15.0 m/s²)
1099: - [ ] Transição LIFTOFF → BURNOUT (az < -8.0 OR totalAccel < 2.0)
1100: - [ ] Transição BURNOUT → APOGEE (|vz| < 1.0 AND az < -0.1)
1101: - [ ] Transição APOGEE → FREEFALL (totalAccel < 11.5 AND vz < -5.0)
1102: - [ ] Transição FREEFALL → PARACHUTE (altitude ≤ 100 AND vz < 0)
1103: - [ ] Validações de segurança (NaN/Inf) funcionando
1104: - [ ] State guards evitando false positives
1105: 
1106: **FreeRTOS:**
1107: - [ ] Task FSM roda a 50Hz preciso
1108: - [ ] Task Telemetry roda a 5Hz
1109: - [ ] Watchdog reseta se Task FSM travar
1110: - [ ] Queue não overflow durante operação normal
1111: 
1112: **Telemetria:**
1113: - [ ] Dados transmitidos via Serial
1114: - [ ] Dados transmitidos via LoRa
1115: - [ ] Dados gravados em `/data/HHMMSS-data.csv`
1116: - [ ] Logs gravados em `/logs/HHMMSS-log.txt` (se habilitado)
1117: 
1118: **Paraquedas:**
1119: - [ ] Deploy aciona no estado APOGEE
1120: - [ ] Servo move para posição correta
1121: - [ ] Buzzer toca sinal "Activated"
1122: 
1123: **WiFi Server:**
1124: - [ ] Access Point criado
1125: - [ ] Web interface acessível
1126: - [ ] Download de arquivos funciona
1127: - [ ] Deletar arquivos funciona
1128: 
1129: ### Testes de Estresse
1130: 
1131: **Simulações:**
1132: - [ ] Atraso Task Telemetry (500ms) - Task FSM continua?
1133: - [ ] Sensor travado - Watchdog reseta sistema?
1134: - [ ] Queue cheia - Sistema continua operando?
1135: - [ ] Voo completo simulado (IDLE → LANDED)
1136: 
1137: **Hardware:**
1138: - [ ] Teste de bancada (sensores reais)
1139: - [ ] Teste de queda livre (simular apogee)
1140: - [ ] Teste de vibração (simular liftoff)
1141: - [ ] Validação de dados CSV gerados
1142: 
1143: ---
1144: 
1145: ## 📊 Uso de Recursos
1146: 
1147: ### Memória Estimada
1148: 
1149: | Componente | RAM | Flash |
1150: |------------|-----|-------|
1151: | Código base | ~50 KB | ~200 KB |
1152: | Bibliotecas Adafruit | ~10 KB | ~50 KB |
1153: | FreeRTOS overhead | ~2 KB | ~20 KB |
1154: | Task stacks (3 × 8KB avg) | 28 KB | - |
1155: | Queues (25+50 slots) | ~8 KB | - |
1156: | Objetos (sensors, FSM) | ~5 KB | - |
1157: | **TOTAL** | **~103 KB** | **~270 KB** |
1158: 
1159: **Disponível:**
1160: - RAM: 512 KB → Uso: ~20% ✅
1161: - Flash: 8 MB → Uso: ~3% ✅
1162: 
1163: ### Performance
1164: 
1165: | Métrica | Valor | Limite |
1166: |---------|-------|--------|
1167: | Task FSM cycle time | ~5-10ms | 20ms (50Hz) |
1168: | Task Telemetry cycle | ~50-100ms | 200ms (5Hz) |
1169: | Queue send latency | <1ms | - |
1170: | Watchdog timeout | 5000ms | - |
1171: 
1172: ---
1173: 
1174: ## ⚠️ Riscos e Mitigações
1175: 
1176: | Risco | Prob. | Impacto | Mitigação |
1177: |-------|-------|---------|-----------|
1178: | API Adafruit_BMP5xx diferente | Média | Alto | Testar biblioteca ANTES (Fase 3) |
1179: | Deadlock entre tasks | Baixa | Alto | Nunca usar `portMAX_DELAY` em Task FSM |
1180: | Queue overflow | Média | Médio | Monitorar `uxQueueMessagesWaiting()` |
1181: | Watchdog falso positivo | Baixa | Alto | Testar com timeout maior (10s) primeiro |
1182: | GPS sem fix indoor | Alta | Baixo | Usar fallback "NOFIX" para filename |
1183: | Filesystem lento | Média | Baixo | OK, Task 2 pode atrasar |
1184: 
1185: ---
1186: 
1187: ## 🐛 Troubleshooting
1188: 
1189: ### Problemas Comuns
1190: 
1191: **"BMP585 initialization failed"**
1192: - Verificar conexões I2C (SDA/SCL)
1193: - Testar com I2C scanner
1194: - Verificar endereço I2C (0x76 ou 0x77)
1195: 
1196: **"Task watchdog got triggered"**
1197: - Task FSM travou (sensor não responde?)
1198: - Aumentar timeout temporariamente
1199: - Adicionar logs antes de operações bloqueantes
1200: 
1201: **"Queue full" ou dados perdidos**
1202: - Task Telemetry muito lenta (filesystem?)
1203: - Aumentar queue size (25 → 50)
1204: - Otimizar Task 2 (remover delays desnecessários)
1205: 
1206: **FSM não transita de estado**
1207: - Verificar thresholds em `config.h`
1208: - Adicionar logs de debug nas condições
1209: - Testar com valores simulados
1210: 
1211: ---
1212: 
1213: ## 📚 Referências
1214: 
1215: ### Documentação
1216: - [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
1217: - [FreeRTOS API Reference](https://www.freertos.org/a00106.html)
1218: - [Adafruit BMP5xx Guide](https://learn.adafruit.com/adafruit-bmp5xx)
1219: - [Adafruit LSM6DS Guide](https://learn.adafruit.com/lsm6ds)
1220: - [TinyGPS++ Documentation](http://arduiniana.org/libraries/tinygpsplus/)
1221: 
1222: ### Código de Referência
1223: - [firmware.ino original](firmware.ino) - Código procedural (EOL)
1224: - [MODULOS.md](MODULOS.md) - Documentação módulos antigos
1225: - **[extras/FSM_tester/FSM_Tester.py](../extras/FSM_tester/FSM_Tester.py)** - ⚠️ **CRÍTICO:** FSM validado em Python
1226: - **[extras/FSM_tester/explicacao.md](../extras/FSM_tester/explicacao.md)** - Documentação completa do FSM
1227: - **[extras/FSM_tester/13_30_11-Dados.csv](../extras/FSM_tester/13_30_11-Dados.csv)** - Dataset de validação (1,873 pontos)
1228: 
1229: ---
1230: 
1231: ## 📝 Changelog
1232: 
1233: ### 2026-03-18 - Planejamento Completo (v1.0)
1234: - ✅ Arquitetura definida (POO + FreeRTOS + FSM)
1235: - ✅ Decisões técnicas tomadas
1236: - ✅ Plano de 9 fases criado
1237: - ✅ Estimativas de tempo e recursos
1238: 
1239: ### 2026-03-18 - Atualização v2.0 (FSM Validado)
1240: - ✅ FSM corrigido para **7 estados** (adicionado PARACHUTE)
1241: - ✅ Thresholds validados com dados reais (1,873 pontos de telemetria)
1242: - ✅ Referências linha-por-linha ao código Python
1243: - ✅ Validações de segurança obrigatórias documentadas
1244: - ✅ Cálculos de Vz e totalAccel especificados
1245: - ✅ Tempo total ajustado: 16.25h → 17.25h (Fase 6 aumentada)
1246: 
1247: ### Próximas Etapas
1248: - ⏳ Aguardando início da implementação (Fase 1)
1249: 
1250: ---
1251: 
1252: ## 🤝 Contribuindo
1253: 
1254: ### Para Desenvolvedores
1255: 
1256: **Antes de começar uma fase:**
1257: 1. Ler esta documentação completa
1258: 2. Marcar fase como "Em Progresso" (⏳ → 🔄)
1259: 3. Criar branch específica: `git checkout -b fase-N-descricao`
1260: 
1261: **Durante implementação:**
1262: 1. Seguir estrutura proposta
1263: 2. Fazer commits pequenos e frequentes
1264: 3. Testar cada componente isoladamente
1265: 
1266: **Ao concluir fase:**
1267: 1. Marcar como "Concluída" (🔄 → ✅)
1268: 2. Validar checklist da fase
1269: 3. Fazer PR para branch principal
1270: 4. Atualizar este documento
1271: 
1272: ### Para Agentes de Código
1273: 
1274: **Você pode usar este documento para:**
1275: - Entender arquitetura do sistema
1276: - Consultar decisões técnicas
1277: - Verificar progresso das fases
1278: - Tirar dúvidas sobre implementação
1279: - Encontrar referências e exemplos
1280: 
1281: **Ao fazer modificações:**
1282: - Sempre atualizar este documento
1283: - Manter coerência com decisões tomadas
1284: - Seguir estrutura de pastas definida
1285: - Respeitar convenções de nomenclatura
1286: 
1287: ---
1288: 
1289: ## 📞 Contato
1290: 
1291: **Team #100 Avionics**  
1292: **Projeto:** Flight Computer  
1293: **Repositório:** `/home/vinicius/Documentos/Projetos/flight-computer/`
1294: 
1295: ---
1296: 
1297: **Última atualização:** 2026-03-18  
1298: **Versão do documento:** 2.0  
1299: **Status geral:** 📋 Planejamento Completo - Pronto para Implementação
1300: 
1301: ---
1302: 
1303: ## ⚠️ ATUALIZAÇÃO v2.0: FSM Validado com Dados Reais
1304: 
1305: **IMPORTANTE:** Este documento foi atualizado para refletir o **FSM de 7 estados** validado com dados reais de voo.
1306: 
1307: **Fonte de Validação:**
1308: - Arquivo: `extras/FSM_tester/FSM_Tester.py` (implementação Python completa)
1309: - Dataset: `extras/FSM_tester/13_30_11-Dados.csv` (1,873 pontos de telemetria)
1310: - Documentação: `extras/FSM_tester/explicacao.md` (lógica completa com referências de linha)
1311: 
1312: **Mudanças principais:**
1313: 1. FSM agora possui **7 estados** (não 6): `IDLE → LIFTOFF → BURNOUT → APOGEE → FREEFALL → PARACHUTE → LANDED`
1314: 2. Thresholds validados com dados reais (não estimativas)
1315: 3. Validações de segurança obrigatórias (NaN/Inf checks, state guards)
1316: 4. Lógica linha-por-linha documentada para tradução C++

(End of file - total 1316 lines)
</content>
```

---

## Assistant (Plan · mimo-v2-pro-free · 10.3s)

_Thinking:_

Now let me also read the current firmware.ino to understand the current implementation:

Now let me read the current firmware to understand the existing parachute logic:

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/firmware.ino"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/firmware.ino</path>
<type>file</type>
<content>1: /**
2:  * @file firmware.ino
3:  * @brief Main firmware entry point for Team #100 Flight Computer (Avionics System)
4:  * 
5:  * This is the main program file for a rocket/drone flight computer that handles
6:  * sensor data collection, parachute deployment, telemetry transmission, and
7:  * data storage. The system uses a modular architecture with separate header files
8:  * for each subsystem.
9:  * 
10:  * System capabilities:
11:  * - Altitude and atmospheric pressure monitoring (BMP280)
12:  * - Inertial measurement unit for orientation (MPU6050)
13:  * - GPS position and time tracking
14:  * - Long-range telemetry via LoRa radio
15:  * - Autonomous parachute deployment based on flight profile
16:  * - Data logging to onboard flash storage (LittleFS)
17:  * - WiFi access point for data retrieval and management
18:  * - Audio feedback via buzzer for system status
19:  * 
20:  * Flight phases:
21:  * 1. Initialization: System startup and sensor calibration
22:  * 2. Pre-launch: Monitoring and data collection on ground
23:  * 3. Ascent: High-frequency data logging during powered flight
24:  * 4. Apogee detection: Tracking maximum altitude
25:  * 5. Descent: Parachute deployment and controlled landing
26:  * 6. Recovery: Post-flight data access via WiFi
27:  * 
28:  * @note All configuration parameters are in config.h
29:  * @note Data logging interval is 200ms (5Hz) as defined by INTERVAL
30:  * 
31:  * @author Team #100 Avionics
32:  * @date 2024
33:  */
34: 
35: //==============================================================================
36: // LIBRARY INCLUDES
37: //==============================================================================
38: 
39: #include <Wire.h>    // I2C communication for BMP280 and MPU6050
40: #include <SPI.h>     // SPI communication for LoRa module
41: 
42: //==============================================================================
43: // MODULE INCLUDES
44: //==============================================================================
45: 
46: #include "config.h"             // Global configuration and constants
47: #include "bmp280_sensor.h"      // Barometric pressure and altitude sensor
48: #include "mpu6050_sensor.h"     // Inertial measurement unit (IMU)
49: #include "gps_module.h"         // GPS positioning and timing
50: #include "lora_module.h"        // LoRa long-range radio communication
51: #include "filesystem_module.h"  // LittleFS data storage
52: #include "parachute_module.h"   // Parachute deployment control
53: #include "buzzer_module.h"      // Audio feedback and alerts
54: #include "server_module.h"      // WiFi access point and web server
55: #include "telemetry_module.h"   // Data aggregation and logging
56: 
57: //==============================================================================
58: // SETUP - ONE-TIME INITIALIZATION
59: //==============================================================================
60: 
61: /**
62:  * @brief Initialize all system components and prepare for flight
63:  * 
64:  * This function runs once at power-on and performs the following operations:
65:  * 1. Initialize communication buses (Serial, I2C)
66:  * 2. Initialize servo motor for parachute deployment
67:  * 3. Startup delay for system stabilization
68:  * 4. Acquire GPS time for unique filename generation
69:  * 5. Initialize filesystem and create data file with CSV header
70:  * 6. Start WiFi access point and web server
71:  * 7. Initialize all sensors (BMP280, MPU6050) and LoRa radio
72:  * 8. Provide audio feedback on initialization status
73:  * 
74:  * The system will restart automatically if critical initialization fails
75:  * (e.g., filesystem mount error). Non-critical errors (sensor failures)
76:  * are logged but allow the system to continue operating.
77:  * 
78:  * @note Serial monitor must be set to 115200 baud
79:  * @note The 5-second startup delay allows time to open Serial monitor
80:  * @warning System will restart on filesystem initialization failure
81:  * 
82:  * @see setup() is called automatically once by Arduino framework
83:  */
84: void setup()
85: {
86:   //----------------------------------------------------------------------------
87:   // Communication and Hardware Initialization
88:   //----------------------------------------------------------------------------
89:   
90:   Serial.begin(115200);   // Initialize USB serial at 115200 baud
91:   Wire.begin();           // Initialize I2C bus for sensors (SDA/SCL default pins)
92:   setupServo();           // Initialize servo motor for parachute deployment
93:   
94:   //----------------------------------------------------------------------------
95:   // Startup Delay and Status Messages
96:   //----------------------------------------------------------------------------
97:   
98:   pinMode(BUZZER_PIN, OUTPUT);  // Configure buzzer pin as output
99:   
100:   // 5-second countdown with status messages
101:   // Provides time to open Serial monitor and stabilize sensors
102:   for (int i = 0; i < 5; i++)
103:   {
104:     Serial.println("Initializing...");
105:     delay(1000);  // 1 second delay per iteration
106:   }
107: 
108:   //----------------------------------------------------------------------------
109:   // GPS Time Acquisition and Filename Generation
110:   //----------------------------------------------------------------------------
111:   
112:   setupGPS();  // Initialize GPS module and begin receiving data
113: 
114:   // Attempt to get GPS time for unique filename, fallback to default if unavailable
115:   // Format: HHMMSS-data.csv (e.g., "143052-data.csv" for 2:30:52 PM)
116:   String time_data = getGPSTimeString();
117:   file_dir = "/" + time_data + "-" + file_name;
118:   Serial.print("Saving data to: ");
119:   Serial.println(file_dir);
120: 
121:   //----------------------------------------------------------------------------
122:   // Filesystem Initialization and Data File Creation
123:   //----------------------------------------------------------------------------
124:   
125:   // CSV header defining all telemetry fields
126:   String data_header = "TEAM_ID,millis,count,altp,temp,umi,p,gp,gr,gy,ap,ar,ay,hora,data,alt,lat,lon,sat,pqd";
127:   
128:   // Mount filesystem and create data file with header
129:   // Critical operation - system will restart on failure
130:   if (!(setupLittleFS() && writeFile(file_dir, data_header)))
131:   {
132:     Serial.println("Filesystem error!");
133:     buzzSignal("Alert");    // 5 rapid beeps to indicate error
134:     delay(3000);            // Allow time to read error message
135:     ESP.restart();          // Restart system to retry initialization
136:   }
137: 
138:   //----------------------------------------------------------------------------
139:   // WiFi Access Point and Web Server Initialization
140:   //----------------------------------------------------------------------------
141:   
142:   setupServer();  // Create WiFi AP and start web server for data access
143: 
144:   //----------------------------------------------------------------------------
145:   // Sensor and Communication Module Initialization
146:   //----------------------------------------------------------------------------
147:   
148:   // Initialize all sensors and LoRa radio
149:   // Non-critical - system continues if initialization fails
150:   if (!(setupBMP() && setupMPU() && setupLoRa()))
151:   {
152:     printBoth("Module configuration error!");  // Log error to Serial and LoRa
153:     buzzSignal("Alert");                       // Audio alert (5 beeps)
154:     delay(3000);                               // Delay for error acknowledgment
155:   }
156:   else
157:   {
158:     printBoth("All modules initialized successfully!");  // Success message
159:     buzzSignal("Success");                                // Audio confirmation (3 beeps)
160:   }
161: }
162: 
163: //==============================================================================
164: // MAIN LOOP - CONTINUOUS OPERATION
165: //==============================================================================
166: 
167: /**
168:  * @brief Main flight computer control loop
169:  * 
170:  * This function runs continuously after setup() completes. It performs
171:  * time-based sensor sampling, data logging, and parachute deployment logic
172:  * at regular intervals defined by INTERVAL (default: 200ms = 5Hz).
173:  * 
174:  * Operations performed each cycle:
175:  * 1. Check if logging interval has elapsed (200ms)
176:  * 2. Read current altitude from BMP280 sensor
177:  * 3. Calculate vertical velocity from altitude change
178:  * 4. Log telemetry data (all sensors + timestamp + parachute status)
179:  * 5. Update highest altitude reached (for apogee detection)
180:  * 6. Evaluate parachute deployment conditions
181:  * 7. Update timestamp for next cycle
182:  * 
183:  * Parachute deployment logic (see parachute_module.h for details):
184:  * - Must be descending from apogee (altitude drop threshold)
185:  * - AND either: altitude below threshold OR descent velocity exceeds threshold
186:  * - Once deployed, parachute remains deployed (no retraction)
187:  * 
188:  * @note The web server runs asynchronously and doesn't need to be called here
189:  * @note GPS updates happen automatically via hardware serial interrupts
190:  * @note Loop frequency is controlled by INTERVAL constant in config.h
191:  * 
192:  * @see loop() is called repeatedly by Arduino framework
193:  * @see INTERVAL is defined in config.h (default: 200ms)
194:  * @see handleParachute() in parachute_module.h for deployment logic
195:  */
196: void loop()
197: {
198:   //----------------------------------------------------------------------------
199:   // Time-Based Execution Control
200:   //----------------------------------------------------------------------------
201:   
202:   unsigned long current_millis = millis();  // Get current time in milliseconds
203:   
204:   // Execute data logging and control logic at fixed interval (200ms)
205:   if (current_millis - previous_millis >= INTERVAL)
206:   {
207:     //--------------------------------------------------------------------------
208:     // Sensor Reading and Calculation
209:     //--------------------------------------------------------------------------
210:     
211:     // Read current altitude from pressure sensor
212:     float altitude = BMP.readAltitude(base_pressure);
213:     
214:     // Calculate vertical velocity (m/s) from altitude change over time
215:     // Positive = ascending, Negative = descending
216:     float velocity = (altitude - previous_altitude) / ((current_millis - previous_millis) / 1000.0);
217:     
218:     //--------------------------------------------------------------------------
219:     // Data Logging and Telemetry
220:     //--------------------------------------------------------------------------
221:     
222:     // Log complete telemetry packet to Serial, LoRa, and filesystem
223:     logData(current_millis, parachute_deployed);
224:     
225:     //--------------------------------------------------------------------------
226:     // Flight State Management
227:     //--------------------------------------------------------------------------
228:     
229:     // Update maximum altitude reached (for apogee detection)
230:     checkHighest(altitude);
231:     
232:     // Evaluate parachute deployment conditions and deploy if criteria met
233:     handleParachute(altitude, velocity);
234:     
235:     //--------------------------------------------------------------------------
236:     // Timestamp Update for Next Cycle
237:     //--------------------------------------------------------------------------
238:     
239:     previous_millis = current_millis;  // Update timestamp for next interval
240:   }
241: }

(End of file - total 241 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/config.h"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/config.h</path>
<type>file</type>
<content>1: /**
2:  * @file config.h
3:  * @brief Global configuration file for the avionics system
4:  * 
5:  * This file contains all pin definitions, constants, and parameters
6:  * used throughout the system. Centralizing configurations here makes
7:  * maintenance and parameter modifications easier.
8:  * 
9:  * @author Team #100
10:  * @date 2026
11:  */
12: 
13: #ifndef CONFIG_H
14: #define CONFIG_H
15: 
16: #include <Arduino.h>
17: 
18: //==============================================================================
19: // TIMING AND LOOP CONFIGURATION
20: //==============================================================================
21: 
22: /**
23:  * Main loop execution interval in milliseconds
24:  * 200ms = ~5Hz sampling rate
25:  * Adjust this value to increase/decrease data collection rate
26:  */
27: #define INTERVAL 200
28: 
29: //==============================================================================
30: // LORA COMMUNICATION CONFIGURATION
31: //==============================================================================
32: 
33: /**
34:  * LoRa module operating frequency in Hz
35:  * 868 MHz is the ISM frequency used in Europe
36:  * For Brazil/Americas, consider 915 MHz (915E6)
37:  */
38: #define LORA_FREQ 868E6
39: 
40: /**
41:  * Slave Select (SS/CS) pin for SPI communication with LoRa module
42:  */
43: #define SS_LORA 7
44: 
45: /**
46:  * Reset pin for LoRa module
47:  * Used to reinitialize the module in case of issues
48:  */
49: #define RST_LORA 1
50: 
51: /**
52:  * DIO0 pin of LoRa module
53:  * Used for interrupts and transmission complete signaling
54:  */
55: #define DIO0_LORA 2
56: 
57: /**
58:  * Synchronization word for LoRa communication
59:  * Ensures only devices with the same word communicate
60:  * Change this value to create isolated LoRa networks
61:  */
62: #define SYNC_WORD 0xF3
63: 
64: //==============================================================================
65: // PIN DEFINITIONS - ACTUATORS
66: //==============================================================================
67: 
68: /**
69:  * Digital pin connected to parachute servo motor
70:  * Uses PWM to control servo position
71:  */
72: #define SERVO_PIN 3
73: 
74: /**
75:  * Digital pin connected to piezoelectric buzzer
76:  * Emits sound signals for status indication
77:  */
78: #define BUZZER_PIN 0
79: 
80: //==============================================================================
81: // PIN DEFINITIONS - GPS
82: //==============================================================================
83: 
84: /**
85:  * RX pin for serial communication with GPS module
86:  * Connects to GPS module TX
87:  */
88: #define RX_GPS 20
89: 
90: /**
91:  * TX pin for serial communication with GPS module
92:  * Connects to GPS module RX
93:  */
94: #define TX_GPS 21
95: 
96: //==============================================================================
97: // PARACHUTE CONTROL CONSTANTS
98: //==============================================================================
99: 
100: /**
101:  * Maximum servo position (parachute open)
102:  * Value in degrees: 0° = fully open
103:  */
104: const int MAXPOS = 0;
105: 
106: /**
107:  * Minimum servo position (parachute closed)
108:  * Value in degrees: 90° = fully closed
109:  */
110: const int MINPOS = 90;
111: 
112: /**
113:  * Altitude drop threshold in meters
114:  * Parachute will only deploy after altitude drops this value
115:  * below the maximum peak reached (detects descent after apogee)
116:  */
117: const float ALTITUDE_DROP_THRESHOLD = 10.0;
118: 
119: /**
120:  * Minimum altitude for parachute deployment in meters
121:  * Prevents opening too high (outside recovery zone)
122:  * Adjust according to competition rules
123:  */
124: const float ALTITUDE_THRESHOLD = 750.0;
125: 
126: /**
127:  * Minimum descent velocity for parachute deployment in m/s
128:  * Ensures parachute only opens in free fall
129:  * Absolute value: 80 m/s ≈ 288 km/h
130:  */
131: const float VELOCITY_THRESHOLD = 80.0;
132: 
133: //==============================================================================
134: // TEAM IDENTIFICATION
135: //==============================================================================
136: 
137: /**
138:  * Team unique identifier
139:  * Used as prefix in all telemetry transmissions
140:  * Allows identifying data from different teams
141:  */
142: const String TEAM_ID = "#100";
143: 
144: //==============================================================================
145: // FILESYSTEM CONFIGURATION
146: //==============================================================================
147: 
148: /**
149:  * CSV data file name
150:  * Stores all telemetry readings
151:  */
152: String file_name = "Dados.csv";
153: 
154: /**
155:  * Complete data file path
156:  * Will be filled during setup() with GPS timestamp
157:  * Format: /HH_MM_SS-Dados.csv
158:  */
159: String file_dir = "";
160: 
161: //==============================================================================
162: // WEB SERVER CONFIGURATION
163: //==============================================================================
164: 
165: /**
166:  * WiFi network name (SSID) for access point
167:  * Concatenates "Server" with team ID
168:  */
169: String ssid_str = "Server " + TEAM_ID;
170: 
171: /**
172:  * SSID in C-string format for WiFi library compatibility
173:  */
174: const char *ssid = ssid_str.c_str();
175: 
176: /**
177:  * WiFi network password
178:  * WARNING: Change to a more secure password in production environment
179:  */
180: const char *password = "Iamarobot";
181: 
182: #endif // CONFIG_H

(End of file - total 182 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/parachute_module.h"
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/parachute_module.h</path>
<type>file</type>
<content>1: /**
2:  * @file parachute_module.h
3:  * @brief Parachute deployment control module
4:  * 
5:  * This module manages the recovery system, controlling parachute deployment.
6:  * The parachute is deployed using a servo motor that releases the parachute compartment.
7:  * 
8:  * Deployment logic:
9:  * 1. Detects apogee (maximum altitude reached)
10:  * 2. Waits for descent confirmation (altitude drops below apogee)
11:  * 3. Checks safety conditions (altitude and velocity thresholds)
12:  * 4. Activates servo to deploy parachute
13:  * 5. Verifies deployment success
14:  * 
15:  * Safety features:
16:  * - Minimum altitude threshold (prevents deployment too high)
17:  * - Velocity threshold (confirms free fall)
18:  * - Apogee detection (waits until descent starts)
19:  * - One-time deployment (prevents multiple activations)
20:  * - Servo verification (checks position after actuation)
21:  * 
22:  * @author Team #100
23:  * @date 2026
24:  */
25: 
26: #ifndef PARACHUTE_MODULE_H
27: #define PARACHUTE_MODULE_H
28: 
29: #include <Arduino.h>
30: #include <ESP32Servo.h>
31: #include "config.h"
32: #include "bmp280_sensor.h"
33: 
34: //==============================================================================
35: // PARACHUTE GLOBAL VARIABLES
36: //==============================================================================
37: 
38: /**
39:  * Servo motor object for parachute deployment
40:  * Controls mechanical release mechanism
41:  */
42: Servo ParachuteServo;
43: 
44: /**
45:  * Parachute deployment status flag
46:  * true: Parachute has been deployed
47:  * false: Parachute is still packed
48:  * Prevents multiple deployment attempts
49:  */
50: bool parachute_deployed = false;
51: 
52: //==============================================================================
53: // FORWARD DECLARATIONS
54: //==============================================================================
55: 
56: /**
57:  * Forward declaration for printBoth function
58:  * Defined in telemetry_module.h
59:  * Used to send deployment status via Serial and LoRa
60:  */
61: void printBoth(const String &message);
62: 
63: //==============================================================================
64: // INITIALIZATION FUNCTIONS
65: //==============================================================================
66: 
67: /**
68:  * Initialize servo motor and set to closed position
69:  * 
70:  * Attaches servo to control pin and moves it to locked position (MINPOS).
71:  * The 500ms delay allows servo to reach position before flight operations begin.
72:  * 
73:  * Servo positions:
74:  * - MINPOS (90°): Parachute compartment locked/closed
75:  * - MAXPOS (0°): Parachute compartment released/open
76:  * 
77:  * @note Call during setup() before flight operations
78:  * @see config.h for SERVO_PIN, MINPOS, and MAXPOS definitions
79:  */
80: void setupServo()
81: {
82:   // Attach servo to PWM pin
83:   ParachuteServo.attach(SERVO_PIN);
84:   
85:   // Wait for servo to initialize
86:   delay(500);
87:   
88:   // Move to closed/locked position
89:   ParachuteServo.write(MINPOS);
90: }
91: 
92: //==============================================================================
93: // PARACHUTE DEPLOYMENT LOGIC
94: //==============================================================================
95: 
96: /**
97:  * Handle parachute deployment based on altitude and velocity criteria
98:  * 
99:  * Implements multi-condition deployment logic to ensure safe parachute release:
100:  * 
101:  * Deployment conditions (ALL must be true):
102:  * 1. altitude <= max_altitude - ALTITUDE_DROP_THRESHOLD (descent detected)
103:  * 2. altitude < ALTITUDE_THRESHOLD OR abs(velocity) > VELOCITY_THRESHOLD
104:  *    (either below minimum altitude OR in free fall)
105:  * 3. !parachute_deployed (not already deployed)
106:  * 
107:  * Deployment sequence:
108:  * 1. Command servo to MAXPOS (open position)
109:  * 2. Wait up to 500ms for servo to reach position
110:  * 3. Verify servo reached MAXPOS
111:  * 4. Report deployment status
112:  * 5. Set parachute_deployed flag
113:  * 
114:  * After deployment:
115:  * - Updates previous_altitude for velocity calculation
116:  * - Continues to emit "Activated" buzzer signal (handled externally)
117:  * 
118:  * @param altitude Current barometric altitude in meters
119:  * @param velocity Current vertical velocity in m/s (negative = descending)
120:  * 
121:  * @note Velocity calculated from altitude difference between loop iterations
122:  * @note max_altitude is updated by checkHighest() in bmp280_sensor.h
123:  * @see config.h for threshold values
124:  * @see bmp280_sensor.h for altitude and max_altitude variables
125:  */
126: void handleParachute(float altitude, float velocity)
127: {
128:   // Check if parachute has already been deployed
129:   if (!parachute_deployed)
130:   {
131:     // Check all deployment conditions
132:     if (altitude <= max_altitude - ALTITUDE_DROP_THRESHOLD && 
133:         (altitude < ALTITUDE_THRESHOLD || abs(velocity) > VELOCITY_THRESHOLD))
134:     {
135:       // Command servo to open position
136:       ParachuteServo.write(MAXPOS);
137:       
138:       // Record deployment initiation time
139:       unsigned long startTime = millis();
140:       
141:       // Wait up to 500ms for servo to reach target position
142:       while (millis() - startTime < 500)
143:       {
144:         if (ParachuteServo.read() == MAXPOS)
145:         {
146:           // Servo reached target position
147:           break;
148:         }
149:       }
150:       
151:       // Verify servo successfully moved to open position
152:       if (ParachuteServo.read() != MAXPOS)
153:       {
154:         printBoth("ERROR: Servo failed to open!");
155:       }
156:       
157:       // Report deployment with telemetry
158:       printBoth("Parachute deployed. Altitude: " + String(altitude) + 
159:                 " Vel: " + String(velocity));
160:       
161:       // Set flag to prevent re-deployment
162:       parachute_deployed = true;
163:     }
164:   }
165:   
166:   // Update previous altitude for next velocity calculation
167:   previous_altitude = altitude;
168: }
169: 
170: #endif // PARACHUTE_MODULE_H

(End of file - total 170 lines)
</content>
```

---

## Assistant (Plan · mimo-v2-pro-free · 57.4s)

_Thinking:_

Now I have a comprehensive understanding of all three pieces. Let me analyze the main differences:

1. **test/FSM/FSM.ino** - The "approved" functional FSM implementation in C++ for Arduino
2. **extras/FSM_tester/FSM_Tester.py** - The Python FSM tester/simulator that was used for validation
3. **firmware/** - The current firmware code + the REFACTORING_PLAN.md with the planned architecture

Let me identify the key differences:

### 1. Number of States / State Machine Design

**test/FSM (approved):** 4 states
- IDLE, ASCENT, DESCENT, LANDED


**Python tester:** 6 event flags without formal FSM states, relying on sequential if/elif chains to track LIFTOFF, BURNOUT, APOGEE, FREEFALL, and PARACHUTE transitions
**Plan:** 7 states - IDLE, LIFTOFF, BURNOUT, APOGEE, FREEFALL, PARACHUTE, LANDED
**Current firmware:** No formal FSM exists; parachute deployment uses ad-hoc condition checks

The approved FSM deliberately collapsed the event chain into 4 states for simplicity, whereas the plan maintains granular event tracking through all 7 states.

Now I'm comparing how detection logic differs across implementations. The approved version checks for liftoff when acceleration exceeds 15, and burnout when acceleration drops below 2.0 or goes negative below -8, requiring altitude greater than 5 and velocity above 0.5. Apogee detection looks for near-zero vertical velocity with slight negative acceleration, freefall triggers on low acceleration and rapid descent, and parachute deployment fires at 100m altitude with descending velocity. The Python tester follows similar thresholds but uses a different freefall velocity threshold and...

Comparing detection thresholds... Looking at the parachute deployment logic across the different implementations—the current firmware uses a two-condition approach that checks both absolute altitude and velocity thresholds, while the test and plan versions use a simpler single condition. Then there's the apogee detection, where the current firmware relies on a straightforward relative altitude drop, but the test version adds velocity and acceleration checks for more robust detection. The input handling also differs significantly—the test firmware reads CSV data from Serial, which is a departure from how the current version operates.

For smoothing, the test firmware applies an exponential moving average filter to the raw acceleration values, whereas the Python tester only smooths altitude and acceleration individually. There's also a key architectural difference: the test firmware computes vertical velocity from the barometric altitude data directly, the plan proposes moving that calculation into the sensor class itself, and the current firmware doesn't capture velocity at all. Finally, the test firmware includes an automatic reset mechanism that triggers when the timestamp jumps backward, allowing it to handle multiple sequential test runs without manual intervention.

The parachute deployment strategy differs significantly across implementations—the current firmware deploys as soon as it detects descent past apogee and drops below 10 meters, the test version waits for 100 meters during descent, and the plan version triggers at freefall detection with higher priority. The plan also introduces infrastructure like task management and sensor interfaces that don't exist in the simpler test and current versions. The test firmware validates sensor data for NaN values before processing...

Looking at the accelerometer smoothing, the test firmware applies the exponential moving average to each axis individually before computing the total acceleration, whereas the Python tester smooths altitude and axes separately. I should organize this comparison more clearly to see the full picture of differences.

Agora tenho informações suficientes. Aqui está a análise comparativa:

---

## Principais Diferenças: `/test/FSM/FSM.ino` vs `/extras/FSM_tester` vs `/firmware` (Planejamento)

### 1. Número de Estados da FSM

| Fonte | Estados | Detalhe |
|-------|---------|---------|
| **test/FSM/FSM.ino** ✅ | **4** | `IDLE → ASCENT → DESCENT → LANDED` |
| FSM_Tester.py | 5 flags | Eventos sequenciais (sem enum formal) |
| REFACTORING_PLAN.md | **7** | `IDLE → LIFTOFF → BURNOUT → APOGEE → FREEFALL → PARACHUTE → LANDED` |
| firmware.ino atual | 0 | Sem FSM — lógica ad-hoc em `handleParachute()` |

O FSM aprovado em `/test` **colapsa toda a fase motorizada em `ASCENT`** e toda a descida em `DESCENT`, diferentemente do planejamento que separa `LIFTOFF`, `BURNOUT`, `APOGEE`, `FREEFALL` e `PARACHUTE` como estados distintos.

---

### 2. Gatilho de Paraquedas

| Fonte | Condição |
|-------|----------|
| **test/FSM** | `altitude ≤ 100m AND vz < 0` (dentro de `DESCENT`) |
| FSM_Tester.py | Mesmo, mas **comentado** (parachute desativado no teste) |
| Plan/Refaactoring | `altitude ≤ 100m AND vz < 0` (estado `PARACHUTE` dedicado) |
| **firmware atual** | `altitude ≤ max_alt - 10m` AND `(altitude < 750m OR |vz| > 80m/s)` |

**Diferença crítica:** O firmware atual usa `ALTITUDE_DROP_THRESHOLD=10m` relativo ao apogeu + `ALTITUDE_THRESHOLD=750m` + `VELOCITY_THRESHOLD=80m/s`. O FSM aprovado e o plano usam um limiar fixo de **100m absolutos** — uma simplificação significativa.

---

### 3. Detecção de Apogeu

| Fonte | Método |
|-------|--------|
| **test/FSM** | `|vz| < 1.0 AND az < -0.1` (baseado em aceleração) |
| FSM_Tester.py | Mesmo: `abs(vz) < 1.0 and az < -0.1` |
| REFACTORING_PLAN | Mesmo, traduzido para C++ |
| **firmware atual** | `altitude ≤ max_altitude - 10m` (queda relativa) |

O FSM aprovado detecta apogeu por **velocidade vertical ~zero + aceleração negativa**, enquanto o firmware atual usa uma **queda absoluta de altitude** do pico. O método do FSM é mais robusto e preciso.

---

### 4. Detecção de Freefall (Queda Livre)

| Fonte | Condição | Guardas |
|-------|----------|---------|
| **test/FSM** | `total_acc < 11.5 AND vz < -5 AND height > 5` | `vz >= -5` impede |
| FSM_Tester.py | `total_acc < 11.5 AND vz < -5 AND height > 5` | `vz >= -0.2` impede |
| REFACTORING_PLAN | Mesmo que Python (referência linha-por-linha) | |
| **firmware atual** | **Não existe** | |

O FSM aprovado tem o guarda de velocidade em `-5.0` (mesmo que o threshold), enquanto o Python original usa `-0.2` como guarda separada — mais restritivo. O FSM aprovado é **menos restritivo** na guarda de entrada.

---

### 5. Lógica de Burnout (Fim de Motor)

| Fonte | Condição |
|-------|----------|
| **test/FSM** | `az < -8 OR total_acc < 2`, com guarda `height > 5 AND vz > 0.5` |
| FSM_Tester.py | Idêntico |
| REFACTORING_PLAN | Idêntico + **debounce de 3 leituras consecutivas** |
| **firmware atual** | Não detecta burnout |

O plano adiciona um mecanismo de **debounce** (`liftoffCounter++`) que o FSM aprovado **não tem** — isso é uma fragilidade, pois uma leitura espúria pode causar falso positivo.

---

### 6. Filtro de Suavização

| Fonte | Aplicação |
|-------|-----------|
| **test/FSM** | Exponencial (α=0.2) em ax, ay, az **individualmente** antes de calcular `total_acc` |
| FSM_Tester.py | Exponencial (α=0.2) em **altp, ax, ay, az** (todos suavizados) |
| **firmware atual** | Sem filtro |

Diferença: o Python suaviza a **altitude também** (`smooth(height, ...)`), o FSM aprovado em C++ **não suaviza altitude** — apenas acelerações.

---

### 7. Cálculo de Velocidade Vertical

| Fonte | Método |
|-------|---------|
| **test/FSM** | `vz = (altp - prev_altp) / dt`, clipping `[-200, 200]` |
| FSM_Tester.py | Mesmo |
| REFACTORING_PLAN | Mesmo, mas implementado **dentro da classe BMP585Sensor** |
| **firmware atual** | Cálculo similar no `loop()` principal, sem clipping |

O plano encapsula o cálculo em `BMP585Sensor::update()`, o FSM aprovado mantém no `process_sensor_data()`, e o firmware atual tem no `loop()` global.

---

### 8. Validação de Segurança (NaN/Inf)

| Fonte | Validação |
|-------|-----------|
| **test/FSM** | `isnan()` nos 5 campos de entrada |
| FSM_Tester.py | `np.isfinite()` em **todas** funções de detecção |
| REFACTORING_PLAN | `std::isfinite()` obrigatório em **cada função** |
| **firmware atual** | **Nenhuma** |

O FSM aprovado valida apenas na **entrada**, enquanto o Python e o plano validam em **cada função de detecção** — o que é mais seguro contra NaN propagados.

---

### 9. Arquitetura Geral

| Aspecto | test/FSM | firmware atual | REFACTORING_PLAN |
|---------|----------|----------------|------------------|
| Tarefas | single loop | single loop | **3 FreeRTOS tasks** |
| Sensores | Serial input | procedimental | **POO (ISensor)** |
| Freq. FSM | async (Serial) | 5Hz | **50Hz** |
| Watchdog | não | não | **sim (5s)** |
| Parachute | flag simples | servo + verificação | **Task dedicada** |
| Reset automático | sim (timestamp) | não | não mencionado |

---

### Resumo das Diferenças Mais Críticas

1. **4 estados vs 7 estados** — o FSM aprovado é mais simples, mas perde granularidade para `BURNOUT`, `APOGEE`, `FREEFALL` e `PARACHUTE` como estados separados
2. **Sem debounce no liftoff** — o plano inclui confirmação de 3 leituras, o FSM aprovado não
3. **Parachute thresholds completamente diferentes** — firmware atual usa queda relativa (10m/750m/80m/s), FSM e plano usam 100m absolutos
4. **Filtro de altitude ausente** — o Python suaviza altitude, o FSM aprovado não
5. **Validação de segurança na entrada vs por detecção** — o FSM aprovado é mais frágil que o que o plano propõe
6. **Reset automático** — presente no FSM aprovado, ausente no plano (útil para testes iterativos)

---

## User

quero agora trabalhar nas próximas tarefas, quais seriam?

---

## Assistant (Plan · mimo-v2-pro-free · 67.5s)

_Thinking:_

The user is asking about the next tasks to work on. Based on the analysis I just did, I need to help them plan the next steps. Let me think about what the logical next steps would be given:

1. They have a working FSM in `/test/FSM/FSM.ino` (4 states, validated)
2. They have a Python FSM tester in `/extras/FSM_tester/`
3. They have a detailed refactoring plan in `/firmware/REFACTORING_PLAN.md` (9 phases)
4. The current firmware in `/firmware/` is procedural, no FSM, old sensors

The REFACTORING_PLAN.md says all 9 phases are "⏳ Pendente" (Pending).


Before jumping into implementation, I should clarify what direction they want to take. I'm weighing a few paths: integrating the validated FSM into the current firmware for immediate value, starting the full refactoring plan from phase 1, or consolidating the FSM approaches first since there are differences between the test version, the plan, and the current code. I need to ask some clarifying questions to understand their priorities.

Antes de montar o plano, preciso alinhar algumas decisões com você:

**Tool: question**

**Input:**
```json
{
  "questions": [
    {
      "question": "Qual abordagem você quer seguir agora?",
      "header": "Próximo passo",
      "options": [
        {
          "label": "Integrar FSM no firmware atual (Recomendado)",
          "description": "Adaptar o FSM aprovado (4 estados) para o firmware existente, substituindo a lógica ad-hoc de paraquedas. Ganho rápido, validado."
        },
        {
          "label": "Iniciar refatoração completa (9 fases)",
          "description": "Começar pelas Fases 1-2 do REFACTORING_PLAN.md (estrutura de pastas, ISensor interface, structs). Mudança maior."
        },
        {
          "label": "Reconciliar FSM antes de integrar",
          "description": "Primeiro unificar as 3 versões (test/, Python, plano) em uma FSM definitiva (estados, thresholds, validações) e depois integrar."
        },
        {
          "label": "Outro caminho",
          "description": "Descreva uma abordagem diferente"
        }
      ],
      "multiple": false
    },
    {
      "question": "O FSM aprovado tem 4 estados. O planejamento propõe 7. Qual usar na integração?",
      "header": "Número de estados",
      "options": [
        {
          "label": "4 estados (test/FSM)",
          "description": "IDLE → ASCENT → DESCENT → LANDED. Mais simples, já validado."
        },
        {
          "label": "7 estados (plano)",
          "description": "IDLE → LIFTOFF → BURNOUT → APOGEE → FREEFALL → PARACHUTE → LANDED. Mais granular, reflete o plano de refatoração."
        },
        {
          "label": "5-6 estados (híbrido)",
          "description": "Manter detecções de burnout/freefall como eventos internos, mas só transitar nos estados críticos."
        }
      ],
      "multiple": false
    },
    {
      "question": "Os sensores novos (BMP585, LSM6DS3) já estão disponíveis no hardware?",
      "header": "Hardware disponível",
      "options": [
        {
          "label": "Sim, sensores novos prontos",
          "description": "Posso usar as classes POO do plano (BMP585Sensor, LSM6DS3Sensor)"
        },
        {
          "label": "Não, ainda usando BMP280 + MPU6050",
          "description": "Integrar FSM com os sensores atuais (código procedural existente)"
        },
        {
          "label": "Parcialmente",
          "description": "Alguns sensores novos, outros antigos"
        }
      ],
      "multiple": false
    }
  ]
}
```

**Output:**
```
User has answered your questions: "Qual abordagem você quer seguir agora?"="Iniciar refatoração completa (9 fases)", "O FSM aprovado tem 4 estados. O planejamento propõe 7. Qual usar na integração?"="4 estados (test/FSM)", "Os sensores novos (BMP585, LSM6DS3) já estão disponíveis no hardware?"="Não, ainda usando BMP280 + MPU6050". You can now continue with the user's answers in mind.
```

---

## Assistant (Plan · mimo-v2-pro-free · 65.8s)

_Thinking:_

The user wants to:
1. Start the full refactoring (9 phases)
2. Use 4 states (from test/FSM) instead of 7
3. Still using old sensors (BMP280 + MPU6050)

This changes things from the REFACTORING_PLAN.md. I need to adapt the plan:

1. The refactoring plan was written for BMP585 + LSM6DS3 (new sensors). Since the user still has old sensors, the sensor classes should wrap the existing BMP280 and MPU6050 libraries instead.

2. The FSM should use 4 states (IDLE, ASCENT, DESCENT, LANDED) from the approved test/FSM, not the 7-state version from the plan.


3. The 9-phase plan still works structurally, but I need to adjust Phases 3 and 4 to wrap the existing sensor libraries with the ISensor interface rather than creating entirely new sensor classes, which is actually simpler. I should also confirm whether I'm adapting the FSM to 4 states or if there's flexibility there.

Looking at the REFACTORING_PLAN.md, there's a contradiction—the plan documents a 7-state FSM and explicitly mentions deploying the parachute at APOGEE, but the user wants 4 states. I need to figure out where the parachute deployment logic fits with the simplified FSM.

Let me think through this: if we're collapsing to IDLE, ASCENT, DESCENT, and LANDED, then apogee becomes just a transition point from ASCENT to DESCENT rather than its own state. The parachute could deploy either during that transition or as a sub-event within DESCENT. The real question is whether the user's preference for 4 states is about simplifying the state machine structure while keeping the detection logic intact, or if they want to genuinely reduce complexity by merging detection events into broader categories. I should clarify this before mapping out the full plan.

Rather than asking more questions, I should present a concrete refactored plan based on what they've told me: start with the full 9-phase refactoring, keep 4 states, and work with the existing sensors. I'll lay out the phases adapted to their constraints and let them approve or adjust from there.

Entendido. Com base nas suas respostas, vou montar o plano adaptado. Preciso de mais uma clarificação:

**Tool: question**

**Input:**
```json
{
  "questions": [
    {
      "header": "Detecções internas",
      "multiple": false,
      "options": [
        {
          "description": "BURNOUT, APOGEE, FREEFALL e PARACHUTE continuam sendo detectados como eventos/logs, mas sem estados próprios na enum. Ex: ao detectar apogeu, transita direto de ASCENT para DESCENT e dispara paraquedas.",
          "label": "Detectar tudo, mas só 4 estados formais (Recomendado)"
        },
        {
          "description": "Descartar detecções de BURNOUT e FREEFALL. Focar apenas em liftoff (ASCENT), apogeu (DESCENT), pouso (LANDED) e paraquedas.",
          "label": "Simplificar detecções também"
        },
        {
          "description": "Manter BURNOUT como evento dentro de ASCENT e FREEFALL/PARACHUTE como eventos dentro de DESCENT, com logs separados.",
          "label": "Eventos como sub-estados lógicos"
        }
      ],
      "question": "Com 4 estados (IDLE, ASCENT, DESCENT, LANDED), o que fazer com as detecções de BURNOUT, APOGEE, FREEFALL e PARACHUTE?"
    }
  ]
}
```

**Output:**
```
User has answered your questions: "Com 4 estados (IDLE, ASCENT, DESCENT, LANDED), o que fazer com as detecções de BURNOUT, APOGEE, FREEFALL e PARACHUTE?"="Eventos como sub-estados lógicos". You can now continue with the user's answers in mind.
```

---

## Assistant (Plan · mimo-v2-pro-free · 150.0s)

_Thinking:_

The user wants:
1. Full refactoring (9 phases)2. 4 states (IDLE, ASCENT, DESCENT, LANDED)
3. Old sensors (BMP280, MPU6050)
4. BURNOUT, APOGEE, FREEFALL, PARACHUTE as logical sub-events within states (with separate logs)

Now let me construct the adapted plan. I need to update the REFACTORING_PLAN.md to reflect:

1. **Sensor classes**: Wrap BMP280 and MPU6050 (not BMP585/LSM6DS3)2. 

I'll simplify the FSM to 4 states with sub-events tracked as flags and logs, use FreeRTOS tasks and OOP structure as planned, and adapt the existing parachute_module logic into the FSM while adding NaN/Inf validation. Now I need to review the current codebase structure to create a concrete, phased plan.

**Tool: task**

**Input:**
```json
{
  "description": "Analyze current firmware code",
  "prompt": "I need to understand the current firmware codebase at /home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/ to plan a refactoring. Read and analyze:\n\n1. All .h files in /firmware/ - note what global variables exist, what functions are defined, and dependencies between modules\n2. Focus on:\n   - How bmp280_sensor.h works (global vars, functions, library used)\n   - How mpu6050_sensor.h works (global vars, functions, library used)\n   - How telemetry_module.h aggregates data\n   - What globals are shared across modules (especially: previous_altitude, max_altitude, base_pressure, parachute_deployed, previous_millis, file_dir, BMP object)\n3. List ALL global variables that are shared between modules and where they're defined\n\nReturn a detailed report of all modules, their functions, their global variables, and inter-module dependencies. This is critical for planning the refactoring.",
  "subagent_type": "explore"
}
```

**Output:**
```
task_id: ses_2cfb56452ffeJQuVMpGCQjp8Pe (for resuming to continue this task if needed)

<task_result>
Now I have a complete picture. Let me compile the comprehensive report.

---

# Firmware Codebase Analysis Report

**Target:** `/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/`
**Platform:** ESP32 (Arduino framework)
**Total files:** 1 `.ino` + 10 `.h` files (all in flat directory structure)

---

## 1. FILE INVENTORY

| File | Lines | Role |
|------|-------|------|
| `config.h` | 182 | Pin definitions, constants, thresholds, WiFi credentials |
| `bmp280_sensor.h` | 166 | BMP280 barometric pressure/altitude sensor |
| `mpu6050_sensor.h` | 143 | MPU6050 IMU (accelerometer + gyroscope) |
| `gps_module.h` | 223 | GPS receiver (TinyGPS++) |
| `lora_module.h` | 113 | LoRa long-range radio transmission |
| `filesystem_module.h` | 143 | LittleFS flash filesystem for data logging |
| `parachute_module.h` | 170 | Servo-controlled parachute deployment |
| `buzzer_module.h` | 86 | Audio feedback via piezo buzzer |
| `server_module.h` | 242 | WiFi AP + async web server for data download |
| `telemetry_module.h` | 192 | Data aggregation, logging, and dual-channel TX |
| `firmware.ino` | 241 | Main entry point (setup + loop) |
| `flight/` | 0 entries | Empty directory (planned refactoring target) |
| `sensors/` | 0 entries | Empty directory (planned refactoring target) |

---

## 2. MODULE-BY-MODULE ANALYSIS

### 2.1. `config.h` -- Configuration Hub

**Includes:** `<Arduino.h>`

**Preprocessor Defines:**
| Symbol | Value | Purpose |
|--------|-------|---------|
| `INTERVAL` | `200` | Main loop period in ms (~5Hz) |
| `LORA_FREQ` | `868E6` | LoRa frequency (868 MHz Europe) |
| `SS_LORA` | `7` | LoRa SPI chip select pin |
| `RST_LORA` | `1` | LoRa reset pin |
| `DIO0_LORA` | `2` | LoRa interrupt pin |
| `SYNC_WORD` | `0xF3` | LoRa network synchronization word |
| `SERVO_PIN` | `3` | Parachute servo PWM pin |
| `BUZZER_PIN` | `0` | Buzzer digital pin |
| `RX_GPS` | `20` | GPS UART RX pin |
| `TX_GPS` | `21` | GPS UART TX pin |

**Constants:**
| Name | Type | Value | Purpose |
|------|------|-------|---------|
| `MAXPOS` | `const int` | `0` | Servo open position (parachute release) |
| `MINPOS` | `const int` | `90` | Servo closed position (locked) |
| `ALTITUDE_DROP_THRESHOLD` | `const float` | `10.0` | Meters below apogee to confirm descent |
| `ALTITUDE_THRESHOLD` | `const float` | `750.0` | Minimum altitude for deploy (m) |
| `VELOCITY_THRESHOLD` | `const float` | `80.0` | Min descent velocity for deploy (m/s) |
| `TEAM_ID` | `const String` | `"#100"` | Team identifier for telemetry |

**Mutable Global Variables (defined here, used elsewhere):**
| Name | Type | Initial Value | Used By |
|------|------|---------------|---------|
| `file_name` | `String` | `"Dados.csv"` | `firmware.ino` |
| **`file_dir`** | `String` | `""` | `firmware.ino`, `telemetry_module.h` |
| `ssid_str` | `String` | `"Server #100"` | `server_module.h` |
| `ssid` | `const char*` | (from ssid_str) | `server_module.h` |
| `password` | `const char*` | `"Iamarobot"` | `server_module.h` |

**CRITICAL ISSUE:** `file_name`, `file_dir`, and `ssid_str` are mutable globals defined in a header. If `config.h` were included in multiple translation units, this would cause linker errors. Currently works only because Arduino IDE compiles everything as one unit.

---

### 2.2. `bmp280_sensor.h` -- Barometric Sensor

**Library Used:** `Adafruit_BMP280` (via `#include <Adafruit_BMP280.h>`)

**Global Variables Defined:**
| Name | Type | Initial Value | Mutated By | Read By |
|------|------|---------------|------------|---------|
| **`BMP`** | `Adafruit_BMP280` | (default constructor) | -- | `setupBMP()`, `BMPData()`, `firmware.ino` loop |
| **`previous_altitude`** | `float` | `0` | `setupBMP()`, `parachute_module.h` (`handleParachute()`) | `firmware.ino` loop (velocity calc) |
| **`max_altitude`** | `float` | `0` | `setupBMP()`, `checkHighest()` | `parachute_module.h` (`handleParachute()`) |
| **`base_altitude`** | `float` | `0` | `setupBMP()` | (unused outside init) |
| **`base_pressure`** | `float` | `0` | `setupBMP()` | `BMPData()`, `firmware.ino` loop |

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupBMP()` | `bool setupBMP()` | Initializes BMP280 via I2C, reads `base_pressure`, calibrates `previous_altitude`, `max_altitude`, `base_altitude` |
| `BMPData()` | `String BMPData()` | Returns CSV: `"altitude,temperature,nan,pressure"` using `base_pressure` as reference |
| `checkHighest()` | `void checkHighest(float altitude)` | Updates `max_altitude` if current altitude exceeds it |

**Dependencies:**
- **Writes to:** `previous_altitude`, `max_altitude`, `base_altitude`, `base_pressure`
- **Reads from:** `BMP` object, `base_pressure`
- **Used by:** `telemetry_module.h` (calls `BMPData()`), `parachute_module.h` (reads `max_altitude`, writes `previous_altitude`), `firmware.ino` (reads `BMP`, `base_pressure`, `previous_altitude`, calls `checkHighest()`)

---

### 2.3. `mpu6050_sensor.h` -- Inertial Measurement Unit

**Library Used:** `Adafruit_MPU6050` (via `#include <Adafruit_MPU6050.h>`, `#include <Adafruit_Sensors.h>`)

**Global Variables Defined:**
| Name | Type | Initial Value | Mutated By | Read By |
|------|------|---------------|------------|---------|
| `MPU` | `Adafruit_MPU6050` | (default constructor) | -- | `setupMPU()`, `MPUData()` |
| `acc` | `sensors_event_t` | (default) | `MPUData()` | `MPUData()` (local extraction) |
| `gyr` | `sensors_event_t` | (default) | `MPUData()` | `MPUData()` (local extraction) |
| `temp` | `sensors_event_t` | (default) | `MPUData()` | (written but never read) |

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupMPU()` | `bool setupMPU()` | Initializes MPU6050 via I2C |
| `MPUData()` | `String MPUData()` | Calls `MPU.getEvent()` for all 3 axes, returns CSV: `"gp,gr,gy,ap,ar,ay"` |

**Dependencies:**
- **Writes to:** `acc`, `gyr`, `temp` (event buffers)
- **Used by:** `telemetry_module.h` (calls `MPUData()`)
- **Self-contained:** No cross-module global sharing (unlike BMP280)

---

### 2.4. `gps_module.h` -- GPS Positioning

**Library Used:** `TinyGPS++` (via `#include <TinyGPS++.h>`)

**Global Variables Defined:**
| Name | Type | Initial Value | Notes |
|------|------|---------------|-------|
| `GPS` | `TinyGPSPlus` | (default constructor) | Only used within this module |

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupGPS()` | `void setupGPS()` | Starts Serial1 at 9600 baud on RX/TX pins, feeds GPS data for 3 seconds |
| `getGPSTimeString()` | `String getGPSTimeString()` | Returns `"HH_MM_SS"` or `"init_<millis>"` fallback; used for filename generation |
| `GPSData()` | `String GPSData()` | Processes NMEA, returns CSV: `"time,date,alt,lat,lon,satellites"` |

**Dependencies:**
- **Reads from:** `config.h` (`RX_GPS`, `TX_GPS`)
- **Used by:** `firmware.ino` (calls `getGPSTimeString()`), `telemetry_module.h` (calls `GPSData()`)

---

### 2.5. `lora_module.h` -- LoRa Radio

**Library Used:** `LoRa` (via `#include <LoRa.h>`)

**Global Variables Defined:** None (uses the LoRa library singleton internally).

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupLoRa()` | `bool setupLoRa()` | Sets SPI pins, initializes LoRa at `LORA_FREQ`, sets `SYNC_WORD` |
| `sendLoRa()` | `void sendLoRa(const String &message)` | Transmits a string packet via LoRa radio |

**Dependencies:**
- **Reads from:** `config.h` (`SS_LORA`, `RST_LORA`, `DIO0_LORA`, `LORA_FREQ`, `SYNC_WORD`)
- **Used by:** `telemetry_module.h` (called from `printBoth()`)

---

### 2.6. `filesystem_module.h` -- LittleFS Storage

**Library Used:** `LittleFS` (via `#include "LittleFS.h"`)

**Global Variables Defined:** None.

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupLittleFS()` | `bool setupLittleFS()` | Mounts LittleFS with auto-format |
| `writeFile()` | `bool writeFile(const String &path, const String &data_string)` | Creates/overwrites file (used for CSV header) |
| `appendFile()` | `void appendFile(const String &path, const String &message)` | Appends line to file (used for telemetry rows) |

**Dependencies:**
- **Used by:** `firmware.ino` (calls `setupLittleFS()`, `writeFile()`), `telemetry_module.h` (calls `appendFile()`)

---

### 2.7. `parachute_module.h` -- Parachute Deployment

**Library Used:** `ESP32Servo` (via `#include <ESP32Servo.h>`)

**Global Variables Defined:**
| Name | Type | Initial Value | Notes |
|------|------|---------------|-------|
| `ParachuteServo` | `Servo` | (default) | PWM servo control |
| **`parachute_deployed`** | `bool` | `false` | **SHARED** -- read by `firmware.ino` loop and `logData()`, written here |

**Forward Declarations:**
```cpp
void printBoth(const String &message);  // Defined in telemetry_module.h
```

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setupServo()` | `void setupServo()` | Attaches servo to `SERVO_PIN`, moves to `MINPOS` (closed) |
| `handleParachute()` | `void handleParachute(float altitude, float velocity)` | Multi-condition deploy logic, updates `previous_altitude` |

**CRITICAL CROSS-MODULE WRITE:** `handleParachute()` directly mutates `previous_altitude` (defined in `bmp280_sensor.h`) at line 167:
```cpp
previous_altitude = altitude;
```

**Dependencies:**
- **Reads from:** `config.h` (`SERVO_PIN`, `MAXPOS`, `MINPOS`, `ALTITUDE_DROP_THRESHOLD`, `ALTITUDE_THRESHOLD`, `VELOCITY_THRESHOLD`), `bmp280_sensor.h` (`max_altitude`)
- **Writes to:** `bmp280_sensor.h` (`previous_altitude`), `parachute_deployed` (local)
- **Calls:** `printBoth()` from `telemetry_module.h`
- **Used by:** `firmware.ino` (calls `setupServo()`, `handleParachute()`)

---

### 2.8. `buzzer_module.h` -- Audio Feedback

**Library Used:** Arduino core (`tone()` function)

**Global Variables Defined:** None.

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `buzzSignal()` | `void buzzSignal(String signal)` | Generates audio patterns: "Alert" (5 beeps), "Success" (3 beeps), "Activated" (long beep), "Beep" (short) |

**Dependencies:**
- **Reads from:** `config.h` (`BUZZER_PIN`)
- **Used by:** `firmware.ino` (calls `buzzSignal()`), `telemetry_module.h` (calls `buzzSignal("Beep")` in `printBoth()`)

---

### 2.9. `server_module.h` -- WiFi Web Server

**Libraries Used:** `WiFi.h`, `ESPAsyncWebServer.h`, `ArduinoJson.h`, `LittleFS.h`

**Global Variables Defined:**
| Name | Type | Notes |
|------|------|-------|
| `server` | `AsyncWebServer` | Port 80, used only in this module |

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `setServerRoutes()` | `void setServerRoutes()` | Configures routes: `/`, `/style.css`, `/index.js`, `GET /api/files`, `GET /api/file`, `DELETE /api/file` |
| `setupServer()` | `void setupServer()` | Creates WiFi AP with `ssid`/`password`, starts server |

**Dependencies:**
- **Reads from:** `config.h` (`ssid`, `password`)
- **Used by:** `firmware.ino` (calls `setupServer()`)

---

### 2.10. `telemetry_module.h` -- Data Aggregation Hub

**This is the central orchestrator module.**

**Libraries/Modules Included:**
```cpp
#include "config.h"
#include "bmp280_sensor.h"
#include "mpu6050_sensor.h"
#include "gps_module.h"
#include "lora_module.h"
#include "filesystem_module.h"
#include "buzzer_module.h"
```

**Global Variables Defined:**
| Name | Type | Initial Value | Notes |
|------|------|---------------|-------|
| `packet_count` | `int` | `0` | Monotonically increasing packet counter |
| **`previous_millis`** | `unsigned long` | `0` | **SHARED** -- read/written by `firmware.ino` loop for timing |

**Functions Defined:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `getDataString()` | `String getDataString()` | Aggregates: `BMPData() + "," + MPUData() + "," + GPSData()` |
| `printBoth()` | `void printBoth(const String &message)` | Sends to `Serial.println()`, `sendLoRa()`, `buzzSignal("Beep")` |
| `logData()` | `void logData(unsigned long current_millis, bool parachute_deployed)` | Assembles full CSV, calls `printBoth()`, `appendFile(file_dir, data_string)`, increments `packet_count` |

**Dependencies:**
- **Reads from:** `config.h` (`TEAM_ID`, `file_dir`), all sensor modules
- **Writes to:** `packet_count` (local), `file_dir` indirectly via `appendFile()`
- **Calls:** `BMPData()`, `MPUData()`, `GPSData()`, `sendLoRa()`, `buzzSignal()`, `appendFile()`

---

### 2.11. `firmware.ino` -- Main Entry Point

**Includes (explicit buses):**
```cpp
#include <Wire.h>    // I2C for BMP280 + MPU6050
#include <SPI.h>     // SPI for LoRa
```

**Include order:**
1. `config.h`
2. `bmp280_sensor.h`
3. `mpu6050_sensor.h`
4. `gps_module.h`
5. `lora_module.h`
6. `filesystem_module.h`
7. `parachute_module.h`
8. `buzzer_module.h`
9. `server_module.h`
10. `telemetry_module.h`

**`setup()` call sequence:**
1. `Serial.begin(115200)`
2. `Wire.begin()`
3. `setupServo()`
4. 5-second delay loop with status messages
5. `setupGPS()`
6. `getGPSTimeString()` -> constructs `file_dir`
7. `setupLittleFS()` + `writeFile(file_dir, header)` (restarts on failure)
8. `setupServer()`
9. `setupBMP()` + `setupMPU()` + `setupLoRa()` (non-fatal if fails)

**`loop()` logic (runs every 200ms):**
```cpp
float altitude = BMP.readAltitude(base_pressure);         // Reads BMP global + base_pressure global
float velocity = (altitude - previous_altitude) / dt;     // Reads previous_altitude global
logData(current_millis, parachute_deployed);               // Reads parachute_deployed global
checkHighest(altitude);                                    // Writes max_altitude global
handleParachute(altitude, velocity);                       // Reads max_altitude, writes previous_altitude + parachute_deployed
previous_millis = current_millis;                          // Writes previous_millis global
```

---

## 3. COMPLETE GLOBAL VARIABLE INVENTORY

### 3.1. Shared Global Variables (cross-module coupling)

These are the variables that create tight coupling between modules. Each one is defined in one header but mutated or read in another.

| Variable | Defined In | Type | Initial | Mutated By | Read By | Coupling Severity |
|----------|-----------|------|---------|------------|---------|-------------------|
| **`previous_altitude`** | `bmp280_sensor.h:38` | `float` | `0` | `setupBMP()` (bmp280), `handleParachute()` (**parachute_module**) | `firmware.ino` loop (velocity calc) | **HIGH** -- written in 2 modules |
| **`max_altitude`** | `bmp280_sensor.h:45` | `float` | `0` | `setupBMP()` (bmp280), `checkHighest()` (bmp280) | `handleParachute()` (**parachute_module**) | **HIGH** -- read across module boundary |
| **`base_pressure`** | `bmp280_sensor.h:58` | `float` | `0` | `setupBMP()` (bmp280) | `BMPData()` (bmp280), `firmware.ino` loop | MEDIUM -- read in 2 places |
| **`BMP`** | `bmp280_sensor.h:32` | `Adafruit_BMP280` | (default) | -- | `setupBMP()`, `BMPData()`, `firmware.ino` loop | **HIGH** -- raw sensor access in main loop |
| **`parachute_deployed`** | `parachute_module.h:50` | `bool` | `false` | `handleParachute()` (parachute_module) | `firmware.ino` loop, `logData()` (**telemetry_module**) | **HIGH** -- read across 2 modules |
| **`previous_millis`** | `telemetry_module.h:57` | `unsigned long` | `0` | `firmware.ino` loop | `firmware.ino` loop (timing check) | **HIGH** -- defined in telemetry, used in main loop |
| **`file_dir`** | `config.h:159` | `String` | `""` | `firmware.ino` setup | `logData()` (**telemetry_module**) | **HIGH** -- filesystem path shared |
| **`file_name`** | `config.h:152` | `String` | `"Dados.csv"` | -- | `firmware.ino` setup | LOW |

### 3.2. Module-Local Global Variables

These are used only within the module that defines them.

| Variable | Defined In | Type | Notes |
|----------|-----------|------|-------|
| `base_altitude` | `bmp280_sensor.h:51` | `float` | Set once in `setupBMP()`, never read externally |
| `MPU` | `mpu6050_sensor.h:40` | `Adafruit_MPU6050` | IMU object, self-contained |
| `acc` | `mpu6050_sensor.h:49` | `sensors_event_t` | Accelerometer event buffer |
| `gyr` | `mpu6050_sensor.h:58` | `sensors_event_t` | Gyroscope event buffer |
| `temp` | `mpu6050_sensor.h:65` | `sensors_event_t` | Temperature event buffer (written but never read) |
| `GPS` | `gps_module.h:40` | `TinyGPSPlus` | GPS parser, self-contained |
| `ParachuteServo` | `parachute_module.h:42` | `Servo` | Servo motor object |
| `packet_count` | `telemetry_module.h:49` | `int` | Packet counter, only used in `logData()` |
| `server` | `server_module.h:44` | `AsyncWebServer` | Web server on port 80 |

### 3.3. Config Globals (defined in `config.h`)

| Variable | Type | Value | Read By |
|----------|------|-------|---------|
| `file_name` | `String` | `"Dados.csv"` | `firmware.ino` |
| `file_dir` | `String` | `""` (populated in setup) | `firmware.ino`, `telemetry_module.h` |
| `ssid_str` | `String` | `"Server #100"` | `config.h` (for `ssid`) |
| `ssid` | `const char*` | (from ssid_str) | `server_module.h` |
| `password` | `const char*` | `"Iamarobot"` | `server_module.h` |

---

## 4. INTER-MODULE DEPENDENCY MAP

```
firmware.ino
  |
  +---> config.h          (reads: INTERVAL, TEAM_ID, file_dir, file_name, BUZZER_PIN)
  +---> bmp280_sensor.h   (reads: BMP, base_pressure, previous_altitude; calls: checkHighest())
  +---> mpu6050_sensor.h  (no direct interaction in loop)
  +---> gps_module.h      (calls: getGPSTimeString() in setup)
  +---> lora_module.h     (indirect via telemetry)
  +---> filesystem_module.h (calls: setupLittleFS(), writeFile() in setup)
  +---> parachute_module.h (calls: setupServo(), handleParachute(); reads: parachute_deployed)
  +---> buzzer_module.h   (calls: buzzSignal() in setup)
  +---> server_module.h   (calls: setupServer() in setup)
  +---> telemetry_module.h (calls: logData())

telemetry_module.h  [CENTRAL HUB]
  |
  +---> bmp280_sensor.h   (calls: BMPData())
  +---> mpu6050_sensor.h  (calls: MPUData())
  +---> gps_module.h      (calls: GPSData())
  +---> lora_module.h     (calls: sendLoRa())
  +---> filesystem_module.h (calls: appendFile())
  +---> buzzer_module.h   (calls: buzzSignal())
  +---> config.h          (reads: TEAM_ID, file_dir)

parachute_module.h  [CROSS-BOUNDARY WRITES]
  |
  +---> bmp280_sensor.h   (READS: max_altitude; WRITES: previous_altitude)
  +---> config.h          (reads: SERVO_PIN, MAXPOS, MINPOS, thresholds)
  +---> telemetry_module.h (calls: printBoth() via forward declaration)
```

---

## 5. KEY PROBLEMS IDENTIFIED FOR REFACTORING

### 5.1. Header-Only Architecture with Mutable Globals
All code is in `.h` files with mutable global variable definitions. This is a single-translation-unit hack that will break if headers are included from multiple files. Every `.h` defines variables **and** functions, violating the one-definition rule.

### 5.2. Cross-Module Global Mutation (Highest Severity)
The worst coupling violations are:

1. **`previous_altitude`** (defined in `bmp280_sensor.h`) is written by `parachute_module.h:handleParachute()` at line 167. This means parachute logic silently modifies sensor state.

2. **`max_altitude`** (defined in `bmp280_sensor.h`) is read by `parachute_module.h:handleParachute()` at line 132. Deployment logic depends on sensor module's internal tracking.

3. **`BMP`** object (defined in `bmp280_sensor.h`) is read directly in `firmware.ino:loop()` line 212: `BMP.readAltitude(base_pressure)`. The main loop bypasses the module's API to read the raw sensor.

4. **`previous_millis`** (defined in `telemetry_module.h`) is used for timing control in `firmware.ino:loop()` -- a timing variable has no business being in the telemetry module.

5. **`parachute_deployed`** (defined in `parachute_module.h`) is read by `firmware.ino:loop()` and passed as a parameter to `logData()` in `telemetry_module.h`.

### 5.3. Velocity Calculation in Main Loop
The velocity calculation `(altitude - previous_altitude) / dt` is done in `firmware.ino:loop()` using globals from `bmp280_sensor.h` and timing from `telemetry_module.h`. This couples the main loop to two other modules' internal state.

### 5.4. Forward Declaration Circular Dependency
`parachute_module.h` forward-declares `printBoth()` which is defined in `telemetry_module.h`. But `telemetry_module.h` includes `parachute_module.h` (transitively via `bmp280_sensor.h`). The include order in `firmware.ino` places `parachute_module.h` before `telemetry_module.h` to make this work, but it is fragile.

### 5.5. No Sensor Abstraction
`BMP` and `MPU` are used directly. `firmware.ino` calls `BMP.readAltitude()` directly instead of going through `BMPData()`. There is no interface or abstraction that would allow swapping BMP280 for BMP585 or MPU6050 for LSM6DS3 without touching every consumer.

### 5.6. `temp` Event Buffer Wasted
`mpu6050_sensor.h` allocates `sensors_event_t temp` and populates it on every `MPU.getEvent()` call, but it is never read. The temperature data from the MPU6050 is discarded.

### 5.7. Blocking Code
`buzzer_module.h` uses `delay()` calls inside `buzzSignal()`, which blocks the entire system during audio feedback. The `parachute_module.h` also has a 500ms blocking wait loop for servo verification.

---

## 6. REFACTORING PRIORITY MATRIX

| Priority | Item | Current Problem | Target State |
|----------|------|-----------------|--------------|
| **P0** | `previous_altitude` ownership | Written by 2 modules | Own it in a shared `SensorData` struct or FSM |
| **P0** | `max_altitude` ownership | Cross-module read | Same as above |
| **P0** | `parachute_deployed` ownership | Read by 3 modules | Move to FSM state |
| **P1** | `BMP` raw access in loop | Bypasses module API | All access via `BMP585Sensor` class |
| **P1** | `previous_millis` location | In telemetry, used in loop | Move to main loop or timing module |
| **P1** | `file_dir` in config.h | Mutable global in header | Pass as parameter or singleton |
| **P2** | Velocity calculation | In main loop using raw globals | Move to `BMP585Sensor::getVerticalVelocity()` |
| **P2** | Sensor abstraction | Direct Adafruit objects | `ISensor` interface with `begin()`, `update()`, `getData()` |
| **P3** | Blocking buzzer | `delay()` blocks system | Use non-blocking millis()-based timing |
| **P3** | Unused `temp` buffer | Wasted memory in MPU6050 | Remove or use for thermal compensation |
</task_result>
```

---

## Assistant (Plan · mimo-v2-pro-free · 102.3s)

_Thinking:_

Excellent! Now I have a very detailed picture of the codebase. Let me construct the adapted refactoring plan. 

Key decisions from the user:
1. Full 9-phase refactoring
2. 4 states (IDLE, ASCENT, DESCENT, LANDED) with BURNOUT, APOGEE, FREEFALL, PARACHUTE as logical sub-events
3. Old sensors (BMP280, MPU6050) - wrap them in ISensor interface
4. FreeRTOS tasks
5. OOP for sensors

Let me now construct the adapted plan. The main adaptations from the original REFACTORING_PLAN.md:


- Phase 3: BMP280Sensor instead of BMP585Sensor
- Phase 4: MPU6050Sensor instead of LSM6DS3Sensor
- Phase 6: 4-state FSM with sub-events instead of 7-state FSM
- Phase 7-9: Mostly the same but adapted for 4-state FSM

Now I'm laying out the complete adapted plan for the user to approve before starting implementation.

**Phase 1 (30min):** Set up the directory structure with `flight/` for the FSM, `sensors/` for sensor abstractions, and `modules/` for existing modules, then create a branch.

**Phase 2 (45min):** Define the ISensor interface, SensorData struct with FlightState enum and LogMessage struct, and establish the FSM thresholds.

**Phase 3 (2h):** Create the BMP280Sensor class that wraps the existing Adafruit_BMP280 implementation, encapsulating the global state like BMP, previous_altitude, max_altitude, and base_pressure.

**Phase 4 (1.5h):** Build the MPU6050Sensor class as a straightforward wrapper around Adafruit_MPU6050.

**Phase 5 (1h):** Implement the GPSModule class, which is relatively straightforward since it's mostly self-contained.

**Phase 6 (4h):** Build the FlightStateMachine with 4 states and sub-events for liftoff, burnout, apogee, freefall, and parachute deployment, porting the logic from FSM.ino with NaN/Inf validation and guard checks.

**Phase 7 (4h):** Set up three FreeRTOS tasks: FlightControl running at 50Hz, Telemetry at 5Hz, and Logger as needed.

**Phase 8 (2h):** Wire everything together in the main firmware.ino file, initializing all classes and launching the tasks.

**Phase 9 (1.5h):** Refactor the existing modules to remove globals and adapt the parachute logic to work with the FSM structure.

I'm noticing the parachute deployment is a key design decision here. The test FSM triggers it at altitude ≤ 100m during descent, while the current firmware uses a different logic based on altitude drop and velocity thresholds. I need to decide whether to integrate parachute handling directly into the FSM or keep it separate as an event handler.

Looking at the refactoring plan, the FSM should drive the parachute deployment when entering the DESCENT state and altitude conditions are met. I should present the full plan with this architecture decision called out clearly, along with the adapted phases and timeline.

Tenho todas as informações necessárias. Aqui está o plano adaptado:

---

## Plano de Refatoração Adaptado

### Resumo das Decisões

| Aspecto | Original (REFACTORING_PLAN.md) | **Adaptado** |
|---------|-------------------------------|--------------|
| Estados FSM | 7 (`IDLE→LIFTOFF→BURNOUT→APOGEE→FREEFALL→PARACHUTE→LANDED`) | **4** (`IDLE→ASCENT→DESCENT→LANDED`) |
| Sub-eventos | Estados próprios | **Flags + logs** (BURNOUT, APOGEE, FREEFALL, PARACHUTE dentro de ASCENT/DESCENT) |
| Sensores | BMP585 + LSM6DS3 | **BMP280 + MPU6050** (wrappers sobre libs existentes) |
| Deploy paraquedas | Estado `PARACHUTE` dedicado | **Sub-evento em `DESCENT`**: `altitude ≤ 100 AND vz < 0` |
| Validação NaN/Inf | Em cada função de detecção | **Em cada função + na entrada** (como test/FSM) |
| FreeRTOS | 3 tasks (igual) | **3 tasks** (igual) |
| POO sensores | ISensor → BMP585/LSM6DS3 | **ISensor → BMP280Sensor/MPU6050Sensor** |

---

### Fase 1: Setup (30min)

- [ ] Criar branch `feature/oop-freertos-refactor`
- [ ] Criar `flight/` e `sensors/` (já existem vazios)
- [ ] Criar `modules/` e mover: `buzzer_module.h`, `filesystem_module.h`, `lora_module.h`, `parachute_module.h`, `server_module.h`
- [ ] Manter `config.h`, `firmware.ino`, `telemetry_module.h` na raiz por enquanto

### Fase 2: Interface Base + Structs (45min)

Arquivos novos:
- **`sensors/ISensor.h`** — interface abstrata: `begin()`, `update()`, `getData()`, `isReady()`
- **`flight/SensorData.h`** — structs + enum:
  ```cpp
  enum FlightState { IDLE=0, ASCENT=1, DESCENT=2, LANDED=3 };
  
  struct FlightEvents { // flags de sub-eventos
    bool liftoff, burnout, apogee, freefall, parachute_deployed;
  };
  
  struct SensorData { ... };  // timestamp, altitude, velocity, accel, gyro, state, events
  struct LogMessage { ... };  // message, timestamp, taskId, level
  ```
- **`config.h`** — adicionar thresholds da FSM:
  ```cpp
  #define LIFTOFF_TOTAL_ACCEL_THRESHOLD  15.0
  #define BURNOUT_AZ_THRESHOLD          -8.0
  #define BURNOUT_TOTAL_ACCEL_MIN        2.0
  #define BURNOUT_MIN_HEIGHT             5.0
  #define BURNOUT_MIN_VZ                 0.5
  #define APOGEE_VZ_THRESHOLD            1.0
  #define APOGEE_AZ_THRESHOLD           -0.1
  #define FREEFALL_TOTAL_ACCEL_MAX      11.5
  #define FREEFALL_VZ_THRESHOLD         -5.0
  #define FREEFALL_MIN_HEIGHT            5.0
  #define PARACHUTE_ALTITUDE_THRESHOLD  100.0
  ```

### Fase 3: BMP280Sensor (2h)

- **`sensors/BMP280Sensor.h`** — classe que encapsula:
  - Objeto `Adafruit_BMP280` (atual `BMP` global)
  - `base_pressure`, `max_altitude`, `previous_altitude` (membros privados)
  - **Cálculo de vz** no `update()`: `vz = (alt - prev_alt) / dt`, clipping [-200, 200]
  - `checkHighest()` como método privado chamado em `update()`
- Remove todos os globals de `bmp280_sensor.h`

### Fase 4: MPU6050Sensor (1.5h)

- **`sensors/MPU6050Sensor.h`** — classe que encapsula:
  - Objeto `Adafruit_MPU6050` (atual `MPU` global)
  - Aceleração X/Y/Z, giro X/Y/Z como membros privados
  - **Cálculo de `totalAccel`** no `update()`: `sqrt(ax² + ay² + az²)`
  - Remove buffer `temp` não utilizado
- Remove globals de `mpu6050_sensor.h`

### Fase 5: GPSModule (1h)

- **`sensors/GPSModule.h`** — classe que encapsula `TinyGPS++`
- Já é relativamente self-contained; encapsular `setupGPS()`, `getGPSTimeString()`, `GPSData()`

### Fase 6: FSM — Máquina de Estados (4h) ⭐ CRÍTICO

**`flight/FlightStateMachine.h`** — 4 estados + sub-eventos:

```
IDLE ──[totalAccel > 15]──→ ASCENT ──[apogee detectado]──→ DESCENT ──[vz≈0, alt<2]──→ LANDED
                               │                               │
                               ├─ BURNOUT (log event)          ├─ FREEFALL (log event)
                               └─ APOGEE (log event)           └─ PARACHUTE (log event)
```

```cpp
class FlightStateMachine {
private:
  FlightState currentState;
  FlightEvents events;
  BMP280Sensor* baro;
  MPU6050Sensor* imu;
  
  // Filtro de suavização (α=0.2 em ax,ay,az)
  float prev_filtered_ax, prev_filtered_ay, prev_filtered_az;
  bool first_reading;
  
  void checkTransitions();
  void transitionTo(FlightState newState);
  
  // Detecções (traduzidas de test/FSM/FSM.ino)
  bool detectLiftoff();      // totalAccel > 15
  bool detectBurnout();      // az < -8 || totalAccel < 2, guarda: height>5, vz>0.5
  bool detectApogee();       // |vz| < 1 && az < -0.1
  bool detectFreefall();     // totalAccel < 11.5 && vz < -5, guarda: height>5
  bool detectParachute();    // altitude <= 100 && vz < 0
  bool detectLanded();       // |vz| < 0.5 && height < 2
  
  // Suavização (de test/FSM)
  float smooth(float value, float prev, float alpha);
  
  // Validação de segurança (de FSM_Tester.py)
  bool isSafe(float value);

public:
  FlightStateMachine(BMP280Sensor* b, MPU6050Sensor* i);
  void begin();
  void update();
  FlightState getState() const;
  const FlightEvents& getEvents() const;
  const char* getStateName() const;
};
```

**Lógica de transição no `checkTransitions()`:**
```cpp
switch(currentState) {
  case IDLE:
    if (detectLiftoff()) {
      events.liftoff = true;
      transitionTo(ASCENT);
    }
    break;
    
  case ASCENT:
    if (!events.burnout && detectBurnout()) {
      events.burnout = true;  // sub-evento, NÃO muda estado
      log("BURNOUT");
    }
    if (detectApogee()) {
      events.apogee = true;
      transitionTo(DESCENT);
    }
    break;
    
  case DESCENT:
    if (!events.freefall && detectFreefall()) {
      events.freefall = true;  // sub-evento
      log("FREEFALL");
    }
    if (!events.parachute_deployed && detectParachute()) {
      events.parachute_deployed = true;  // sub-evento + acionar servo
      deployParachute();
      log("PARACHUTE");
    }
    if (detectLanded()) {
      transitionTo(LANDED);
    }
    break;
    
  case LANDED:
    break;
}
```

**Validações de segurança em cada `detect*()`:**
```cpp
bool FlightStateMachine::isSafe(float value) {
  return std::isfinite(value);
}

bool FlightStateMachine::detectLiftoff() {
  float totalAccel = computeTotalAccel();
  if (!isSafe(totalAccel)) return false;
  return totalAccel > LIFTOFF_TOTAL_ACCEL_THRESHOLD;
}
// ... mesma abordagem para todas
```

### Fase 7: FreeRTOS Tasks (4h)

Três tasks — **sem alterações significativas** do plano original:

| Task | Core | Prioridade | Freq. | Responsabilidade |
|------|------|------------|-------|-----------------|
| **FlightControl** | 1 | 20 (HIGH) | 50Hz | baro→update(), imu→update(), FSM→update(), deploy paraquedas |
| **Telemetry** | 0 | 5 (LOW) | 5Hz | GPS→update(), Serial, LoRa, LittleFS logging |
| **Logger** | 0 | 1 (IDLE+) | async | Recebe LogMessage via queue, Serial.printf thread-safe |

Queues: `sensorDataQueue` (25 slots), `logQueue` (50 slots)
Watchdog: 5s, apenas na Task FlightControl

### Fase 8: Integração firmware.ino (2h)

- `setup()`: instanciar sensores, FSM, criar queues, criar tasks
- `loop()`: `vTaskDelay(portMAX_DELAY)` — tasks controlam tudo

### Fase 9: Adaptar Módulos (1.5h)

- **`parachute_module.h`**: remover lógica de detecção (vai para FSM), manter apenas `setupServo()` e `deployServo()`
- **`telemetry_module.h`**: ler de `SensorData` struct em vez de chamar funções globais
- Deletar: `bmp280_sensor.h`, `mpu6050_sensor.h` (substituídos pelas classes)
- Atualizar `gps_module.h` para classe `GPSModule`

---

### Preocupação: Paraquedas

Uma decisão arquitetural precisa ser tomada:

**Tool: question**

**Input:**
```json
{
  "questions": [
    {
      "header": "Deploy do paraquedas",
      "multiple": false,
      "options": [
        {
          "description": "A FSM detecta e dispara o servo diretamente (via parachute_module reduzido). Mais simples, menos acoplamento.",
          "label": "FSM dispara o servo (Recomendado)"
        },
        {
          "description": "A FSM seta o flag e a FlightControl Task verifica e dispara. Separa detecção de ação.",
          "label": "Task FlightControl dispara baseado no evento da FSM"
        }
      ],
      "question": "Quando detectar o paraquedas (altitude≤100 em DESCENT), quem deve acionar o servo?"
    },
    {
      "header": "Mudar firmware.ino agora?",
      "multiple": false,
      "options": [
        {
          "description": "Manter o firmware.ino atual funcionando e ir integrando fase a fase, só trocando o loop() na Fase 8.",
          "label": "Manter firmware.ino atual até Fase 8 (Recomendado)"
        },
        {
          "description": "Começar a migrar o firmware.ino desde a Fase 1 (reorganizar includes, etc)",
          "label": "Migrar gradualmente desde o início"
        }
      ],
      "question": "O firmware.ino atual deve continuar funcionando durante as fases iniciais?"
    }
  ]
}
```

**Output:**
```
User has answered your questions: "Quando detectar o paraquedas (altitude≤100 em DESCENT), quem deve acionar o servo?"="Task FlightControl dispara baseado no evento da FSM", "O firmware.ino atual deve continuar funcionando durante as fases iniciais?"="Migrar gradualmente desde o início". You can now continue with the user's answers in mind.
```

---

## Assistant (Plan · mimo-v2-pro-free · 85.3s)

_Thinking:_

Now I have all the information. Let me finalize the plan:

1. Paraquedas: Task FlightControl dispara baseado no evento da FSM. So the FSM sets `events.parachute_deployed = true` and the FlightControl Task checks this and calls `deployServo()`.

2. firmware.ino: Migrar gradualmente desde o início. So we'll update the firmware.ino as we go through the phases, not just at phase 8.

Let me present the final plan summary. I should keep it concise since this is a CLI.

Plano completo definido. Aqui está o resumo final para aprovação:

---

## Plano Final de Refatoração — Resumo

**Branch:** `feature/oop-freertos-refactor`
**FSM:** 4 estados (`IDLE → ASCENT → DESCENT → LANDED`) + sub-eventos (BURNOUT, APOGEE, FREEFALL, PARACHUTE)
**Sensores:** BMP280 + MPU6050 encapsulados em classes `ISensor`
**Deploy paraquedas:** FSM detecta → seta flag → FlightControl Task aciona servo

### Sequência de 9 Fases

| # | Fase | Tempo | O que muda no firmware.ino? |
|---|------|-------|-----------------------------|
| 1 | Setup (pastas, branch) | 30min | Nada |
| 2 | ISensor + SensorData + config.h thresholds | 45min | Nada (só adiciona headers) |
| 3 | BMP280Sensor (classe, vz calc) | 2h | Substitui acesso direto a `BMP` e globals |
| 4 | MPU6050Sensor (classe, totalAccel calc) | 1.5h | Substitui chamadas a `MPUData()` |
| 5 | GPSModule (classe) | 1h | Substitui chamadas a `GPSData()` |
| 6 | **FlightStateMachine** (4 estados, detecções, filtros, validação NaN) | **4h** | Adiciona FSM no loop |
| 7 | FreeRTOS Tasks (FlightControl 50Hz, Telemetry 5Hz, Logger) | 4h | Loop vira tasks |
| 8 | Integração final firmware.ino | 2h | `setup()` cria objetos + tasks; `loop()` vazio |
| 9 | Limpeza — deletar código antigo, adaptar módulos | 1.5h | Remove headers antigos |
| **TOTAL** | | **~17h** | |

### Arquivos que serão criados
```
firmware/
├── sensors/
│   ├── ISensor.h              # Fase 2
│   ├── BMP280Sensor.h         # Fase 3
│   └── MPU6050Sensor.h        # Fase 4
├── flight/
│   ├── SensorData.h           # Fase 2 (enum FlightState, structs)
│   ├── FlightStateMachine.h   # Fase 6 (4 estados + sub-eventos)
│   ├── FlightControlTask.h    # Fase 7
│   ├── TelemetryTask.h        # Fase 7
│   └── LoggerTask.h           # Fase 7
├── modules/                   # Fase 9 (módulos existentes movidos)
│   ├── buzzer_module.h
│   ├── filesystem_module.h
│   ├── lora_module.h
│   ├── parachute_module.h    # Reduzido: só setupServo() + deployServo()
│   └── server_module.h
├── firmware.ino               # Migrado gradualmente
└── config.h                   # Thresholds FSM adicionados na Fase 2
```

### Arquivos que serão deletados
- `bmp280_sensor.h` → substituído por `BMP280Sensor` (Fase 9)
- `mpu6050_sensor.h` → substituído por `MPU6050Sensor` (Fase 9)
- `gps_module.h` → substituído por `GPSModule` (Fase 9)

### Principais riscos
| Risco | Mitigação |
|-------|-----------|
| Quebra do firmware entre fases | Cada fase mantém compilação funcional |
| Erro de NaN na FSM | Validação `isFinite()` em toda detecção |
| Queue overflow no FreeRTOS | Monitorar `uxQueueMessagesWaiting()` + tamanhos generosos |
| Debounce ausente no liftoff | Adicionar contador de confirmação (3 leituras) na Fase 6 |

**Tool: question**

**Input:**
```json
{
  "questions": [
    {
      "header": "Aprovação do plano",
      "multiple": false,
      "options": [
        {
          "description": "O plano está bom, pode começar a implementação pela Fase 1",
          "label": "Aprovado, começar implementação"
        },
        {
          "description": "Quero ajustar algo antes de começar",
          "label": "Preciso de ajustes"
        },
        {
          "description": "Quero só documentar o plano por enquanto",
          "label": "Salvar plano sem implementar"
        }
      ],
      "question": "O plano está aprovado para iniciar a implementação?"
    }
  ]
}
```

**Output:**
```
User has answered your questions: "O plano está aprovado para iniciar a implementação?"="Salvar plano sem implementar". You can now continue with the user's answers in mind.
```

---

## Assistant (Plan · mimo-v2-pro-free · 9.6s)

_Thinking:_

The user wants to save the plan without implementing it yet. I'm in plan mode (read-only), so I cannot create files. I should inform the user that:

1. I'm currently in read-only/plan mode and cannot write files
2. They need to exit plan mode to save the plan
3. I can provide the full plan in a format they can copy/save manually

Let me provide a concise plan document they can save.

Estou em modo de leitura (plan mode) e não posso criar arquivos. Você pode salvar este plano manualmente ou sair do modo de planejamento para que eu possa escrever o arquivo.

Para referência, o plano completo foi definido acima com todas as 9 fases adaptadas. Quando quiser implementar, basta me pedir para começar — posso salvar o plano em um arquivo `REFACTORING_PLAN_v2.md` e iniciar pela Fase 1.

Resumo rápido do que foi decidido:

- **4 estados** (`IDLE → ASCENT → DESCENT → LANDED`) com BURNOUT/APOGEE/FREEFALL/PARACHUTE como sub-eventos
- **Sensores atuais** (BMP280/MPU6050) encapsulados em classes `ISensor`
- **Deploy do paraquedas** pela FlightControl Task (não pela FSM diretamente)
- **Migração gradual** do `firmware.ino` desde a Fase 1
- **~17h estimadas** para as 9 fases

---

## User

escreva o novo plano

---

## Assistant (Build · mimo-v2-pro-free · 132.9s)

_Thinking:_

The user wants me to write the new refactoring plan to a file. I'm now in build mode, so I can write files. Let me create the REFACTORING_PLAN_v2.md file with all the details we discussed.

Let me also update the original REFACTORING_PLAN.md or create a new one. The user said "escreva o novo plano" (write the new plan). I think the best approach is to update the existing REFACTORING_PLAN.md since it's the main planning document, or create a v2. Let me check what makes more sense.

Actually, looking at it, the original REFACTORING_PLAN.md is very detailed and has a lot of content. 

I'll create REFACTORING_PLAN_v2.md with all the adaptations we discussed - the updated scope with 4 states and sub-events, current sensors integration, gradual firmware migration, parachute deployment logic through the FlightControl Task, debounce for liftoff detection, and NaN validation in each detection function.

---

## Assistant (Build · mimo-v2-pro-free · 1.9s)

---

## User

tente novamente

---

## Assistant (Build · mimo-v2-pro-free · 5.9s)

_Thinking:_

Now I'm in build mode. Let me write the new refactoring plan. I'll create the file REFACTORING_PLAN_v2.md in the firmware directory.

**Tool: read**

**Input:**
```json
{
  "filePath": "/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/REFACTORING_PLAN.md",
  "limit": 50
}
```

**Output:**
```
<path>/home/viniciusmonnerat/Documentos/Projetos/flight-computer/firmware/REFACTORING_PLAN.md</path>
<type>file</type>
<content>1: # 🚀 Plano de Refatoração: POO + FreeRTOS + FSM
2: 
3: **Projeto:** Flight Computer - Team #100  
4: **Hardware:** ESP32-S3-DevKitC-1-N8R8 (8MB Flash, 512KB RAM)  
5: **Data Início:** 2026-03-18  
6: **Status:** 📋 Planejamento Completo - Aguardando Implementação
7: 
8: ---
9: 
10: ## 📊 Resumo Executivo
11: 
12: ### Objetivos
13: 1. ✅ Refatorar código procedural para **POO seletivo** (sensores apenas)
14: 2. ✅ Implementar **FSM** para controle de estados de voo
15: 3. ✅ Usar **FreeRTOS** para separar lógica crítica (FSM) de I/O (telemetria)
16: 4. ✅ Substituir sensores: **BMP280→BMP585**, **MPU6050→LSM6DS3**, **GPS N6M→N8M**
17: 5. ✅ Preparar arquitetura para modificações futuras
18: 
19: ### Motivação
20: - Código monolítico (584 linhas) chegou ao **EOL**
21: - Troca de sensores planejada requer abstração
22: - FSM necessária para controle de voo robusto
23: - Separação safety-critical (paraquedas) de I/O (logging)
24: 
25: ---
26: 
27: ## 🎯 Decisões Arquiteturais
28: 
29: | Aspecto | Decisão | Justificativa |
30: |---------|---------|---------------|
31: | **Abordagem POO** | Seletivo | Apenas sensores que mudam viram classes |
32: | **FSM** | Enum/Switch | Simples, direto, baixo overhead |
33: | **FreeRTOS** | 3 Tasks | Separação FSM (50Hz) / Telemetry (5Hz) / Logger |
34: | **Migração** | Big Bang | Refatoração completa de uma vez |
35: | **Testes** | Manuais/Hardware | Sem infraestrutura de unit tests |
36: | **Queue Size** | 25 (sensores), 50 (logs) | Cobre pior caso com margem |
37: | **Watchdog** | 5s, apenas Task FSM | Reset automático se travamento crítico |
38: | **Logging** | Híbrido (Queue + #ifdef) | Debug verboso em dev, seletivo em produção |
39: 
40: ---
41: 
42: ## 🏗️ Arquitetura Final
43: 
44: ### Estrutura de Diretórios
45: 
46: ```
47: firmware/
48: │
49: ├── firmware.ino                    # Main - Setup + Tasks creation
50: │

(Showing lines 1-50 of 1316. Use offset=51 to continue.)
</content>
```

---

## Assistant (Build · mimo-v2-pro-free · 131.4s)

_Thinking:_

Let me write the new REFACTORING_PLAN_v2.md file with all the adaptations we discussed. I'll base it on the original but modify it significantly for the new decisions.

---

## Assistant (Build · mimo-v2-pro-free · 1.0s)

---

## User

tente novamente

---

## Assistant (Build · mimo-v2-pro-free · 138.5s)

_Thinking:_

I can now write files. Let me create the REFACTORING_PLAN_v2.md. I'll write a comprehensive but adapted plan.

---

## Assistant (Build · mimo-v2-pro-free · 1.2s)

---

