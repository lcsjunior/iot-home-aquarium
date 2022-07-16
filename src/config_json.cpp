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
  JsonArray aps = dst.createNestedArray("access_points");
  for (int i = 0; i < src.accessPoints; i++)
    aps.add(src.accessPoint[i]);
}

void convertFromJson(JsonVariantConst src, Config &dst) {
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
  dst["passphrase"] = src.passphrase;
}

void convertFromJson(JsonVariantConst src, ApConfig &dst) {
  strncpy_P(dst.ssid, src["ssid"] | "", sizeof(dst.ssid));
  strncpy_P(dst.passphrase, src["passphrase"] | "", sizeof(dst.passphrase));
}