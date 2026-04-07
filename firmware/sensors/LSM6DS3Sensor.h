#ifndef LSM6DS3SENSOR_H
#define LSM6DS3SENSOR_H

#include "ISensor.h"
#include <Adafruit_LSM6DS.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>


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
  float getTotalAccel() const;    
private:
  Adafruit_LSM6DS3 lsm;
  bool ready;
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float total_accel;  // Calculado em update()
};


#endif // LSM6DS3SENSOR_H