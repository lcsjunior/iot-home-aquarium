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
static const char *otaPass = OTA_AUTH;
static const char *hostname = HOSTNAME;
static const char *cipherKey = CIPHER_KEY;
static const char *myTZ = "<-03>3";
static const char *ntpServer = "pool.ntp.org";
static const unsigned long wifiConnectInterval = 60000;
static const unsigned long wiFiRetryInterval = 30000;

void initWiFi();
void handleWiFi();
void printLocalTime();
void printUptime();
uint8_t dBm2Quality(int16_t dBm);
int16_t quality2dBm(uint8_t quality);
char *XORCipher(char* in, char* out, const char* key);
void str2hex(char* in, char* out);
void hex2str(char* in, char* out, uint8_t outLen);

#endif