#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <CronAlarms.h>
#include <ArduinoJson.h>
#include "config_json.h"
#include "esp_utils.h"
#include "builtinfiles.h"
#include "relay.h"
#include "temp_sensor.h"
#include "tstat.h"

Relay lamp(D5);
Relay heater(D6);
DSTempSensor tempSensor(D7);
Tstat tstat(&tempSensor, &heater);

const char *configFilename = "/.config.json";
Config config;
bool shouldReboot = false;

ESP8266WebServer server(80);
const char *www_user = BASIC_AUTH_USER;
const char *www_pass = BASIC_AUTH_PASS;

bool isAuthenticated();
void redirect();
void handleNotFound();
void handleRoot();
void handleHeap();
void handleFSInfo();
void handleReboot();
void handleConfigDetail();
void handleConfigUpdate();
void handleLampOn();
void handleLampOff();

void configTstat() {
  tstat.config(
    config.tstat.setpoint,
    config.tstat.hysteresis,
    0,
    29,
    300000UL);
}

void setup() {
  Serial.begin(115200);
  lamp.setup();
  heater.setup();
  tempSensor.setup();

  if (!LittleFS.begin()) {
    Serial.println(F("Failed to mount LittleFS"));
    return;
  }
  bool loaded = loadConfigFile(configFilename, config);
  if (!loaded) {
    Serial.println(F("Using default config"));
    strcpy(config.accessPoint[0].ssid, ssid);
    char encoded[32] = {0};
    XORCipher((char *)pass, encoded, cipherKey);
    char hexStr[(strlen(encoded) * 2) + 1] = {0};
    str2hex(encoded, hexStr);
    strcpy(config.accessPoint[0].pass, hexStr);
    config.accessPoints = 1;
  }
  saveConfigFile(configFilename, config);

  configTstat();

  initWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/heap", HTTP_GET, handleHeap);
  server.on("/fsinfo", HTTP_GET, handleFSInfo);
  server.on("/reboot", HTTP_GET, handleReboot);
  server.on("/config", HTTP_GET, handleConfigDetail);
  server.on("/config", HTTP_PUT, handleConfigUpdate);
  server.on("/lamp/on", HTTP_GET, handleLampOn);
  server.on("/lamp/off", HTTP_GET, handleLampOff);
  server.onNotFound(handleNotFound);
  server.begin();

  Cron.create("0 0 8 * * *", []() {
    lamp.turnOn();
  }, false);
  Cron.create("0 0 16 * * *", []() {
    lamp.turnOff();
  }, false);
}

void loop() {
  if (shouldReboot) {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  tstat.handleHeater();
  handleWiFi();
  server.handleClient();
  Cron.delay();
  delay(1000);
}

bool isAuthenticated() {
  if (server.authenticate(www_user, www_pass)) {
    return true;
  }
  server.requestAuthentication();
  return false;
}

void redirect(const char *url) {
  server.sendHeader("Location", (char *)url, true);
  server.send(302, "text/plain", "");
}

void handleNotFound() {
  server.send(404, "text/plain", FPSTR(notFoundText));
}

void handleRoot() {
  StaticJsonDocument<512> doc;
  doc["ssid"] = ssid;
  int8_t wifiQlt = dBm2Quality(WiFi.RSSI());
  doc["wifiQlt"] = wifiQlt;
  time_t now = time(nullptr);
  doc["time"] = now;
  doc["uptime"] = millis();
  doc["nextTrg"] = Cron.getNextTrigger();
  doc["isLampOn"] = lamp.isOn();
  doc["cTemp"] = tempSensor.getCTemp();
  doc["tstat"] = tstat.getStatus();
  doc["isHeaterOn"] = heater.isOn();
  String json((char *)0);
  serializeJson(doc, json);
  server.send(200, "application/json", json.c_str(), measureJson(doc));
}

void handleHeap() {
  uint32_t free;
  uint16_t max;
  uint8_t frag;
  ESP.getHeapStats(&free, &max, &frag);
  char buf[64];
  snprintf_P(buf,
              sizeof(buf),
              PSTR("free: %7u B - max: %7u B - frag: %3d%%"),
              free, max, frag
              );
  server.send(200, "text/plain", FPSTR(buf));
}

void handleFSInfo() {
  FSInfo fs_info;
  LittleFS.info(fs_info);
  float perc = roundf((float)fs_info.usedBytes / fs_info.totalBytes * 100.0);
  char buf[64];
  snprintf_P(buf,
              sizeof(buf),
              PSTR("total: %7u B - used: %7u B %3d%%"),
              fs_info.totalBytes,
              fs_info.usedBytes,
              (uint8_t)perc
              );
  server.send(200, "text/plain", FPSTR(buf));
}

void handleReboot() {
  if (!isAuthenticated()) return;
  shouldReboot = true;
  server.send(200);
}

void handleConfigDetail() {
  if (!isAuthenticated()) return;
  File file = LittleFS.open(configFilename, "r");
  if (!file) {
    server.send(400, "text/plain", FPSTR(openFileError));
    return;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    server.send(400, "text/plain", FPSTR(deserializeJSONError));
    return;
  }
  JsonArray aps = doc["access_points"];
  for (JsonObject ap : aps) {
    ap.remove("pass");
  }
  String json((char *)0);
  serializeJson(doc, json);
  server.send(200, "application/json", json.c_str(), measureJson(doc));
}

void handleConfigUpdate() {
  if (!isAuthenticated()) return;
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "text/plain", FPSTR(deserializeJSONError));
    return;
  }
  config.tstat.setpoint = doc["setpoint"];
  config.tstat.hysteresis = doc["hysteresis"];
  configTstat();
  saveConfigFile(configFilename, config);
  redirect("/config");
}

void handleLampOn() {
  if (!isAuthenticated()) return;
  lamp.turnOn();
  server.send(200);
}

void handleLampOff() {
  if (!isAuthenticated()) return;
  lamp.turnOff();
  server.send(200);
}