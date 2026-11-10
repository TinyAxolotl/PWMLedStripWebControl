#pragma once
#include "conf.h"

namespace Control {
  void begin();
  void setBrightness(uint32_t brightness);
  uint32_t getBrightness();
}