#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>
#include <ESP8266FtpServer.h>
#include <EEPROM.h>

#include "conf.h"

FtpServer ftpSrv;
AsyncWebServer server(80);

const char* PARAM_INPUT = "value";

volatile uint32_t sliderValue = 0;
uint16_t timeout = 0;
volatile uint16_t ledState = 0;

unsigned long timer_value = 0;
unsigned long wifi_reconnect_timer = 0;

void resetFunc(void) {
  ESP.restart();
}

int calculate_duty(int in) {
  return map(in, SERVER_SLIDER_MIN, SERVER_SLIDER_MAX, 0, (1UL << resolution) - 1);
}

String processor(const String& var) {
  #ifdef ENABLE_DEBUG_PRINT
    Serial.printf("Processor received string: %s\n", var.c_str());
  #endif

  if (var == "SLIDERVALUE") {
    return String(sliderValue);
  }
  return String();
}

void tryToStartAP() {
  #ifdef ENABLE_DEBUG_PRINT
    Serial.printf("Trying to start access point\n");
  #endif

  bool useOpenAP = conf.ap_password.length() == 0;
  if (!useOpenAP && conf.ap_password.length() < 8) {
    #ifdef ENABLE_DEBUG_PRINT
      Serial.printf("Password is %u chars long, WiFi requires at least 8\n", conf.ap_password.length());
    #endif
    return;
  }

  bool ok = useOpenAP ? WiFi.softAP(conf.ap_ssid.c_str()) : WiFi.softAP(conf.ap_ssid.c_str(), conf.ap_password.c_str());

  if (!ok) {
    #ifdef ENABLE_DEBUG_PRINT
      Serial.printf("WiFi AP failed to start\n");
    #endif
    return;
  }

  IPAddress IP = WiFi.softAPIP();
  #ifdef ENABLE_DEBUG_PRINT
    Serial.printf("AP IP address: %s\n", IP.toString().c_str());
  #endif
}

#ifdef ENABLE_DEBUG_PRINT
void printSPIFFSInfo() {
  Serial.printf("Total space: %u bytes\n", SPIFFS.totalBytes());
  Serial.printf("Used space: %u bytes\n", SPIFFS.usedBytes());
  Serial.printf("Free space: %u bytes\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
}
#endif

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  #ifdef ENABLE_DEBUG_PRINT
    Serial.printf("Starting backlight controller\n");
  #endif

  if (SPIFFS.begin()) {
    #ifdef ENABLE_DEBUG_PRINT
      Serial.println("SPIFFS opened");
      printSPIFFSInfo();
    #endif

    File f = SPIFFS.open("/config.json");

    if (!f) {
      #ifdef ENABLE_DEBUG_PRINT
        Serial.println("Failed to open config.json, using default config");
      #endif
    } else {
      JsonDocument jsonBuffer;
      auto err = deserializeJson(jsonBuffer, f);
      f.close();
      if (err) {
        #ifdef ENABLE_DEBUG_PRINT
          Serial.println("Parse config file failed");
        #endif
      } else {
        conf.ssid         = jsonBuffer["ssid"]         | conf.ssid;
        conf.password     = jsonBuffer["password"]     | conf.password;
        conf.ap_ssid      = jsonBuffer["ap_ssid"]      | conf.ap_ssid;
        conf.ap_password  = jsonBuffer["ap_password"]  | conf.ap_password;
        conf.hostname     = jsonBuffer["hostname"]     | conf.hostname;
        conf.ftp_username = jsonBuffer["ftp_username"] | conf.ftp_username;
        conf.ftp_password = jsonBuffer["ftp_password"] | conf.ftp_password;
        conf.ota_username = jsonBuffer["ota_username"] | conf.ota_username;
        conf.ota_password = jsonBuffer["ota_password"] | conf.ota_password;
      }
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(conf.hostname.c_str());
  WiFi.begin(conf.ssid.c_str(), conf.password.c_str());

  while (WL_CONNECTED != WiFi.status()) {
    delay(500);

    #ifdef ENABLE_DEBUG_PRINT
      Serial.printf("%d seconds left to enter the AP mode\n", (WIFI_CONNECT_TIMEOUT_S - timeout) / 2);
    #endif

    timeout++;
    if (WIFI_CONNECT_TIMEOUT_S <= (timeout * 2)) {
      #ifdef ENABLE_DEBUG_PRINT
        Serial.printf("Unsuccessfull attempt to connect WiFi\n");
      #endif

      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      tryToStartAP();
      break;
    }
  }

  #ifdef ENABLE_DEBUG_PRINT
  if (WL_CONNECTED == WiFi.status()) {
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("RSSI level is: %d\n", WiFi.RSSI());
  }
  #endif

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css");
  });

  server.on("/initial-slider-value", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(sliderValue));
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = constrain(inputMessage.toInt(), SERVER_SLIDER_MIN, SERVER_SLIDER_MAX);
      ledcWrite(ledPin, calculate_duty(sliderValue));
    }
    else {
      inputMessage = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/reset", HTTP_POST, [] (AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
    delay(20);
    resetFunc();
  });

  EEPROM.begin(EEPROM_SIZE);
  ledState = (EEPROM.read(0) << 8) | EEPROM.read(1);
  sliderValue = ledState;

  ledcAttach(ledPin, freq, resolution);
  ledcWrite(ledPin, calculate_duty(sliderValue));
  timer_value = millis();

  ElegantOTA.begin(&server);

  server.begin();

  ftpSrv.begin(conf.ftp_username.c_str(), conf.ftp_password.c_str());

  #ifdef ENABLE_DEBUG_PRINT
    Serial.println("HTTP server started");
    Serial.println("FTP server started!");
    Serial.println("PWM configuration finished!");
  #endif
}

void loop() {
  ftpSrv.handleFTP();
  ElegantOTA.loop();
  if ((millis() - timer_value) > SAVE_PWM_VALUE_TIMEOUT_S * 1000) {
    if (ledState != sliderValue) {
      ledState = sliderValue;
      EEPROM.write(0, highByte(ledState));
      EEPROM.write(1, lowByte(ledState));
      EEPROM.commit();
    }
    timer_value = millis();
  }

  if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED &&
      (millis() - wifi_reconnect_timer) > WIFI_RECONNECT_INTERVAL_S * 1000) {
    #ifdef ENABLE_DEBUG_PRINT
      Serial.println("WiFi disconnected, trying to reconnect");
    #endif
    WiFi.reconnect();
    wifi_reconnect_timer = millis();
  }
}
