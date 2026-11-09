#pragma once

//#define ENABLE_DEBUG_PRINT        1

#define EEPROM_SIZE               2
#define WIFI_CONNECT_TIMEOUT_S    300
#define WIFI_RECONNECT_INTERVAL_S 30
#define SAVE_PWM_VALUE_TIMEOUT_S  300

#define SERVER_SLIDER_MIN         0
#define SERVER_SLIDER_MAX         4095

#define CONTROL_TASK_DELAY_MS     200

const int ledPin      = 18;
const int freq        = 10000;
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
  String ota_username = "";
  String ota_password = "";
};
