#include "config_json.h"

bool loadConfigFile(const char *filename, Config &config) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to open config file"));
    return false;
  }
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    Serial.println(F("Failed to deserialize configuration: "));
    Serial.println(err.f_str());
    return false;
  }
  config = doc.as<Config>();
  return true;
}

void saveConfigFile(const char *filename, const Config &config) {
  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create config file"));
    return;
  }
  DynamicJsonDocument doc(1024);
  doc.set(config);
  bool success = serializeJsonPretty(doc, file) > 0;
  if (!success) {
    Serial.println(F("Failed to serialize configuration"));
  }
}

void printFile(const char *filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to open config file"));
    return;
  }
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();
}

void convertToJson(const Config &src, JsonVariant dst) {
  dst["thermostat"] = src.thermostat;
  JsonArray aps = dst.createNestedArray("access_points");
  for (uint8_t i = 0; i < src.accessPoints; i++)
    aps.add(src.accessPoint[i]);
}

void convertFromJson(JsonVariantConst src, Config &dst) {
  dst.thermostat = src["thermostat"];
  JsonArrayConst aps = src["access_points"];
  dst.accessPoints = 0;
  for (JsonVariantConst ap : aps) {
    dst.accessPoint[dst.accessPoints] = ap;
    dst.accessPoints++;
    if (dst.accessPoints >= Config::maxAccessPoints)
      break;
  }
}

void convertToJson(const ApConfig &src, JsonVariant dst) {
  dst["ssid"] = src.ssid;
  dst["pass"] = src.pass;
}

void convertFromJson(JsonVariantConst src, ApConfig &dst) {
  strncpy(dst.ssid, src["ssid"] | "", sizeof(dst.ssid));
  strncpy(dst.pass, src["pass"] | "", sizeof(dst.pass));
}

void convertToJson(const ThermostatConfig &src, JsonVariant dst) {
  dst["setpoint"] = src.setpoint;
  dst["hysteresis"] = src.hysteresis;
}

void convertFromJson(JsonVariantConst src, ThermostatConfig &dst) {
  dst.setpoint = src["setpoint"];
  dst.hysteresis = src["hysteresis"];
}