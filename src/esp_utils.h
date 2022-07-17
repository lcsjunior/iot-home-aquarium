#ifndef ESP_UTILS_H_
#define ESP_UTILS_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <sys/time.h>
#include "secrets.h"

extern const char *notFoundContent;

void initWiFi();
void handleWiFi();
void printLocalTime();
int dBmToQuality(int dBm);
int qualityTodBm(int quality);

#endif