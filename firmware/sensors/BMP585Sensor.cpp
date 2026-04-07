#include "BMP585Sensor.h"

BMP585Sensor::BMP585Sensor()
    : ready(false),
      base_pressure(0.0F),
      altitude(0.0F),
      temperature(0.0F),
      pressure(0.0F),
      max_altitude(0.0F),
      prev_altitude(0.0F),
      prev_time(0UL),
      vertical_velocity(0.0F) {}

bool BMP585Sensor::begin() {
  if (!bmp.begin_I2C()) {
    Serial.println("BMP585 initialization failed.");
    ready = false;
    return false;
  }

  if (!bmp.performReading()) {
    Serial.println("BMP585 first reading failed.");
    ready = false;
    return false;
  }

  // inicializando dados da base
  pressure = bmp.pressure / 100.0F;
  temperature = bmp.temperature;
  base_pressure = pressure;
  altitude = bmp.readAltitude(base_pressure);

  prev_altitude = altitude;
  max_altitude = altitude;
  prev_time = millis();
  vertical_velocity = 0.0F;
  ready = true;

  return true;
}

void BMP585Sensor::update() {
  if (!isReady()) {
    return;
  }

  if (!bmp.performReading()) {
    return;
  }

  const unsigned long current_time = millis();
  const float current_altitude = bmp.readAltitude(base_pressure);

  pressure = bmp.pressure / 100.0F;
  temperature = bmp.temperature;
  altitude = current_altitude;

  const float dt = (current_time - prev_time) / 1000.0F;
  if (dt > 0.001F) {
    float vz = (current_altitude - prev_altitude) / dt;
    
    // Clip to a physically plausible range to reduce numerical spikes.
    if (vz > 200.0F) {
      vz = 200.0F;
    } else if (vz < -200.0F) {
      vz = -200.0F;
    }

    vertical_velocity = vz;
    prev_altitude = current_altitude;
    prev_time = current_time;
  }

  checkHighest();
}

String BMP585Sensor::getData() {
  return String(altitude) + "," + String(temperature) + ",nan," +
         String(pressure);
}

bool BMP585Sensor::isReady() {
  return ready;
}

float BMP585Sensor::getAltitude() const {
  return altitude;
}

float BMP585Sensor::getMaxAltitude() const {
  return max_altitude;
}

float BMP585Sensor::getVerticalVelocity() const {
  return vertical_velocity;
}

void BMP585Sensor::checkHighest() {
  if (altitude > max_altitude) {
    max_altitude = altitude;
  }
}

