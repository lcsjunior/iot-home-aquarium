#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <CronAlarms.h>
#include <ArduinoJson.h>
#include "esp_utils.h"
#include "config_json.h"

const char *configFilename PROGMEM = "/config.json";
Config config;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println(F("Failed to mount LittleFS"));
    return;
  }
  // LittleFS.remove(configFilename);
  bool loaded = loadConfigFile(configFilename, config);
  if (!loaded) {
    Serial.println(F("Using default config"));
    strcpy_P(config.accessPoint[0].ssid, SECRET_SSID);
    strcpy_P(config.accessPoint[0].passphrase, SECRET_PASS);
    config.accessPoints = 1;
  }
  saveConfigFile(configFilename, config);

  initWiFi();

  server.on("/", []() {
    StaticJsonDocument<512> doc;
    time_t now = time(nullptr);
    doc[F("time")] = now;
    doc[F("ssid")] = WiFi.SSID();
    doc[F("rssi")] = WiFi.RSSI();
    doc[F("wifiQlt")] = dBmToQuality(WiFi.RSSI());
    doc[F("nextTrg")] = Cron.getNextTrigger();
    String json((char *)0);
    serializeJson(doc, json);
    server.send(200, "application/json", json.c_str(), measureJson(doc));
  });
  server.on("/heap", []() {
    char buf[16];
    snprintf_P(buf, sizeof(buf), PSTR("%lu B"), ESP.getFreeHeap());
    server.send(200, "text/plain", buf);
  });
  server.onNotFound([]() {
    server.send(404, "text/plain", notFoundContent);
  });
  server.begin();

  Cron.create("0 0 8 * * *", []() {}, false);
  Cron.create("0 0 16 * * *", []() {}, false);
}

void loop() {
  handleWiFi();
  server.handleClient();
  Cron.delay();
  printLocalTime();
  delay(1000);
}
