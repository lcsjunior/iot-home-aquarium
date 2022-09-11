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
  JsonArray aps = dst.createNestedArray("accessPoints");
  for (uint8_t i = 0; i < src.accessPoints; i++)
    aps.add(src.accessPoint[i]);
  dst["thingSpeak"] = src.thingSpeak;
  dst["lamp"] = src.lamp;
  dst["tstat"] = src.tstat;
}

void convertFromJson(JsonVariantConst src, Config &dst) {
  JsonArrayConst aps = src["accessPoints"];
  dst.accessPoints = 0;
  for (JsonVariantConst ap : aps) {
    dst.accessPoint[dst.accessPoints] = ap;
    dst.accessPoints++;
    if (dst.accessPoints >= Config::maxAccessPoints)
      break;
  }
  dst.thingSpeak = src["thingSpeak"];
  dst.lamp = src["lamp"];
  dst.tstat = src["tstat"];
}

void convertToJson(const ApConfig &src, JsonVariant dst) {
  dst["ssid"] = src.ssid;
  dst["pass"] = src.pass;
}

void convertFromJson(JsonVariantConst src, ApConfig &dst) {
  strncpy(dst.ssid, src["ssid"] | "", sizeof(dst.ssid));
  strncpy(dst.pass, src["pass"] | "", sizeof(dst.pass));
}

void convertToJson(const ThingSpeakConfig &src, JsonVariant dst) {
  dst["chID"] = src.chID;
  dst["rkey"] = src.rkey;
  dst["wkey"] = src.wkey;
}

void convertFromJson(JsonVariantConst src, ThingSpeakConfig &dst) {
  dst.chID = src["chID"];
  strncpy(dst.rkey, src["rkey"] | "", sizeof(dst.rkey));
  strncpy(dst.wkey, src["wkey"] | "", sizeof(dst.wkey));
}

void convertToJson(const LampConfig &src, JsonVariant dst) {
  dst["turnOn"] = src.turnOn;
  dst["turnOff"] = src.turnOff;
}

void convertFromJson(JsonVariantConst src, LampConfig &dst) {
  strncpy(dst.turnOn, src["turnOn"] | "", sizeof(dst.turnOn));
  strncpy(dst.turnOff, src["turnOff"] | "", sizeof(dst.turnOff));
}

void convertToJson(const TstatConfig &src, JsonVariant dst) {
  dst["setpoint"] = src.setpoint;
  dst["hysteresis"] = src.hysteresis;
}

void convertFromJson(JsonVariantConst src, TstatConfig &dst) {
  dst.setpoint = src["setpoint"];
  dst.hysteresis = src["hysteresis"];
}