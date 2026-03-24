// ================================
// 🚀 FMS Simplificada - timestamps reais
// ================================

#include <math.h>

const float PARACHUTE_ALTITUDE = 100.0;

enum FlightState { IDLE = 0, ASCENT = 1, DESCENT = 2, LANDED = 3 };
FlightState current_state = IDLE;

float prev_altp = 0.0;
float prev_millis = 0.0;
bool first_reading = true;
float prev_vz = 0.0;
float prev_acc = 0.0;

// Converte enum para string estilo Python
const char* state_name(FlightState state) {
  switch(state) {
    case IDLE: return "IDLE";
    case ASCENT: return "ASCENT";
    case DESCENT: return "DESCENT";
    case LANDED: return "LANDED";
    default: return "UNKNOWN";
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 FMS Simplificada");
  Serial.println("Formato: millis,altp,ax,ay,az");
}

void print_event(const char* event, float t, float h, float vz, FlightState state) {
  // Usa o timestamp do CSV (tempo real do voo)
  Serial.print(event);
  Serial.print(" @ ");
  Serial.print(t, 2);
  Serial.print("s | h=");
  Serial.print(h, 1);
  Serial.print(" | vz=");
  Serial.print(vz, 1);
  Serial.print(" | state=State.");
  Serial.println(state_name(state));
}

void process_sensor_data(float millis, float altp, float ax, float ay, float az) {
  float vz = 0.0;

  if (!first_reading) {
    float dt = millis - prev_millis;
    if (dt > 0) vz = (altp - prev_altp)/dt;
  } else first_reading = false;

  float acc = sqrt(ax*ax + ay*ay + az*az);
  const char* event = NULL;

  // ===== FSM =====
  if (current_state == IDLE && acc > 12.0) {
    current_state = ASCENT;
    event = "LIFTOFF";
  }

  if (current_state == ASCENT) {
    if (prev_acc > 10.0 && acc < 5.0) event = "BURNOUT";
    if (prev_vz > 0 && vz <= 0) {
      current_state = DESCENT;
      event = "APOGEE";
    }
  }

  if (current_state == DESCENT) {
    if (vz < -5.0 && acc < 12.0) event = "FREEFALL";
    if (altp < PARACHUTE_ALTITUDE && vz < 0) event = "PARACHUTE";
    if (abs(vz) < 0.5 && altp < 2.0) {
      current_state = LANDED;
      event = "LANDED";
    }
  }

  // ===== PRINT EVENT =====
  if (event != NULL) print_event(event, millis, altp, vz, current_state);

  // ===== Atualiza histórico =====
  prev_altp = altp;
  prev_millis = millis;
  prev_vz = vz;
  prev_acc = acc;
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    data.trim();

    int i1 = data.indexOf(',');
    int i2 = data.indexOf(',', i1 + 1);
    int i3 = data.indexOf(',', i2 + 1);
    int i4 = data.indexOf(',', i3 + 1);

    if (i1 != -1 && i2 != -1 && i3 != -1 && i4 != -1) {
      float millis = data.substring(0, i1).toFloat(); // tempo real do CSV
      float altp   = data.substring(i1 + 1, i2).toFloat();
      float ax     = data.substring(i2 + 1, i3).toFloat();
      float ay     = data.substring(i3 + 1, i4).toFloat();
      float az     = data.substring(i4 + 1).toFloat();

      process_sensor_data(millis, altp, ax, ay, az);
    }
  }
}