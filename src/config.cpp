#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <String>

#include "config.h"
#include "conf.h"

namespace Config {
  namespace {
    #ifdef ENABLE_DEBUG_PRINT
    void printSPIFFSInfo() {
      Serial.printf("Total space: %u bytes\n", SPIFFS.totalBytes());
      Serial.printf("Used space: %u bytes\n", SPIFFS.usedBytes());
      Serial.printf("Free space: %u bytes\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
    }
    #endif
  }

  void loadConfigs(backlight_conf& conf) {
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
  }
}
