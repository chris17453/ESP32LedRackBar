#include "includes/config.h"
#include "includes/defaults.h"
#include "includes/display.h"
#include "includes/utils.h"

// Initialize global variables
DisplayConfig config;
SecurityConfig securityConfig;
TempIPConfig ipDisplayConfig = {
  false,        // Not active initially
  "",           // No text initially
  0,            // No start time
  IP_DISPLAY_DURATION
};
bool textNeedsUpdate = true;

// Function implementations
void loadConfig() {
  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file || file.size() == 0) {
    Serial.println("⚠️ Config file missing or empty. Resetting...");
    resetConfig();
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("⚠️ Config file corrupted. Resetting...");
    resetConfig();
    return;
  }
  
  // Load global settings
  config.displayOn = doc["displayOn"] | true;
  config.loopItems = doc["loopItems"] | true;
  config.currentItemIndex = 0;  // Always start with the first item
  config.itemStartTime = 0;
  
  // Clear existing items
  config.items.clear();
  
  // Load items array
  if (doc["items"].is<JsonArray>()) {
    for (JsonObject itemObj : doc["items"].as<JsonArray>()) {
      DisplayItem item;
      
      // Load item settings
      item.mode = itemObj["mode"].as<String>();
      if (item.mode.length() == 0) {
        item.mode = "text";  // Default mode
      }
      
      // Load mode-specific parameters
      if (item.mode == "text") {
        item.text = itemObj["text"].as<String>();
        if (item.text.length() == 0) {
          item.text = "ESP32 LED Display";  // Default text
        }
        item.alignment = itemObj["alignment"] | PA_SCROLL_LEFT;
        item.scrollSpeed = itemObj["scrollSpeed"] | DEFAULT_SCROLL_SPEED;
        item.pauseTime = itemObj["pauseTime"] | DEFAULT_PAUSE_TIME;
      }
      else if (item.mode == "twinkle") {
        item.twinkleDensity = itemObj["twinkleDensity"] | DEFAULT_TWINKLE_DENSITY;
        item.twinkleMinSpeed = itemObj["twinkleMinSpeed"] | DEFAULT_TWINKLE_MIN_SPEED;
        item.twinkleMaxSpeed = itemObj["twinkleMaxSpeed"] | DEFAULT_TWINKLE_MAX_SPEED;
      }
      else if (item.mode == "knightrider") {
        item.knightRiderSpeed = itemObj["knightRiderSpeed"] | 50;
        item.knightRiderTailLength = itemObj["knightRiderTailLength"] | 3;
      }
      else if (item.mode == "pong") {
        item.pongSpeed = itemObj["pongSpeed"] | 100;
        item.pongBallSpeedX = itemObj["pongBallSpeedX"] | 0.5;
        item.pongBallSpeedY = itemObj["pongBallSpeedY"] | 0.25;
      }
      else if (item.mode == "sinewave") {
        item.sineWaveSpeed = itemObj["sineWaveSpeed"] | 50;
        item.sineWaveAmplitude = itemObj["sineWaveAmplitude"] | 3;
        item.sineWavePhases = itemObj["sineWavePhases"] | 3;
      }
      
      // Load common parameters
      item.invert = itemObj["invert"] | false;
      item.brightness = itemObj["brightness"] | DEFAULT_BRIGHTNESS;
      item.duration = itemObj["duration"] | 0;  // Default: show forever
      item.playCount = itemObj["playCount"] | 0;
      item.maxPlays = itemObj["maxPlays"] | 0;  // 0 = unlimited plays
      item.deleteAfterPlay = itemObj["deleteAfterPlay"] | false;
      
      // Add to items array
      config.items.push_back(item);
    }
  }
  
  // If no items were loaded, add a default item
  if (config.items.empty()) {
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
    defaultItem.duration = 0;  // Show forever
    defaultItem.playCount = 0;
    defaultItem.maxPlays = 0;
    defaultItem.deleteAfterPlay = false;
    
    config.items.push_back(defaultItem);
  }
  
  file.close();
  Serial.println("✅ Config loaded successfully!");
  Serial.println("Number of display items: " + String(config.items.size()));
}

void saveConfig() {
  File file = SPIFFS.open(CONFIG_FILE, "w");
  if (!file) {
    Serial.println("⚠️ Failed to open config file for writing!");
    return;
  }
  
  JsonDocument doc;
  
  // Save global settings
  doc["displayOn"] = config.displayOn;
  doc["loopItems"] = config.loopItems;
  
  // Create items array
  JsonArray itemsArray = doc.createNestedArray("items");
  
  // Add each item
  for (const DisplayItem& item : config.items) {
    JsonObject itemObj = itemsArray.createNestedObject();
    
    // Save common parameters
    itemObj["mode"] = item.mode;
    itemObj["invert"] = item.invert;
    itemObj["brightness"] = item.brightness;
    itemObj["duration"] = item.duration;
    itemObj["playCount"] = item.playCount;
    itemObj["maxPlays"] = item.maxPlays;
    itemObj["deleteAfterPlay"] = item.deleteAfterPlay;
    
    // Save mode-specific parameters
    if (item.mode == "text") {
      itemObj["text"] = item.text;
      itemObj["alignment"] = item.alignment;
      itemObj["scrollSpeed"] = item.scrollSpeed;
      itemObj["pauseTime"] = item.pauseTime;
    }
    else if (item.mode == "twinkle") {
      itemObj["twinkleDensity"] = item.twinkleDensity;
      itemObj["twinkleMinSpeed"] = item.twinkleMinSpeed;
      itemObj["twinkleMaxSpeed"] = item.twinkleMaxSpeed;
    }
    else if (item.mode == "knightrider") {
      itemObj["knightRiderSpeed"] = item.knightRiderSpeed;
      itemObj["knightRiderTailLength"] = item.knightRiderTailLength;
    }
    else if (item.mode == "pong") {
      itemObj["pongSpeed"] = item.pongSpeed;
      itemObj["pongBallSpeedX"] = item.pongBallSpeedX;
      itemObj["pongBallSpeedY"] = item.pongBallSpeedY;
    }
    else if (item.mode == "sinewave") {
      itemObj["sineWaveSpeed"] = item.sineWaveSpeed;
      itemObj["sineWaveAmplitude"] = item.sineWaveAmplitude;
      itemObj["sineWavePhases"] = item.sineWavePhases;
    }
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("⚠️ Failed to write to config file!");
  } else {
    Serial.println("✅ Config saved!");
  }
  
  file.close();
}

// In config.cpp, update the resetConfig function to set a default duration
void resetConfig() {
  Serial.println("⚠️ Resetting config to default...");
  
  // Clear the configuration
  config.displayOn = true;
  config.loopItems = true;
  config.currentItemIndex = 0;
  config.itemStartTime = 0;
  config.items.clear();
  
  // Add default text item
  DisplayItem textItem;
  textItem.mode = "text";
  textItem.text = "Connect to " + securityConfig.apName + " WiFi - Go to 192.168.4.1";
  textItem.alignment = PA_SCROLL_LEFT;
  textItem.invert = false;
  textItem.brightness = DEFAULT_BRIGHTNESS;
  textItem.scrollSpeed = DEFAULT_SCROLL_SPEED;
  textItem.pauseTime = DEFAULT_PAUSE_TIME;
  textItem.duration = 10000;  // 10 seconds default duration
  textItem.playCount = 0;
  textItem.maxPlays = 0;
  textItem.deleteAfterPlay = false;
  
  config.items.push_back(textItem);
  
  // Add a twinkle effect item
  DisplayItem twinkleItem;
  twinkleItem.mode = "twinkle";
  twinkleItem.invert = false;
  twinkleItem.brightness = DEFAULT_BRIGHTNESS;
  twinkleItem.twinkleDensity = DEFAULT_TWINKLE_DENSITY;
  twinkleItem.twinkleMinSpeed = DEFAULT_TWINKLE_MIN_SPEED;
  twinkleItem.twinkleMaxSpeed = DEFAULT_TWINKLE_MAX_SPEED;
  twinkleItem.duration = 5000;  // 5 seconds
  twinkleItem.playCount = 0;
  twinkleItem.maxPlays = 0;
  twinkleItem.deleteAfterPlay = false;
  
  config.items.push_back(twinkleItem);
  
  // Add a Knight Rider effect item
  DisplayItem knightRiderItem;
  knightRiderItem.mode = "knightrider";
  knightRiderItem.invert = false;
  knightRiderItem.brightness = DEFAULT_BRIGHTNESS;
  knightRiderItem.knightRiderSpeed = 50;
  knightRiderItem.knightRiderTailLength = 3;
  knightRiderItem.duration = 5000;  // 5 seconds
  knightRiderItem.playCount = 0;
  knightRiderItem.maxPlays = 0;
  knightRiderItem.deleteAfterPlay = false;
  
  config.items.push_back(knightRiderItem);
  
  saveConfig();
}

void loadSecurityConfig() {
  File file = SPIFFS.open(SECURITY_FILE, "r");
  if (!file || file.size() == 0) {
    Serial.println("⚠️ Security config file missing or empty. Using defaults...");
    securityConfig.apiKey = DEFAULT_API_KEY;
    securityConfig.apName = DEFAULT_AP_NAME;
    securityConfig.hostname = DEFAULT_HOSTNAME;
    saveSecurityConfig();
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("⚠️ Security config file corrupted. Using defaults...");
    securityConfig.apiKey = DEFAULT_API_KEY;
    securityConfig.apName = DEFAULT_AP_NAME;
    securityConfig.hostname = DEFAULT_HOSTNAME;
    saveSecurityConfig();
    return;
  }
  
  // Load security settings
  securityConfig.apiKey = doc["apiKey"] | DEFAULT_API_KEY;
  securityConfig.apName = doc["apName"] | DEFAULT_AP_NAME;
  securityConfig.hostname = doc["hostname"] | DEFAULT_HOSTNAME;
  
  file.close();
  Serial.println("✅ Security config loaded successfully!");
  Serial.println("AP Name: " + securityConfig.apName);
  Serial.println("Hostname: " + securityConfig.hostname);
}

void saveSecurityConfig() {
  File file = SPIFFS.open(SECURITY_FILE, "w");
  if (!file) {
    Serial.println("⚠️ Failed to open security config file for writing!");
    return;
  }
  
  JsonDocument doc;
  doc["apiKey"] = securityConfig.apiKey;
  doc["apName"] = securityConfig.apName;
  doc["hostname"] = securityConfig.hostname;
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("⚠️ Failed to write to security config file!");
  } else {
    Serial.println("✅ Security config saved!");
  }
  
  file.close();
}

void changeApiKey(String newKey) {
  if (newKey.length() < 8) {
    Serial.println("⚠️ API key too short (min 8 chars)");
    return;
  }
  
  securityConfig.apiKey = newKey;
  saveSecurityConfig();
  Serial.println("✅ API key updated successfully");
}
void factoryReset() {
  Serial.println("⚠️ FACTORY RESET INITIATED ⚠️");
  updateInProgress = true;
  
  // Set the factory reset flag to prevent immediate reset on next boot
  Serial.println("Setting factory reset flag...");
  bool prefSuccess = false;
  try {
    preferences.begin("powercycle", false);
    prefSuccess = preferences.putBool("fr_done", true);
    preferences.end();
    
    if (prefSuccess) {
      Serial.println("Factory reset flag set successfully");
    } else {
      Serial.println("WARNING: Failed to set factory reset flag");
    }
  } catch (const std::exception& e) {
    Serial.printf("ERROR: Exception in setting reset flag: %s\n", e.what());
  } catch (...) {
    Serial.println("ERROR: Unknown exception in setting reset flag");
  }
  
    // Reset WiFi settings with error handling and extra precautions
    Serial.println("Clearing WiFi credentials...");
    try {
      // Initialize WiFi if not already done
      WiFi.mode(WIFI_MODE_STA);
      Serial.println("WiFi set to station mode");
      delayWithWatchdog(200); // Give it more time to initialize
      
      // Perform the disconnect with longer delayWithWatchdog
      Serial.println("Attempting WiFi.disconnect()...");
      bool wifiDisconnectResult = WiFi.disconnect(true, true);
      delayWithWatchdog(500); // Allow more time for the operation to complete
      
      if (wifiDisconnectResult) {
        Serial.println("WiFi credentials cleared successfully");
      } else {
        Serial.println("WARNING: WiFi.disconnect() returned false, trying alternative methods");
        
        // Try alternative approach - force WiFi reset through NVS
        Serial.println("Attempting to clear WiFi settings through NVS...");
        
        // Try with Preferences API first (safer)
        Preferences wifiPrefs;
        if (wifiPrefs.begin("wifi", false)) {
          size_t clearedBytes = wifiPrefs.clear();
          wifiPrefs.end();
          Serial.printf("WiFi preferences cleared (%d bytes)\n", clearedBytes);
        }
        
        // If using WiFiManager, also try to clear its settings
        #ifdef USE_WIFI_MANAGER
        try {
          Serial.println("Clearing WiFiManager settings...");
          WiFiManager wifiManager;
          wifiManager.resetSettings();
          Serial.println("WiFiManager settings cleared");
        } catch (...) {
          Serial.println("Error while clearing WiFiManager settings");
        }
        #endif
        
        // Final attempt - restart WiFi subsystem
        Serial.println("Restarting WiFi subsystem...");
        WiFi.mode(WIFI_OFF);
        delayWithWatchdog(500);
        WiFi.mode(WIFI_STA);
        delayWithWatchdog(500);
      }
    } catch (...) {
      Serial.println("ERROR: Exception while clearing WiFi credentials");
    }
    
  // Try to safely clear files with detailed error handling
  Serial.println("Attempting to clear configuration files...");
  bool spiffsOk = false;
  
  try {
    spiffsOk = SPIFFS.begin(true);  // Mount SPIFFS with formatting if needed
    if (spiffsOk) {
      Serial.println("SPIFFS mounted successfully");
    } else {
      Serial.println("ERROR: SPIFFS.begin() returned false");
    }
  } catch (const std::exception& e) {
    Serial.printf("ERROR: Exception in SPIFFS.begin(): %s\n", e.what());
  } catch (...) {
    Serial.println("ERROR: Unknown exception in SPIFFS.begin()");
  }
  
  if (spiffsOk) {
    // Check if the files exist before removing
    if (SPIFFS.exists(CONFIG_FILE)) {
      Serial.printf("Found config file: %s\n", CONFIG_FILE);
      try {
        bool removeResult = SPIFFS.remove(CONFIG_FILE);
        if (removeResult) {
          Serial.printf("Successfully removed %s\n", CONFIG_FILE);
        } else {
          Serial.printf("ERROR: Failed to remove %s\n", CONFIG_FILE);
        }
      } catch (const std::exception& e) {
        Serial.printf("ERROR: Exception while removing %s: %s\n", CONFIG_FILE, e.what());
      } catch (...) {
        Serial.printf("ERROR: Unknown exception while removing %s\n", CONFIG_FILE);
      }
    } else {
      Serial.printf("WARNING: Config file %s does not exist\n", CONFIG_FILE);
    }
    
    if (SPIFFS.exists(SECURITY_FILE)) {
      Serial.printf("Found security file: %s\n", SECURITY_FILE);
      try {
        bool removeResult = SPIFFS.remove(SECURITY_FILE);
        if (removeResult) {
          Serial.printf("Successfully removed %s\n", SECURITY_FILE);
        } else {
          Serial.printf("ERROR: Failed to remove %s\n", SECURITY_FILE);
        }
      } catch (const std::exception& e) {
        Serial.printf("ERROR: Exception while removing %s: %s\n", SECURITY_FILE, e.what());
      } catch (...) {
        Serial.printf("ERROR: Unknown exception while removing %s\n", SECURITY_FILE);
      }
    } else {
      Serial.printf("WARNING: Security file %s does not exist\n", SECURITY_FILE);
    }
    
    // List remaining files to verify deletion
    Serial.println("Remaining files after deletion attempt:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    bool filesExist = false;
    
    while (file) {
      filesExist = true;
      Serial.printf("  - %s (%d bytes)\n", file.name(), file.size());
      file = root.openNextFile();
    }
    
    if (!filesExist) {
      Serial.println("  No files remaining in SPIFFS");
    }
    
    try {
      SPIFFS.end();  // Safely unmount
      Serial.println("SPIFFS unmounted successfully");
    } catch (const std::exception& e) {
      Serial.printf("ERROR: Exception in SPIFFS.end(): %s\n", e.what());
    } catch (...) {
      Serial.println("ERROR: Unknown exception in SPIFFS.end()");
    }
  }
  
  // Reset preferences with error handling
  Serial.println("Clearing all preferences...");
  try {
    preferences.begin("powercycle", false);
    size_t clearedBytes = preferences.clear();  // Clear all preferences
    preferences.end();
    Serial.printf("Preferences cleared successfully (%d bytes)\n", clearedBytes);
  } catch (const std::exception& e) {
    Serial.printf("ERROR: Exception while clearing preferences: %s\n", e.what());
  } catch (...) {
    Serial.println("ERROR: Unknown exception while clearing preferences");
  }
  
  Serial.println("Factory reset completed, waiting before restart...");
  // Give the system time to finish all operations and serial output to complete
  delayWithWatchdog(2000);
  
  Serial.println("Restarting device...");
  // Restart the device
  ESP.restart();
}

void checkFactoryResetCondition() {
  if (FACTORY_RESET_DISABLED==true) {
    Serial.println("========== Factory Reset Disabled ==========");
    return;
  }
  preferences.begin("powercycle", false);
  
  // Get stored values
  unsigned int resetCount = preferences.getUInt("count", 0);
  unsigned long firstResetTime = preferences.getULong("first_reset", 0);
  unsigned long lastResetTime = preferences.getULong("last_reset", 0);
  unsigned long currentTime = millis();
  
  // Check if this is the first boot after a factory reset
  bool wasFactoryReset = preferences.getBool("fr_done", false);
  
  Serial.println("========== BOOT SEQUENCE CHECK ==========");
  Serial.print("Reset count: ");
  Serial.print(resetCount);
  Serial.print(", First reset time: ");
  Serial.print(firstResetTime);
  Serial.print(", Last reset time: ");
  Serial.print(lastResetTime);
  Serial.print(", Current time: ");
  Serial.println(currentTime);
  
  if (wasFactoryReset) {
    // Clear the factory reset flag and reset counters
    Serial.println("✅ First boot after factory reset - skipping reset detection");
    preferences.putBool("fr_done", false);
    preferences.putUInt("count", 0);
    preferences.putULong("first_reset", 0);
    preferences.putULong("last_reset", currentTime);
    preferences.end();
    return;
  }
  
  // Get the real-time clock time if available (more reliable than millis)
  unsigned long realTimeSinceLastReset;
  if (lastResetTime == 0) {
    // This is the first recorded boot
    realTimeSinceLastReset = RESET_WINDOW_MS + 1; // Set to something larger than window
    Serial.println("First recorded boot - no previous timestamp");
  } else {
    // On ESP32, we can use more reliable time sources if available
    // For this example, we'll use millis() but in a real implementation
    // you might want to use RTC time or other persistent time source
    
    // Calculate time between now and the last reset
    // Since millis() resets on reboot, we're actually checking how long 
    // the system has been up since the reboot
    realTimeSinceLastReset = currentTime;
    
    Serial.print("System uptime since reboot: ");
    Serial.print(realTimeSinceLastReset);
    Serial.println("ms");
  }
  
  // If we've been up less than RESET_WINDOW_MS, it's a quick boot
  if (realTimeSinceLastReset < RESET_WINDOW_MS) {
    // This is a quick reboot
    
    if (resetCount == 0) {
      // First quick reboot in the sequence
      resetCount = 1;
      firstResetTime = currentTime; // Mark the start of our reset sequence
      Serial.println("👉 First quick reboot detected. Count = 1");
    } else {
      // We're continuing a sequence of quick reboots
      resetCount++;
      Serial.print("👉 Quick reboot sequence continues. Count = ");
      Serial.println(resetCount);
      
      // Calculate total time from first quick reboot to now
      unsigned long totalSequenceTime;
      if (currentTime >= firstResetTime) {
        totalSequenceTime = currentTime - firstResetTime;
      } else {
        // Handle millis() rollover or anomaly
        totalSequenceTime = RESET_WINDOW_MS * 3; // Force out of range
      }
      
      Serial.print("Total time for ");
      Serial.print(resetCount);
      Serial.print(" reboots: ");
      Serial.print(totalSequenceTime);
      Serial.println("ms");
      
      // Check if the entire sequence is within our window
      // For 3 quick reboots, they should all happen within 
      // approximately 3 * RESET_WINDOW_MS total time
      if (resetCount >= RESET_COUNT_THRESHOLD) {
        if (totalSequenceTime < (RESET_WINDOW_MS * 3)) {
          // Reset counter
          Serial.println("⚠️ FACTORY RESET TRIGGERED - 3 quick reboots detected within window");
          preferences.putUInt("count", 0);
          preferences.putULong("first_reset", 0);
          preferences.putULong("last_reset", 0);
          // Set the factory reset flag to true to skip detection on next boot
          preferences.putBool("fr_done", true);
          preferences.end();
          
          // Perform factory reset
          factoryReset();
          // factoryReset will restart the device, so we won't get past this point
          return;
        } else {
          // We had 3 reboots but they took too long overall
          Serial.println("⚠️ 3 reboots detected, but total sequence time exceeded window");
          // Reset the sequence
          resetCount = 1;
          firstResetTime = currentTime;
        }
      }
    }
  } else {
    // System has been up too long - this is a normal boot
    // Reset the sequence counters
    resetCount = 0;
    firstResetTime = 0;
    Serial.println("Normal boot (system up too long). Reset sequence cleared.");
  }
  
  // Update reset information
  preferences.putUInt("count", resetCount);
  preferences.putULong("first_reset", firstResetTime);
  preferences.putULong("last_reset", currentTime);
  preferences.end();
  Serial.println("Reset detection complete - continuing normal boot");
  Serial.println("==========================================");
}