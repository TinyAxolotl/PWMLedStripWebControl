#include <WiFi.h>

#include "connectivity.h"
#include "conf.h"

namespace Connectivity {
  namespace {
    unsigned long wifi_reconnect_timer = 0;


    bool tryToStartAP(const backlight_conf& conf) {
      #ifdef ENABLE_DEBUG_PRINT
        Serial.printf("Trying to start access point\n");
      #endif

      bool useOpenAP = conf.ap_password.length() == 0;
      if (!useOpenAP && conf.ap_password.length() < 8) {
        #ifdef ENABLE_DEBUG_PRINT
          Serial.printf("Password is %u chars long, WiFi requires at least 8\n", conf.ap_password.length());
        #endif
        return false;
      }

      bool ok = useOpenAP ? WiFi.softAP(conf.ap_ssid.c_str()) : WiFi.softAP(conf.ap_ssid.c_str(), conf.ap_password.c_str());

      if (!ok) {
        #ifdef ENABLE_DEBUG_PRINT
          Serial.printf("WiFi AP failed to start\n");
        #endif
        return false;
      }

      IPAddress IP = WiFi.softAPIP();
      #ifdef ENABLE_DEBUG_PRINT
        Serial.printf("AP IP address: %s\n", IP.toString().c_str());
      #endif

      return true;
    }
  }

  bool begin(const backlight_conf& conf) {
    bool ret = true;
    uint16_t timeout = 0;

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
        ret = tryToStartAP(conf);
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

    return ret;
  }

  void loop() {
    if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED &&
      (millis() - wifi_reconnect_timer) > WIFI_RECONNECT_INTERVAL_S * 1000) {
    #ifdef ENABLE_DEBUG_PRINT
      Serial.println("WiFi disconnected, trying to reconnect");
    #endif
    WiFi.reconnect();
    wifi_reconnect_timer = millis();
    }
  }
}
