#include "tstat.h"

unsigned long exitIdlePreviousMillis = 0;

void Tstat::setup(const float setpoint,
                  const float hysteresis,
                  const float lowerLimit,
                  const float upperLimit,
                  const unsigned long interval)
{
  _setpoint = setpoint;
  _hysteresis = hysteresis;
  _lowerLimit = lowerLimit;
  _upperLimit = upperLimit;
  _interval = interval;
}

Tstat::State Tstat::getState() const {
  return _state;
}

char* Tstat::getStatus() const {
  switch (_state) {
    case IDLE:    return (char*)"Idle";
    case HEATING: return (char*)"Heating";
    case COOLING: return (char*)"Cooling";
    default:      return (char*)"[Unknown Status]";
  }
}

void Tstat::handleCooler() {}

void Tstat::handleHeater() {
  if (isnan(_tempSensor->getCTemp()) ||
      _tempSensor->getCTemp() < _lowerLimit ||
      _tempSensor->getCTemp() > _upperLimit
  ) {
    _k->turnOff();
    return;
  }

  unsigned long currentMillis = millis();
  switch(_state) {
    case IDLE:
      _k->turnOff();
      if (currentMillis - exitIdlePreviousMillis >= _interval) {
        _state = COOLING;
      }
      break;
    case COOLING:
      _k->turnOff();
      if (_tempSensor->getCTemp() <= _setpoint) {
        _state = HEATING;
      }
      break;
    default:
      _k->turnOn();
      if (_tempSensor->getCTemp() >= _setpoint+_hysteresis) {
        exitIdlePreviousMillis = currentMillis;
        _state = IDLE;
      }
  }
}