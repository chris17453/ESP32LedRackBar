#include <Arduino.h>
#include <esp_task_wdt.h>
#include "includes/utils.h"

// Function to delay while periodically resetting the watchdog
void delayWithWatchdog(unsigned long delayTime) {
    unsigned long startTime = millis();
    while (millis() - startTime < delayTime) {
      esp_task_wdt_reset();
      delay(50); // Small delay between watchdog resets
    }
  }