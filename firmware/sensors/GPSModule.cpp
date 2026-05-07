#include "GPSModule.h"
#include "../config.h"

GPSModule::GPSModule(HardwareSerial* serial) : _serial(serial), _ready(false) {}

bool GPSModule::begin() {
  _serial->begin(9600, SERIAL_8N1, RX_GPS, TX_GPS);
  _ready = true;
  return true;
}

void GPSModule::update() {
  if (!_ready) return;
  while (_serial->available() > 0) {
    _gps.encode(_serial->read());
  }
}

String GPSModule::getData() {
  return getTimeString() + "," + getDateString() + "," +
         (hasValidFix() ? String(_gps.altitude.meters(), 2) : "nan") + "," +
         (hasValidFix() ? String(_gps.location.lat(), 8)   : "nan") + "," +
         (hasValidFix() ? String(_gps.location.lng(), 8)   : "nan") + "," +
         String(_gps.satellites.value());
}

bool GPSModule::isReady() { return _ready; }

String GPSModule::getTimeString() const {
  if (!_gps.time.isValid()) return "nan";
  char buf[16];
  sprintf(buf, "%02d:%02d:%02d", _gps.time.hour(), _gps.time.minute(), _gps.time.second());
  return String(buf);
}

String GPSModule::getDateString() const {
  if (!_gps.date.isValid()) return "nan";
  char buf[16];
  sprintf(buf, "%04d/%02d/%02d", _gps.date.year(), _gps.date.month(), _gps.date.day());
  return String(buf);
}

bool GPSModule::hasValidFix() const {
  return _gps.location.isValid();
}
