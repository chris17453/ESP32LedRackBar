// ESP32 LED Rack Bar - Main Program File
// Main program entry point and loop

#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


// Then include our project headers 
#include "config.h"
#include "defaults.h"
#include "display.h"
#include "effects.h"
#include "wifi_manager.h"
#include "api.h"

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  Serial.println("\n\n--- Starting ESP32 LED Rack Bar ---");
  
  // Initialize random seed
  randomSeed(analogRead(0));

  // Initialize twinkle states array
  initTwinkleStates();

  
  // ✅ SPIFFS Setup
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
  Serial.println("Using API Key: " + securityConfig.apiKey);
  Serial.println("AP Name: " + securityConfig.apName);
  
  // Check for factory reset condition
  checkFactoryResetCondition();

  // Load or create default config
  loadConfig();
  config.itemStartTime = millis();

  // Initialize the LED display
  initDisplay();
  
  // Show a simple fixed message first
  disp.setTextAlignment(PA_CENTER);
  disp.print("Starting");
  delay(1000);
  
  // Setup WiFi
  setupWiFi();
  
  // Setup API endpoints
  setupApiEndpoints();
  
  Serial.println("✅ System initialization complete");
  
  // Initialize the display with the saved settings
  if (!config.items.empty()) {
    Serial.println("Initialized with mode: " + config.items[0].mode);
  } else {
    Serial.println("No display items initialized");
  }
  updateDisplay();
}

void loop() {
  // First check if we're in IP display mode
  if (ipDisplayConfig.active) {
    // Check if the IP display time has elapsed
    if (millis() - ipDisplayConfig.startTime > ipDisplayConfig.duration) {
      // Time to switch to the user's configured mode
      ipDisplayConfig.active = false;
      Serial.println("IP display timeout - switching to user config");
      
      // Clear the display for a clean transition
      disp.displayClear();
      disp.displayReset();
      
      // Force update of the display with user settings
      textNeedsUpdate = true;
      // Initialize the start time for the first item
      config.itemStartTime = millis();
    } else {
      // We're still in IP display mode, show the IP text
      if (disp.displayAnimate()) {
        disp.displayReset();
      }
      
      // Skip the rest of the loop while in IP display mode
      return;
    }
  }
  
  // Regular display operation
  if (!config.displayOn || config.items.empty()) {
    disp.displayClear();
    return;
  }
  
  // Ensure we have a valid current item index
  if (config.currentItemIndex >= config.items.size()) {
    config.currentItemIndex = 0;
  }
  
  // Get current item
  DisplayItem& currentItem = config.items[config.currentItemIndex];
  
  // If item has no duration set, give it a default duration
  if (currentItem.duration <= 0) {
    currentItem.duration = 10000;  // 10 seconds default
    Serial.println("Item had no duration, setting default 10 seconds");
  }

  // Check if it's time to move to the next item
  unsigned long currentTime = millis();
  if (config.itemStartTime > 0 && 
      currentTime - config.itemStartTime > currentItem.duration) {
    
    unsigned long itemDuration = currentTime - config.itemStartTime;
    Serial.println("Item " + String(config.currentItemIndex) + 
                   " duration elapsed: " + String(itemDuration) + "ms, " +
                   "Target duration: " + String(currentItem.duration) + "ms");

    
    // Save the current mode before changing
    String oldMode = currentItem.mode;
    
    // Increment play count for the current item
    config.items[config.currentItemIndex].playCount++;
    
    // Check if the item should be deleted after reaching max plays
    bool shouldDelete = false;
    if (config.items[config.currentItemIndex].deleteAfterPlay) {
      if (config.items[config.currentItemIndex].maxPlays > 0 && 
          config.items[config.currentItemIndex].playCount >= config.items[config.currentItemIndex].maxPlays) {
        shouldDelete = true;
      }
    }
    
    if (shouldDelete) {
      Serial.println("Deleting item after reaching max plays");
      
      // Delete the current item
      config.items.erase(config.items.begin() + config.currentItemIndex);
      
      // Don't increment the index since we've removed an item
      // But make sure we're not out of bounds
      if (config.currentItemIndex >= config.items.size()) {
        config.currentItemIndex = 0;
      }
      
      // Save the updated config
      saveConfig();
      
      // Check if we have any items left
      if (config.items.empty()) {
        Serial.println("No items left after deletion");
        
        // Add a default item so we always have something to display
        DisplayItem defaultItem;
        defaultItem.mode = "text";
        defaultItem.text = "ESP32 LED Display";
        defaultItem.alignment = PA_SCROLL_LEFT;
        defaultItem.invert = false;
        defaultItem.brightness = DEFAULT_BRIGHTNESS;
        defaultItem.scrollSpeed = DEFAULT_SCROLL_SPEED;
        defaultItem.pauseTime = DEFAULT_PAUSE_TIME;
        defaultItem.twinkleDensity = DEFAULT_TWINKLE_DENSITY;
        defaultItem.twinkleMinSpeed = DEFAULT_TWINKLE_MIN_SPEED;
        defaultItem.twinkleMaxSpeed = DEFAULT_TWINKLE_MAX_SPEED;
        defaultItem.duration = 0;
        defaultItem.playCount = 0;
        defaultItem.maxPlays = 0;
        defaultItem.deleteAfterPlay = false;
        
        config.items.push_back(defaultItem);
        saveConfig();
      }
    } else {
      // Move to the next item
      config.currentItemIndex++;
      
      // Loop back to the beginning if needed
      if (config.currentItemIndex >= config.items.size()) {
        if (config.loopItems) {
          config.currentItemIndex = 0;
        } else {
          config.currentItemIndex = config.items.size() - 1;  // Stay on last item
        }
      }
    }
    
    // Get the new item
    DisplayItem& newItem = config.items[config.currentItemIndex];
    
    // Clear the display for mode change if needed
    if (oldMode != newItem.mode) {
      clearDisplayForModeChange(oldMode, newItem.mode);
      
      // Reinitialize the display with the new item's settings
      disp.begin();
      disp.setIntensity(newItem.brightness);
      disp.setSpeed(newItem.scrollSpeed);
      disp.setPause(newItem.pauseTime);
    } else {
      // Even if same mode, update settings as they might be different
      disp.setIntensity(newItem.brightness);
      disp.setSpeed(newItem.scrollSpeed);
      disp.setPause(newItem.pauseTime);
    }
    
    // Force update with the new item
    textNeedsUpdate = true;
    config.itemStartTime = millis();
    
    // Log info about the item switch
    Serial.println("Switched to item " + String(config.currentItemIndex) + 
                  ": Mode=" + newItem.mode + 
                  (newItem.mode == "text" ? ", Text='" + newItem.text + "'" : ""));
  }
  
  // Handle the current display item based on its mode
  if (currentItem.mode == "twinkle") {
    // Twinkle light effect
    updateTwinkleEffect(currentItem);
  }
  else {
    // Text display modes
    if (textNeedsUpdate) {
      disp.displayClear();
      disp.setInvert(currentItem.invert);
      disp.setIntensity(currentItem.brightness);
      disp.setSpeed(currentItem.scrollSpeed);
      disp.setPause(currentItem.pauseTime);
      
      // Handle text based on alignment mode
      if (currentItem.alignment == PA_SCROLL_LEFT) {
        disp.displayText(currentItem.text.c_str(), PA_LEFT, currentItem.scrollSpeed, 
                         currentItem.pauseTime, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        textNeedsUpdate = false;
      } 
      else if (currentItem.alignment == PA_SCROLL_RIGHT) {
        disp.displayText(currentItem.text.c_str(), PA_RIGHT, currentItem.scrollSpeed, 
                         currentItem.pauseTime, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
        textNeedsUpdate = false;
      }
      // For static text (LEFT, CENTER, RIGHT)
      else {
        disp.setTextAlignment((textPosition_t)currentItem.alignment);
        
        // If text is longer than display, use scrolling instead
        if (currentItem.text.length() > MAX_DEVICES * 8 / 6) { 
          Serial.println("Text too long for static display, using scroll instead");
          disp.displayText(currentItem.text.c_str(),(textPosition_t) currentItem.alignment, 
                          currentItem.scrollSpeed, currentItem.pauseTime, 
                          PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        } else {
          disp.print(currentItem.text.c_str());
        }
        textNeedsUpdate = false;
      }
      
      // Set the start time for duration tracking if not already set
      if (config.itemStartTime == 0) {
        config.itemStartTime = millis();
      }
    }
    
    // Animate if needed (only required for scrolling effects)
    if (currentItem.alignment == PA_SCROLL_LEFT || 
        currentItem.alignment == PA_SCROLL_RIGHT || 
        (currentItem.text.length() > MAX_DEVICES * 8 / 6)) {
      if (disp.displayAnimate()) {
        // Animation has finished, restart
        disp.displayReset();
      }
    }
  }
}