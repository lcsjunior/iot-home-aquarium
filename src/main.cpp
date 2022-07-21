#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <CronAlarms.h>
#include <ArduinoJson.h>
#include "config_json.h"
#include "esp_utils.h"
#include "builtinfiles.h"

const char *configFilename PROGMEM = "/config.json";
Config config;
bool shouldReboot = false;

ESP8266WebServer server(80);

const char *www_user PROGMEM = BASIC_AUTH_USER;
const char *www_pass PROGMEM = BASIC_AUTH_PASS;

void handleRoot();
void handleNotFound();
void handleFreeHeap();
void handleConfigDetail();

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println(F("Failed to mount LittleFS"));
    return;
  }
  LittleFS.remove(configFilename);
  bool loaded = loadConfigFile(configFilename, config);
  if (!loaded) {
    Serial.println(F("Using default config"));
    strcpy_P(config.accessPoint[0].ssid, ssid);
    char encoded[64] = {0};
    XORCipher((char *)pass, encoded, cipherKey);
    Serial.println(encoded);
    char decoded[64] = {0};
    XORCipher(encoded, decoded, cipherKey);
    Serial.println(decoded);
    strcpy_P(config.accessPoint[0].pass, encoded);
    config.accessPoints = 1;
  }
  saveConfigFile(configFilename, config);

  initWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/free-heap", HTTP_GET, handleFreeHeap);
  server.on("/config", HTTP_GET, handleConfigDetail);
  server.onNotFound(handleNotFound);
  server.begin();

  Cron.create("0 0 8 * * *", []() {}, false);
  Cron.create("0 0 16 * * *", []() {}, false);
}

void loop() {
  if (shouldReboot) {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  handleWiFi();
  server.handleClient();
  Cron.delay();
  printLocalTime();
  printUptime();
  delay(1000);
}

void handleRoot() {
  StaticJsonDocument<512> doc;
  time_t now = time(nullptr);
  int8_t rssi = WiFi.RSSI();
  doc[F("ssid")] = ssid;
  doc[F("rssi")] = rssi;
  doc[F("wifiQlt")] = dBmToQuality(rssi);
  doc[F("time")] = now;
  doc[F("uptime")] = millis();
  doc[F("nextTrg")] = Cron.getNextTrigger();
  String json((char *)0);
  serializeJson(doc, json);
  server.send(200, "application/json", json.c_str(), measureJson(doc));
}

void handleNotFound() {
  server.send(404, "text/plain", FPSTR(notFoundContent));
}

void handleFreeHeap() {
  char buf[16];
  snprintf_P(buf, sizeof(buf), PSTR("%lu B"), ESP.getFreeHeap());
  server.send(200, "text/plain", FPSTR(buf));
}

void handleConfigDetail() {
  if (!server.authenticate(www_user, www_pass)) {
    return server.requestAuthentication();
  }
  File file = LittleFS.open(configFilename, "r");
  if (!file) {
    server.send(400, "text/plain", FPSTR(errorOpenFileContent));
    return;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    server.send(400, "text/plain", FPSTR(errorDeserializeFileContent));
    return;
  }
  String json((char *)0);
  serializeJson(doc, json);
  server.send(200, "application/json", json.c_str(), measureJson(doc));
}