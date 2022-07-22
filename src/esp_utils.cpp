#include "esp_utils.h"

unsigned long wiFiRetryPreviousMillis = 0;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println();
  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Default hostname: "));
  Serial.println(hostname);
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println(F("Disconnected from Wi-Fi"));
  WiFi.disconnect();
}

void initWiFi() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, pass);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED && millis() <= wifiConnectInterval) {
    Serial.print(F("."));
    delay(500);
  }
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  configTzTime(myTZ, ntpServer);

  ArduinoOTA.setHostname((const char *)hostname); // mDNS (*.local)
  ArduinoOTA.setPassword((const char *)otaPass);
  ArduinoOTA.onStart([]() {
    String type((char *)0);
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = F("sketch");
    } else { // U_FS
      type = F("filesystem");
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.printf_P(PSTR("Start updating %s"), type);
    Serial.println();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf_P(PSTR("Progress: %u%%\r"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf_P(PSTR("Error[%u]: "), error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("End Failed"));
    }
  });
  ArduinoOTA.begin();
}

void handleWiFi() {
  unsigned long currentMillis = millis();
  if (WiFi.status() != WL_CONNECTED && currentMillis - wiFiRetryPreviousMillis >= wiFiRetryInterval) {
    Serial.println(F("Reconnecting to WiFi..."));
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    wiFiRetryPreviousMillis = currentMillis;
  }
  ArduinoOTA.handle();
}

void printLocalTime() {
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);
  char buf[64];
  strftime(buf, sizeof(buf), "%A, %B %d %Y %H:%M:%S", timeinfo);
  Serial.println(buf);
}

void printUptime() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  Serial.printf_P(PSTR("Uptime: %02d:%02d:%02d\r\n"), hr, min % 60, sec % 60);
}

uint8_t dBm2Quality(int16_t dBm) {
  if (dBm <= -100) return 0;
  else if (dBm >= -50) return 100;
  return 2 * (dBm + 100);
}

int16_t quality2dBm(uint8_t quality) {
  if (quality <= 0) return -100;
  else if (quality >= 100) return -50;
  return (quality / 2) - 100;
}

char *XORCipher(char* in, char* out, const char* key) {
  uint8_t inLen = strlen(in);
  uint8_t keyLen = strlen(cipherKey);
  for (uint8_t i = 0; i < inLen; ++i) {
    out[i] = in[i] ^ key[i % keyLen];
  }
  return out;
}

void str2hex(char* in, char* out) {
  uint8_t loop = 0;
  uint8_t i = 0;
  while (in[loop] != '\0') {
    sprintf((char*)(out + i), "%02X", in[loop]);
    loop += 1;
    i += 2;
  }
  out[i++] = '\0';
}

void hex2str(char* in, char* out, uint8_t outLen) {
  int inLen = strlen(in);
  memset(out, '\0', outLen);
  if (inLen % 2 != 0 || inLen / 2 >= outLen)
    return;
  for (uint8_t i = 0; i < inLen; i += 2) {
    char msb = (in[i+0] <= '9' ? in[i+0] - '0' : (in[i+0] & 0x5F) - 'A' + 10);
    char lsb = (in[i+1] <= '9' ? in[i+1] - '0' : (in[i+1] & 0x5F) - 'A' + 10);
    out[i / 2] = (msb << 4) | lsb;
  }
}
