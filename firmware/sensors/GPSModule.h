#ifndef GPS_MODULE_OOP_H
#define GPS_MODULE_OOP_H

#include "ISensor.h"
#include <TinyGPS++.h>
#include <Arduino.h>

/**
 * @brief GPS receiver wrapper implementando a interface ISensor.
 *
 * Alimenta o parser TinyGPS++ de forma não-bloqueante em update().
 * Chamada esperada: TelemetryTask a 5 Hz.
 *
 * Pinos configurados em config.h (RX_GPS / TX_GPS), baud 9600.
 */
class GPSModule : public ISensor {
public:
  GPSModule(HardwareSerial* serial);

  // Interface ISensor
  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;

  // Específicos do GPS
  String getTimeString() const;
  String getDateString() const;
  bool hasValidFix() const;

  // Accessors para preencher SensorData
  double getLatitude()   const { return _gps.location.isValid() ? _gps.location.lat() : 0.0; }
  double getLongitude()  const { return _gps.location.isValid() ? _gps.location.lng() : 0.0; }
  float  getGPSAltitude() const { return _gps.location.isValid() ? (float)_gps.altitude.meters() : 0.0f; }
  uint8_t getSatellites() const { return (uint8_t)_gps.satellites.value(); }

private:
  TinyGPSPlus _gps;
  HardwareSerial* _serial;
  bool _ready;
};

#endif // GPS_MODULE_OOP_H
