#include <ESPAsyncWebServer.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>
#include <ESP8266FtpServer.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "conf.h"
#include "control.h"
#include "web.h"

namespace Web {
  namespace {
    FtpServer ftpSrv;
    AsyncWebServer server(80);
    const char* PARAM_INPUT = "value";

    void resetFunc(void) {
      ESP.restart();
    }

    String processor(const String& var) {
      #ifdef ENABLE_DEBUG_PRINT
        Serial.printf("Processor received string: %s\n", var.c_str());
      #endif

      if (var == "SLIDERVALUE") {
        return String(Control::getBrightness());
      }
      return String();
    }
  }

  void begin(const backlight_conf& conf) {
    server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/style.css");
    });

    server.on("/initial-slider-value", HTTP_ANY, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", String(Control::getBrightness()));
    });

    server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest * request) {
      String inputMessage;
      if (request->hasParam(PARAM_INPUT)) {
        inputMessage = request->getParam(PARAM_INPUT)->value();
        Control::setBrightness(constrain(inputMessage.toInt(), SERVER_SLIDER_MIN, SERVER_SLIDER_MAX));
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

    ElegantOTA.begin(&server);

    server.begin();

    ftpSrv.begin(conf.ftp_username.c_str(), conf.ftp_password.c_str());
  }

  void loop() {
    ftpSrv.handleFTP();
    ElegantOTA.loop();
  }
}

