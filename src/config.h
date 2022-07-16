#ifndef CONFIG_H_
#define CONFIG_H_

struct ApConfig {
  char ssid[32];
  char passphrase[64];
};

struct Config {
  static const int maxAccessPoints = 4;
  ApConfig accessPoint[maxAccessPoints];
  int accessPoints = 0;
};

#endif