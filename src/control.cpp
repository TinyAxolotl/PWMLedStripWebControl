#include <Arduino.h>
#include <atomic>
#include <stdint.h>
#include "control.h"
#include "conf.h"
#include "settings.h"

namespace Control {
namespace {
  std::atomic<uint32_t> requestedBrightness{0};

  uint32_t calculate_duty(uint32_t in) {
    return map(in, SERVER_SLIDER_MIN, SERVER_SLIDER_MAX, 0, (1UL << resolution) - 1);
  }

  void controlThread(void *arg) {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    ledcAttach(ledPin, freq, resolution);

    while(1) {
      ledcWrite(ledPin, calculate_duty(requestedBrightness.load()));
      vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_DELAY_MS));
    }
  }

  void storageThread(void *arg) {

    while(1) {
      vTaskDelay(pdMS_TO_TICKS(SAVE_PWM_VALUE_TIMEOUT_S * 1000));

      uint32_t current_setpoint = requestedBrightness.load();

      if (current_setpoint != Settings::loadBrightness()) {
          Settings::saveBrightness(current_setpoint);
      }
    }
  }

  } // anonymous namespace

  void begin() {
    Settings::begin();

    requestedBrightness.store(Settings::loadBrightness());
    xTaskCreate(controlThread, "controlThread", 4096, NULL, 2, NULL);
    xTaskCreate(storageThread, "storageThread", 2048, NULL, 1, NULL);
  }

  void setBrightness(uint32_t brightness) {
    requestedBrightness.store(brightness);
  }

  uint32_t getBrightness() {
    return requestedBrightness.load();
  }
}