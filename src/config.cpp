#include "config.h"
#include "defaults.h"
#include "display.h"

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
      
      item.text = itemObj["text"].as<String>();
      if (item.text.length() == 0 && item.mode == "text") {
        item.text = "ESP32 LED Display";  // Default text
      }
      
      item.alignment = itemObj["alignment"] | PA_SCROLL_LEFT;
      item.invert = itemObj["invert"] | false;
      item.brightness = itemObj["brightness"] | DEFAULT_BRIGHTNESS;
      item.scrollSpeed = itemObj["scrollSpeed"] | DEFAULT_SCROLL_SPEED;
      item.pauseTime = itemObj["pauseTime"] | DEFAULT_PAUSE_TIME;
      item.twinkleDensity = itemObj["twinkleDensity"] | DEFAULT_TWINKLE_DENSITY;
      item.twinkleMinSpeed = itemObj["twinkleMinSpeed"] | DEFAULT_TWINKLE_MIN_SPEED;
      item.twinkleMaxSpeed = itemObj["twinkleMaxSpeed"] | DEFAULT_TWINKLE_MAX_SPEED;
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
    
    itemObj["mode"] = item.mode;
    itemObj["text"] = item.text;
    itemObj["alignment"] = item.alignment;
    itemObj["invert"] = item.invert;
    itemObj["brightness"] = item.brightness;
    itemObj["scrollSpeed"] = item.scrollSpeed;
    itemObj["pauseTime"] = item.pauseTime;
    itemObj["twinkleDensity"] = item.twinkleDensity;
    itemObj["twinkleMinSpeed"] = item.twinkleMinSpeed;
    itemObj["twinkleMaxSpeed"] = item.twinkleMaxSpeed;
    itemObj["duration"] = item.duration;
    itemObj["playCount"] = item.playCount;
    itemObj["maxPlays"] = item.maxPlays;
    itemObj["deleteAfterPlay"] = item.deleteAfterPlay;
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
  
  // Add default item
  DisplayItem defaultItem;
  defaultItem.mode = "text";
  defaultItem.text = "Connect to " + securityConfig.apName + " WiFi - Go to 192.168.4.1";
  defaultItem.alignment = PA_SCROLL_LEFT;
  defaultItem.invert = false;
  defaultItem.brightness = DEFAULT_BRIGHTNESS;
  defaultItem.scrollSpeed = DEFAULT_SCROLL_SPEED;
  defaultItem.pauseTime = DEFAULT_PAUSE_TIME;
  defaultItem.twinkleDensity = DEFAULT_TWINKLE_DENSITY;
  defaultItem.twinkleMinSpeed = DEFAULT_TWINKLE_MIN_SPEED;
  defaultItem.twinkleMaxSpeed = DEFAULT_TWINKLE_MAX_SPEED;
  defaultItem.duration = 10000;  // 10 seconds default duration
  defaultItem.playCount = 0;
  defaultItem.maxPlays = 0;
  defaultItem.deleteAfterPlay = false;
  
  config.items.push_back(defaultItem);
  
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
  
  // Set the factory reset flag to prevent immediate reset on next boot
  preferences.begin("powercycle", false);
  preferences.putBool("factory_reset_done", true);
  preferences.end();
  
  // Display a simple reset message
  disp.displayClear();
  disp.print("RESET");
  
  // Reset WiFi settings first (this is most important)
  WiFi.disconnect(true, true);
  Serial.println("WiFi credentials cleared");
  
  // Try to safely clear files with error handling
  bool spiffsOk = false;
  try {
    spiffsOk = SPIFFS.begin(true);  // Mount SPIFFS with formatting if needed
  } catch (...) {
    Serial.println("SPIFFS begin failed");
  }
  
  if (spiffsOk) {
    try {
      SPIFFS.remove(CONFIG_FILE);
      SPIFFS.remove(SECURITY_FILE);
    } catch (...) {
      Serial.println("File removal failed");
    }
    
    try {
      SPIFFS.end();  // Safely unmount
    } catch (...) {
      Serial.println("SPIFFS end failed");
    }
  }
  
  // Reset preferences with error handling
  try {
    preferences.begin("powercycle", false);
    preferences.clear();  // Clear all preferences
    preferences.end();
  } catch (...) {
    Serial.println("Preferences clear failed");
  }
  
  // Give the system time to finish all operations
  delay(2000);
  
  // Restart the device
  ESP.restart();
}

void checkFactoryResetCondition() {
  preferences.begin("powercycle", false);
  unsigned int resetCount = preferences.getUInt("count", 0);
  unsigned long lastResetTime = preferences.getULong("timestamp", 0);
  unsigned long currentTime = millis();
  
  // Check if this is the first boot after a factory reset
  bool wasFactoryReset = preferences.getBool("factory_reset_done", false);
  
  if (wasFactoryReset) {
    // Clear the factory reset flag and reset counters
    preferences.putBool("factory_reset_done", false);
    preferences.putUInt("count", 0);
    preferences.putULong("timestamp", 0);
    preferences.end();
    
    Serial.println("✅ First boot after factory reset - skipping reset detection");
    return;
  }
  
  Serial.println("Power cycle count: " + String(resetCount) + ", Last reset: " + String(lastResetTime) + "ms ago");
  
  // If we're within the reset window, increment counter
  if (currentTime < RESET_WINDOW_MS && lastResetTime > 0) {
    resetCount++;
    Serial.println("Quick reboot detected! Count now: " + String(resetCount));
    
    // Check if we've hit the threshold
    if (resetCount >= RESET_COUNT_THRESHOLD) {
      // Reset counter
      preferences.putUInt("count", 0);
      preferences.putULong("timestamp", 0);
      // Set the factory reset flag to true to skip detection on next boot
      preferences.putBool("factory_reset_done", true);
      preferences.end();
      
      // Perform factory reset
      factoryReset();
      // factoryReset will restart the device, so we won't get past this point
    }
  } else {
    // Reset count if outside window
    resetCount = 1;
    Serial.println("Normal boot or outside reset window. Reset count: " + String(resetCount));
  }
  
  // Update reset information
  preferences.putUInt("count", resetCount);
  preferences.putULong("timestamp", currentTime);
  preferences.end();
}