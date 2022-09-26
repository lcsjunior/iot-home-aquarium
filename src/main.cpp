#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <CronAlarms.h>
#include <ArduinoJson.h>
#include <ThingSpeak.h>
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

WiFiClient client;
// const unsigned long myChannelNumber = SECRET_CH_ID;
// const char *myWriteAPIKey = SECRET_WRITE_APIKEY;
static const unsigned long writeChInterval = 15000;
unsigned long writeChPreviousMillis = 0;

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
  ThingSpeak.begin(client);
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

  Cron.create(config.lamp.turnOn, []() {
    lamp.turnOn();
  }, false);
  Cron.create(config.lamp.turnOff, []() {
    lamp.turnOff();
  }, false);
}

void loop() {
  if (shouldReboot) {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  Cron.delay();
  tstat.handleHeater();
  handleWiFi();
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - writeChPreviousMillis >= writeChInterval) {
    ThingSpeak.setField(1, tstat.getState());
    ThingSpeak.setField(2, tempSensor.getCTemp());
    ThingSpeak.setField(3, heater.isOn());
    ThingSpeak.setField(4, lamp.isOn());
    int statusCode = ThingSpeak.writeFields(
      config.thingSpeak.chID,
      config.thingSpeak.wkey);
    if (statusCode == 200) {
      Serial.println(F("Channel update successful."));
      writeChPreviousMillis = currentMillis;
    } else {
      Serial.printf_P(PSTR("Problem updating channel. HTTP error code %d\r\n"), statusCode);
    }
  }
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
  server.send(404, "text/plain", FPSTR(textNotFoundMessage));
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
  doc["lampOn"] = lamp.isOn();
  doc["cTemp"] = tempSensor.getCTemp();
  doc["tstatStats"] = tstat.getStatus();
  doc["heaterOn"] = heater.isOn();
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
    server.send(400, "text/plain", FPSTR(openFileFailedMessage));
    return;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    server.send(400, "text/plain", FPSTR(deserializeJSONFiledMessage));
    return;
  }
  JsonArray aps = doc["accessPoints"];
  for (JsonObject ap : aps) {
    ap.remove("pass");
  }
  JsonObject thingSpeak = doc["thingSpeak"];
  thingSpeak.remove("rkey");
  thingSpeak.remove("wkey");
  String json((char *)0);
  serializeJson(doc, json);
  server.send(200, "application/json", json.c_str(), measureJson(doc));
}

void handleConfigUpdate() {
  if (!isAuthenticated()) return;
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "text/plain", FPSTR(deserializeJSONFiledMessage));
    return;
  }
  config.thingSpeak.chID = doc["chID"];
  strncpy(config.thingSpeak.rkey, doc["rkey"] | "", sizeof(config.thingSpeak.rkey));
  strncpy(config.thingSpeak.wkey, doc["wkey"] | "", sizeof(config.thingSpeak.wkey));
  strncpy(config.lamp.turnOn, doc["turnOnLamp"] | "", sizeof(config.lamp.turnOn));
  strncpy(config.lamp.turnOff, doc["turnOffLamp"] | "", sizeof(config.lamp.turnOff));
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