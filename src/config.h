#ifndef CONFIG_H_
#define CONFIG_H_

struct ApConfig {
  char ssid[32];
  char pass[64];
};

struct ThingSpeakConfig {
  uint32_t chID;
  char rkey[32];
  char wkey[32];
};

struct LampConfig {
  char turnOn[32] = "0 0 8 * * *";
  char turnOff[32] = "0 0 16 * * *";
};

struct TstatConfig {
  double setpoint = 24;
  double hysteresis = 0.5;
};

struct Config {
  static const uint8_t maxAccessPoints = 4;
  ApConfig accessPoint[maxAccessPoints];
  uint8_t accessPoints = 0;
  ThingSpeakConfig thingSpeak;
  LampConfig lamp;
  TstatConfig tstat;
};

#endif