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
static const char *otaPass PROGMEM = OTA_AUTH;
static const char *hostname PROGMEM = HOSTNAME;
static const char *cipherKey PROGMEM = CIPHER_KEY;
static const char *myTZ PROGMEM = "<-03>3";
static const char *ntpServer PROGMEM = "pool.ntp.org";
static const unsigned long wifiConnectInterval = 60000;
static const unsigned long wiFiRetryInterval = 30000;

void initWiFi();
void handleWiFi();
void printLocalTime();
void printUptime();
uint8_t dBmToQuality(int16_t dBm);
int16_t qualityTodBm(uint8_t quality);
char *XORCipher(char* src, char* dest, const char* key);

#endif