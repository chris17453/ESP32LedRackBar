// ESP32 LED Rack Bar - Main Program File
// Main program entry point and loop

#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_task_wdt.h>


// Then include our project headers 
#include "includes/config.h"
#include "includes/defaults.h"
#include "includes/display.h"
#include "includes/effects.h"
#include "includes/wifi_manager.h"
#include "includes/api.h"
#include "includes/globals.h"     
#include "includes/loop_functions.h" 
#include "includes/utils.h"




void setup() {
  // Initialize random seed
  randomSeed(analogRead(0));

  Serial.begin(115200);
  Serial.println("\n\n--- Starting ESP32 LED Rack Bar ---");
  
  initializeEffects();
  
  // SPIFFS Setup
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS Mount Failed!");
  } else {
    Serial.println("✅ SPIFFS Mounted");
    
    // Debug: List all files in SPIFFS
    Serial.println("Listing all files in SPIFFS:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
      String fileName = file.name();
      size_t fileSize = file.size();
      Serial.print("File: ");
      Serial.print(fileName);
      Serial.print(" - Size: ");
      Serial.println(fileSize);
      file = root.openNextFile();
    }
  }

  // Load security configuration
  loadSecurityConfig();
  checkFactoryResetCondition();
  loadConfig();
  config.itemStartTime = millis();
  initDisplay();
  disp.setTextAlignment(PA_CENTER);
  disp.print("Starting");
  if(WIFI_ENABLED==true) {
    startWiFiSetup();
    wifi_api_setup();
  }
  

  Serial.println("Reset Watchdog Init");
  // Initialize watchdog not not until wifimanager is done. because it takes a while
  esp_task_wdt_init(WATCHDOG_RESET_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  
}

void loop() {
  //checkSystemMemory(0);
  esp_task_wdt_reset();
  
  if (handleUpdateProcess()) return; // Skip the rest of the loop while updating
  if (!checkDisplayActive()) return; 
  if (handleIpDisplayMode()) return;
  
  validateCurrentItem();

  if (checkForItemTransition())  processItemTransition();
  updateDisplayContent();
}