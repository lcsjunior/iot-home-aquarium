#ifndef TSTAT_H_
#define TSTAT_H_

#include <Arduino.h>
#include "relay.h"
#include "temp_sensor.h"

class Tstat {
  protected:
    enum State { IDLE, HEATING, COOLING };
    State _state = IDLE;
    TempSensor *_tempSensor;
    Relay *_k;
    float _setpoint;
    float _hysteresis;
    float _lowerLimit;
    float _upperLimit;
    unsigned long _interval;

  public:
    Tstat(TempSensor *tempSensor, Relay *k):
                                            _tempSensor(tempSensor),
                                            _k(k)
                                            {};
    State getState() const;
    char* getStatus() const;
    void setup(const float setpoint,
               const float hysteresis,
               const float lowerLimit,
               const float upperLimit,
               const unsigned long interval = 60000);
    void handleCooler();
    void handleHeater();
};

#endif