#include <Arduino.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include <WireGuard-ESP32.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESP8266FtpServer.h>
#include <EEPROM.h>

#define EEPROM_SIZE 2
#define WIFI_CONNECT_TIMEOUT_S 300
#define SAVE_PWM_VALUE_TIMEOUT_S 300

static WireGuard wg;
FtpServer ftpSrv;
AsyncWebServer server(80);
File data;

String ssid = "ssid";
String password = "password";
String ap_ssid = "ap_ssid";
String ap_password = "ap_password";
String hostname = "hostname";

String private_key = "ppk";
String local_ip = "10.7.0.13";
String public_key = "pk";
String endpoint_address = "ep_ip";
int endpoint_port = 8080;
String ftp_username = "ftp_user";
String ftp_password = "ftp_pass";

String sliderValue = "0";
const char* PARAM_INPUT = "value";

const int ledPin = 15;
const int freq = 15000;
const int ledChannel = 0;
const int resolution = 12;

uint8_t timeout = 0;
uint16_t ledState = 0;
char *json_data;

uint64_t timer_value = 0;

void(* resetFunc) (void) = 0; // Функция перезагрузки. Присоединить на кнопку.

String processor(const String& var) {
  Serial.printf("Processor received string: %s\n", var);
  if (var == "SLIDERVALUE") {
    return sliderValue;
  }
  return String();
}

void tryToStartAP() {
  printf("Trying to start access point\n");
  WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  Serial.printf("Starting backlight controller\n");

  if (SPIFFS.begin(true)) { // Format SPIFFS if failed to mount
    Serial.println("SPIFFS opened!");
    data = SPIFFS.open("/config.json");
    Serial.printf("FILE: %s, size: %d bytes\n", data.name(), data.size());
    json_data = (char*) calloc(data.size(), sizeof(char*));
    Serial.println("Config file contents:");
    for (int i = 0; i < data.size(); i++) {
      json_data[i] = data.read();
      Serial.write(json_data[i]);
    }

    Serial.println("Applying configs");
    uint16_t json_buffer_size = data.size() * 0.2 + data.size(); // size * 20% + size
    DynamicJsonDocument jsonBuffer(json_buffer_size);
    auto err = deserializeJson(jsonBuffer, json_data);
    if (err) {
      Serial.println("Parse config file failed");
      while (true);
    } else {
      ssid = jsonBuffer["ssid"].as<const char*>();
      password = jsonBuffer["password"].as<const char*>();
      ap_ssid = jsonBuffer["ap_ssid"].as<const char*>();
      ap_password = jsonBuffer["ap_password"].as<const char*>();
      hostname = jsonBuffer["hostname"].as<const char*>();
      private_key = jsonBuffer["private_key"].as<const char*>();
      local_ip = jsonBuffer["local_ip"].as<const char*>();
      public_key = jsonBuffer["public_key"].as<const char*>();
      endpoint_address = jsonBuffer["ep_addr_ip"].as<const char*>();
      endpoint_port = jsonBuffer["ap_port"];
      ftp_username = jsonBuffer["ftp_username"].as<const char*>();
      ftp_password = jsonBuffer["ftp_password"].as<const char*>();
    }
  }

  Serial.printf("Attempt to connect to wifi\n");
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WL_CONNECTED != WiFi.status()) {
    delay(500);
    Serial.printf("%d seconds left to enter the AP mode\n", (WIFI_CONNECT_TIMEOUT_S - timeout) / 2);
    timeout++;
    if (WIFI_CONNECT_TIMEOUT_S <= timeout) {
      printf("Unsuccessfull attempt to connect WiFi\n");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      tryToStartAP();
      break;
    }
  }

  if (WL_CONNECTED == WiFi.status()) {
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("RSSI level is: %d\n", WiFi.RSSI());
  }

  configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

  /*if ( !wg.begin(local_ip.c_str(), private_key.c_str(), endpoint_address.c_str(), public_key.c_str(), endpoint_port) ) {
    Serial.println("Failed to initialize WG interface");
    } else {
    Serial.println("Successfully connected to WG!");
    Serial.print("My WG IP is: ");
    Serial.println(local_ip);
    }*/

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css");
  });

  server.on("/initial-slider-value", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", sliderValue);
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      ledcWrite(ledChannel, sliderValue.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");
  ftpSrv.begin(ftp_username, ftp_password);
  Serial.println("FTP server started!");

  Serial.println("PWM configuration finished!");
  EEPROM.begin(EEPROM_SIZE);
  ledState = (EEPROM.read(0) << 8) | EEPROM.read(1);
  sliderValue = String(ledState);

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, sliderValue.toInt());
  Serial.printf("Saved PWM status is: %d\n", ledState);
  timer_value = millis();
}

void loop() {
  ftpSrv.handleFTP();
  if ((millis() - timer_value) > SAVE_PWM_VALUE_TIMEOUT_S * 1000) {
    Serial.println("Checking PWM status");
    if (ledState != sliderValue.toInt()) {
      ledState = sliderValue.toInt();
      Serial.printf("Going to save PWM %d value to EEPROM!\n", ledState);
      EEPROM.write(0, highByte(ledState));
      EEPROM.write(1, lowByte(ledState));
      EEPROM.commit();
    }
    timer_value = millis();
  }
}
