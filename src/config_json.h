#ifndef CONFIG_JSON_H_
#define CONFIG_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"

bool loadConfigFile(const char *filename, Config &config);
void saveConfigFile(const char *filename, const Config &config);
void printFile(const char *filename);
void convertToJson(const Config &src, JsonVariant dst);
void convertFromJson(JsonVariantConst src, Config &dst);
void convertToJson(const ApConfig &src, JsonVariant dst);
void convertFromJson(JsonVariantConst src, ApConfig &dst);

#endif