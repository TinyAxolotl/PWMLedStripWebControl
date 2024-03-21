# ESP32 Backlight controller

## Introduction:

The ESP32 Backlight Controller is a device designed to enhance workspace illumination for nighttime work sessions. Built with convenience and comfort in mind, it provides users with adjustable brightness control for COB LED-strips, ensuring optimal lighting conditions without causing strain on the eyes.

## Project Overview:

The ESP32 Backlight Controller offers a streamlined solution for individuals who prefer working during late hours but find traditional lighting too harsh. By seamlessly integrating Wi-Fi connectivity and a range of features including FTP server support, asynchronous web server functionality, and WireGuard compatibility, users can easily customize and control their workspace lighting to suit their preferences. With its intuitive setup process and OTA firmware update capabilities, the device offers both convenience and versatility, making it an ideal addition to any nighttime workspace.

### Supported features:
- FTP-server to load web-page content and .json configuration;
- Asynchronous web-server provide ability to keep connection from everyone in the same net;
- WireGuard support;
- PWM 15 KHz;
- Save previous state of brightness (Every 300 seconds);
- OTA firmware update;
- Configuration over JSON (which are put via FTP).

## How to install:

0. Prerequesites: ArduinoJson, NTPClient, WireGuard-ESP32, AsyncTCP, ESPAsyncWebServer, AsyncElegantOTA, ESP8266FtpServer;

1. Prepare your config.json:
```
{
    "ssid": "your_wifi_ssid",
    "password": "your_wifi_password",
    "ap_ssid": "access_point_ssid",
    "ap_password": "access_point_password",
    "hostname": "your_hostname",
    "private_key": "your_private_key",
    "local_ip": "your_local_ip",
    "public_key": "your_public_key",
    "ep_addr_ip": "your_ep_addr_ip",
    "ap_port": 8080,
    "ftp_username": "your_string_username",
    "ftp_password": "your_string_password",
    "login_name": "your_login_name",
    "login_pass": "your_login_password"
  }
```

JSON fields description:
    ssid - Name of your home Wi-Fi access point;
    password - Password of your home Wi-Fi access point;
    ap_ssid - SSID of ESP32 AP in case if connection to home Wi-Fi unsuccessfull;
    ap_password - Password of ESP32's AP;
    hostname - ESP32 hostname;
    private_key - Wireguard private key;
    local_ip - ESP32's wireguard-local IP;
    public_key - Wireguard public key;
    ep_addr_ip - Endpoint IP address (IP your WG-installed machine);
    ap_port - Endpoint port (WG);
    ftp_username - FTP username;
    ftp_password - FTP password;
    login_name - 
    login_pass - 

2. Compile and download firmware to your ESP32;
3. Assemble device;
4. Via Wi-fi your Wi-Fi router interface or via VCP (COM-port in Arduino IDE) check ESP32 Wi-Fi IP addr;
5. Connect to ESP32 via WinSCP, FileZilla or any other ftp-service, please use FTP port 22,
   enter your ftp_username and ftp_password. Drop .html, .css, .json files to ESP32;
6. Enter IP address to browser's address bar from a device in one network with ESP32;
7. (optional step) It would be better if you could provide DHCP-reservation of ESP32 IP address in your Wi-fi router;
8. Use the device.

In case if you need to update firmware, you may use OTA update. Just enter <ESP-32-IP-address>/update, drop your file there and wait untill OTA will finish.

## Schematic:

TBD

