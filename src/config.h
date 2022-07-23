#ifndef CONFIG_H_
#define CONFIG_H_

struct ApConfig {
  char ssid[32];
  char pass[64];
};

struct ThermostatConfig {
  double setpoint = 24;
  double hysteresis = 0.5;
};

struct Config {
  static const uint8_t maxAccessPoints = 4;
  ApConfig accessPoint[maxAccessPoints];
  uint8_t accessPoints = 0;
  ThermostatConfig thermostat;
};

#endif