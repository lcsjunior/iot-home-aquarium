#include "temp_sensor.h"

void DSTempSensor::setup() {
  _sensors.begin();
}

float DSTempSensor::getCTemp() {
  _sensors.requestTemperatures();
  return _sensors.getTempCByIndex(0);
}

void SHTTempSensor::setup() {
  if (_sht.init()) {
    _sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);
    Serial.println(F("SHT initialization successful"));
  } else {
    Serial.println(F("SHT initialization failed"));
  }
}

float SHTTempSensor::getCTemp() {
  _sht.readSample();
  return _sht.getTemperature();
}