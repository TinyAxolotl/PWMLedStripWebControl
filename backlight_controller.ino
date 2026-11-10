#include <Arduino.h>

#include "src/conf.h"
#include "src/control.h"
#include "src/config.h"
#include "src/connectivity.h"
#include "src/web.h"

void setup() {
  backlight_conf conf;

  Serial.begin(115200);

  Control::begin();
  Config::loadConfigs(conf);
  Connectivity::begin(conf);
  Web::begin(conf);
}

void loop() {
  Web::loop();
  Connectivity::loop();
}
