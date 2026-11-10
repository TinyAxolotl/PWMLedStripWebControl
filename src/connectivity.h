#pragma once
#include "conf.h"

namespace Connectivity {
  bool begin(const backlight_conf& conf);
  void loop();
}
