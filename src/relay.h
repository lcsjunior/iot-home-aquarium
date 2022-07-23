#ifndef RELAY_H_
#define RELAY_H_

#include <Arduino.h>

class Relay {
  private:
    byte _pin;
    bool _isOn = false;
    void write();

  public:
    Relay(const byte pin):
                          _pin(pin)
                          {};
    void setup();
    bool isOn() const;
    bool isOff() const;
    void turnOn();
    void turnOff();
    void toggle();
};

#endif