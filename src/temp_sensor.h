#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SHTSensor.h>

class TempSensor {
  public:
    virtual float getCTemp() = 0;
};

class DSTempSensor: public TempSensor {
  private:
    DallasTemperature _sensors;

  public:
    DSTempSensor(const byte pin):
                                  _sensors(DallasTemperature(new OneWire(pin)))
                                  {};
    void setup();
    float getCTemp();
};

class SHTTempSensor: public TempSensor {
  private:
    SHTSensor _sht;

  public:
    SHTTempSensor():
                    _sht(SHTSensor(SHTSensor::SHT3X))
                    {};
    void setup();
    float getCTemp();
};

#endif