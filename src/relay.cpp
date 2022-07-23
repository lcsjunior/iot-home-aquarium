#include "relay.h"

void Relay::write() {
  digitalWrite(_pin, _isOn ? HIGH : LOW);
}

void Relay::setup() {
  pinMode(_pin, OUTPUT);
  write();
}

bool Relay::isOn() const {
  return _isOn;
}

bool Relay::isOff() const {
  return !_isOn;
}

void Relay::turnOn() {
  if (!_isOn) {
    _isOn = true;
    write();
  }
}

void Relay::turnOff() {
  if (_isOn) {
    _isOn = false;
    write();
  }
}

void Relay::toggle() {
  _isOn ? turnOff() : turnOn();
}
