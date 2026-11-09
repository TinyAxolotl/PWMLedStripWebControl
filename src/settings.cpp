#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
#include "settings.h"
#include "conf.h"


namespace Settings {
  void begin() {
    EEPROM.begin(EEPROM_SIZE);
  }

  void saveBrightness(uint32_t brightness) {
    EEPROM.write(0, highByte(brightness));
    EEPROM.write(1, lowByte(brightness));
    EEPROM.commit();
  }

  uint32_t loadBrightness() {
    uint32_t brightness = (uint32_t(EEPROM.read(0)) << 8) | EEPROM.read(1);

    return brightness <= SERVER_SLIDER_MAX ? brightness : SERVER_SLIDER_MIN;
  }
}
