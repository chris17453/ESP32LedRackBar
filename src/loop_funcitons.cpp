#include "includes/loop_functions.h"
#include "includes/globals.h"
#include "includes/wifi_manager.h"
#include "includes/api.h"
#include "includes/defaults.h"
#include <esp_task_wdt.h>

// Check system memory usage
void checkSystemMemory(int force=0) {
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 5000 || force==1) {  // Check every 5 seconds
      lastMemCheck = millis();
      Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    } 

  }
  
  // Handle system update process
  bool handleUpdateProcess() {
    if (updateInProgress) {
      // Display the updating message
      disp.displayClear();
      disp.setTextAlignment(PA_CENTER);
      disp.print("UPDATING");
      return true; // Skip the rest of the loop while updating
    }
    return false;
  }
  
  // Handle IP display mode
  bool handleIpDisplayMode() {
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
        return false;
      } else {
        // We're still in IP display mode, show the IP text
        if (disp.displayAnimate()) {
          disp.displayReset();
        }
        return true; // Skip the rest of the loop while in IP display mode
      }
    }
    return false;
  }
  
  // Check if display should be active
  bool checkDisplayActive() {
    if (!config.displayOn || config.items.empty()) {
      disp.displayClear();
      return false;
    }
    
    // Ensure we have a valid current item index
    if (config.currentItemIndex >= config.items.size()) {
      config.currentItemIndex = 0;
    }
    
    return true;
  }
  
  // Validate current item settings
  void validateCurrentItem() {
    // Get current item
    DisplayItem& currentItem = config.items[config.currentItemIndex];
    
    // If item has no duration set, give it a default duration
    if (currentItem.duration <= 0) {
      currentItem.duration = 10000;  // 10 seconds default
      Serial.println("Item had no duration, setting default 10 seconds");
    }
  }
  
  // Check if it's time to move to the next item
  bool checkForItemTransition() {
    DisplayItem& currentItem = config.items[config.currentItemIndex];
    unsigned long currentTime = millis();
    
    if (config.itemStartTime > 0 && 
        currentTime - config.itemStartTime > currentItem.duration) {
      
      unsigned long itemDuration = currentTime - config.itemStartTime;

      char buffer[100];  // Adjust size based on expected message length
      snprintf(buffer, sizeof(buffer), "Item %d duration elapsed: %dms, Target duration: %dms",
               config.currentItemIndex, itemDuration, currentItem.duration);
      Serial.println(buffer);

      return true;
    }
    return false;
  }
  
  // Process item transition - handles item switching, deletion if needed
  void processItemTransition() {
    // Get current item
    DisplayItem& currentItem = config.items[config.currentItemIndex];
    
    // Save the current mode before changing
    String oldMode = currentItem.mode;
    
    // Increment play count for the current item
    config.items[config.currentItemIndex].playCount++;
    
    // Check if the item should be deleted after reaching max plays
    if (handleItemDeletion()) {
      // Item was deleted, nothing more to do for this transition
      return;
    }
    
    // Move to the next item
    moveToNextItem();
    
    // Get the new item
    DisplayItem& newItem = config.items[config.currentItemIndex];
    
    // Handle display mode transition
    handleDisplayModeTransition(oldMode, newItem);
    /*
    // Log info about the item switch
    char buffer[128];  // Adjust size based on expected message length
    snprintf(buffer, sizeof(buffer), "Switched to item %d: Mode=%s%s",
             config.currentItemIndex, newItem.mode.c_str(),
             (newItem.mode == "text") ? (", Text='" + newItem.text + "'").c_str() : "");
    Serial.println(buffer);
    */
  }
  
  // Handle item deletion if needed
  bool handleItemDeletion() {
    DisplayItem& currentItem = config.items[config.currentItemIndex];
    bool shouldDelete = false;
    
    if (currentItem.deleteAfterPlay) {
      if (currentItem.maxPlays > 0 && 
          currentItem.playCount >= currentItem.maxPlays) {
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
        createDefaultItem();
      }
      
      return true;
    }
    
    return false;
  }
  
  // Create a default item when none exist
  void createDefaultItem() {
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
  
  // Move to the next item in the playlist
  void moveToNextItem() {
    // Move to the next item
    
    checkSystemMemory(1);
  
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
  
  // Handle display mode transition, updating settings as needed
  void handleDisplayModeTransition(const String& oldMode, DisplayItem& newItem) {
    // Clear the display for mode change if needed
    if (oldMode != newItem.mode) {
      clearDisplayForModeChange(oldMode, newItem.mode);
      
      // Reinitialize the display with the new item's settings
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
  }
  
  // Update display based on current item mode
  void updateDisplayContent() {
    DisplayItem& currentItem = config.items[config.currentItemIndex];


    if (currentItem.mode == "twinkle")     updateTwinkleEffect(currentItem);     
    if (currentItem.mode == "knightrider") updateKnightRiderEffect(currentItem); 
    if (currentItem.mode == "pong")        updatePongEffect(currentItem);        
    if (currentItem.mode == "sinewave")    updateSineWaveEffect(currentItem);    
    if (currentItem.mode == "text")        updateTextDisplay(currentItem);       
  }
  
  // Update text display mode
  void updateTextDisplay(DisplayItem& currentItem) {
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


void wifi_api_setup(){
  if (WIFI_ENABLED==true) {
    if ( !isWiFiSetupComplete()) {
      processWiFiSetup();
      return;  
    }
      
    if (!apiSetupDone) {
      setupApiEndpoints();
      apiSetupDone = true;
      Serial.println("✅ System initialization complete");
      
      if (!config.items.empty()) {
        Serial.println("Initialized with mode: " + config.items[0].mode);
      } else {
        Serial.println("No display items initialized");
      }
      updateDisplay();
    }
  }
}