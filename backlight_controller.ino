#include <Arduino.h>
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

#define WIFI_CONNECT_TIMEOUT_S 60

static WireGuard wg;
FtpServer ftpSrv;
AsyncWebServer server(80);

const char* ssid = "HA3BA";
const char* password = "NKPTRT141d";
const char* ap_ssid = "PC_backlight";
const char* ap_password = "19216812Zz";
const String hostname = "PC_backlight";

char private_key[] = "ppk";
IPAddress local_ip(10, 7, 0, 13);
char public_key[] = "pk";
char endpoint_address[] = "ep_ip";
int endpoint_port = port;

uint8_t timeout = 0;

void(* resetFunc) (void) = 0; // Функция перезагрузки. Присоединить на кнопку.

void tryToStartAP() {
  printf("Trying to start access point\n");
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setup() {
  Serial.begin(115200);
  Serial.printf("Starting backlight controller\n");

  Serial.printf("Attempt to connect to wifi\n");
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid, password);

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

  if ( !wg.begin(local_ip, private_key, endpoint_address, public_key, endpoint_port) ) {
    Serial.println("Failed to initialize WG interface");
  } else {
    Serial.println("Successfully connected to WG!");
    Serial.print("My WG IP is: ");
    Serial.println(local_ip);
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");

  if (SPIFFS.begin(true)) { // Format SPIFFS if failed to mount
      Serial.println("SPIFFS opened!");
      ftpSrv.begin("ftp_username", "ftp_password");
  }
}

void loop() {
  ftpSrv.handleFTP();
}
