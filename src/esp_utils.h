#ifndef ESP_UTILS_H_
#define ESP_UTILS_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <sys/time.h>
#include "secrets.h"

static const char *ssid PROGMEM = SECRET_SSID;
static const char *pass PROGMEM = SECRET_PASS;
static const char *otaPass PROGMEM = ENV_OTA_AUTH;
static const char *hostname PROGMEM = ENV_HOSTNAME;
static const char *myTZ PROGMEM = "<-03>3";
static const char *ntpServer PROGMEM = "pool.ntp.org";
static const unsigned long wifiConnectInterval = 60000;
static const unsigned long wiFiRetryInterval = 30000;

void initWiFi();
void handleWiFi();
void printLocalTime();
void printUptime();
int dBmToQuality(int dBm);
int qualityTodBm(int quality);

#endif