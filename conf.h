#pragma once

//#define ENABLE_DEBUG_PRINT        1

#define EEPROM_SIZE               2
#define WIFI_CONNECT_TIMEOUT_S    300
#define SAVE_PWM_VALUE_TIMEOUT_S  300

#define SERVER_SLIDER_MIN         0
#define SERVER_SLIDER_MAX         4095

const int ledPin      = 15;
const int freq        = 15000;
const int ledChannel  = 0;
const int resolution  = 12;

struct backlight_conf {
  String ssid         = "wifi_ssid";
  String password     = "wifi_passworf";
  String ap_ssid      = "ap_ssid";
  String ap_password  = "ap_password";
  String hostname     = "hostname";
  String ftp_username = "user";
  String ftp_password = "1111";
};

backlight_conf conf;
