#ifndef BMP585_SENSOR_H
#define BMP585_SENSOR_H

#include "ISensor.h"
#include <Adafruit_BMP5XX.h>
#include <Arduino.h>

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
  float getVerticalVelocity() const; // ⚠️ CRÍTICO: Calculado via diferenciação numérica
  void checkHighest();

private:
  Adafruit_BMP5XX _bmp;
  bool _ready;
  float base_pressure;
  float altitude;
  float temperature;
  float pressure;
  float max_altitude;
  float prev_altitude;
  unsigned long prev_time;
  float vertical_velocity; // Vz = (altitude_current - altitude_previous) / dt
};

#endif // BMP585_SENSOR_H
