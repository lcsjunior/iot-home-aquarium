#ifndef ESP_UTILS_H_
#define ESP_UTILS_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <sys/time.h>
#include "secrets.h"

static const char *ssid = SECRET_SSID;
static const char *pass = SECRET_PASS;
static const char *otaPass = SECRET_OTA_PASS;
static const char *hostname PROGMEM = "iot-home-aquarium";
static const char *myTZ PROGMEM = "<-03>3";
static const char *ntpServer PROGMEM = "pool.ntp.org";
static const unsigned long wifiConnectInterval = 60000;
static const unsigned long wiFiRetryInterval = 30000;

void initWiFi();
void handleWiFi();
void printLocalTime();
int dBmToQuality(int dBm);
int qualityTodBm(int quality);

#endif