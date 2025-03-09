#include "api.h"
#include "defaults.h"
#include "wifi_manager.h"
#include "config.h"
#include "effects.h"
#include "display.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Initialize web server on port 80
AsyncWebServer server(80);

void setupApiEndpoints() {
  Serial.println("Setting up API endpoints...");
  
  // Debug endpoint - no authentication needed
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head><title>ESP32 LED Matrix</title></head>";
    html += "<body style='font-family: Arial, sans-serif; margin: 20px;'>";
    html += "<h1>ESP32 LED Matrix</h1>";
    html += "<p>Status: Running</p>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Hostname: " + securityConfig.hostname + "</p>";
    html += "<p>WiFi SSID: " + WiFi.SSID() + "</p>";
    html += "<p>Signal Strength: " + String(WiFi.RSSI()) + " dBm</p>";
    html += "<p>Access the API with your API key for full control.</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });
  // Settings API endpoints
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    JsonDocument doc;
    doc["displayOn"] = config.displayOn;
    doc["loopItems"] = config.loopItems;
    doc["currentItemIndex"] = config.currentItemIndex;
    
    JsonArray itemsArray = doc.createNestedArray("items");
    
    for (const DisplayItem& item : config.items) {
      JsonObject itemObj = itemsArray.createNestedObject();
      
      itemObj["mode"] = item.mode;
      itemObj["text"] = item.text;
      itemObj["alignment"] = item.alignment == PA_LEFT ? "left" : 
                            (item.alignment == PA_RIGHT ? "right" : 
                            (item.alignment == PA_CENTER ? "center" : 
                            (item.alignment == PA_SCROLL_LEFT ? "scroll_left" : "scroll_right")));
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
    
    String response;
    serializeJson(doc, response);
    Serial.println("✅ Settings requested via API");
    request->send(200, "application/json", response);
  });

  server.on("/items", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    JsonDocument doc;
    JsonArray itemsArray = doc.createNestedArray("items");
    
    for (const DisplayItem& item : config.items) {
      JsonObject itemObj = itemsArray.createNestedObject();
      
      itemObj["mode"] = item.mode;
      itemObj["text"] = item.text;
      itemObj["alignment"] = item.alignment == PA_LEFT ? "left" : 
                            (item.alignment == PA_RIGHT ? "right" : 
                            (item.alignment == PA_CENTER ? "center" : 
                            (item.alignment == PA_SCROLL_LEFT ? "scroll_left" : "scroll_right")));
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
    
    String response;
    serializeJson(doc, response);
    Serial.println("✅ Items requested via API");
    request->send(200, "application/json", response);
  });

  server.on("/items", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    DisplayItem newItem;
    
    // Parse the item details
    if (doc["mode"].is<String>()) {
      newItem.mode = doc["mode"].as<String>();
    } else {
      newItem.mode = "text";
    }
 
    // Ensure there's a duration
    if (doc["duration"].is<int>()) {
      newItem.duration = doc["duration"].as<int>();
    } else {
      newItem.duration = 10000;  // Default 10 seconds if not specified
    }
    
    if (doc["text"].is<String>()) {
      newItem.text = doc["text"].as<String>();
    } else {
      newItem.text = "New Item";
    }
    
    if (doc["alignment"].is<String>()) {
      String alignment = doc["alignment"].as<String>();
      if (alignment == "left") {
        newItem.alignment = PA_LEFT;
      } else if (alignment == "right") {
        newItem.alignment = PA_RIGHT;
      } else if (alignment == "center") {
        newItem.alignment = PA_CENTER;
      } else if (alignment == "scroll_left") {
        newItem.alignment = PA_SCROLL_LEFT;
      } else if (alignment == "scroll_right") {
        newItem.alignment = PA_SCROLL_RIGHT;
      } else {
        newItem.alignment = PA_SCROLL_LEFT;
      }
    } else {
      newItem.alignment = PA_SCROLL_LEFT;
    }
    
    newItem.invert = doc["invert"] | false;
    newItem.brightness = doc["brightness"] | DEFAULT_BRIGHTNESS;
    newItem.scrollSpeed = doc["scrollSpeed"] | DEFAULT_SCROLL_SPEED;
    newItem.pauseTime = doc["pauseTime"] | DEFAULT_PAUSE_TIME;
    newItem.twinkleDensity = doc["twinkleDensity"] | DEFAULT_TWINKLE_DENSITY;
    newItem.twinkleMinSpeed = doc["twinkleMinSpeed"] | DEFAULT_TWINKLE_MIN_SPEED;
    newItem.twinkleMaxSpeed = doc["twinkleMaxSpeed"] | DEFAULT_TWINKLE_MAX_SPEED;
    newItem.duration = doc["duration"] | 0;
    newItem.playCount = doc["playCount"] | 0;
    newItem.maxPlays = doc["maxPlays"] | 0;
    newItem.deleteAfterPlay = doc["deleteAfterPlay"] | false;
    
    // Add the new item
    config.items.push_back(newItem);
    
    // Save the config
    saveConfig();
    
    // Send response
    JsonDocument responseDoc;
    responseDoc["status"] = "success";
    responseDoc["message"] = "Item added successfully";
    responseDoc["index"] = config.items.size() - 1;
    
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
  });
  
  // Update or replace all items
  server.on("/items/replace", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Check if we have an items array
    if (!doc["items"].is<JsonArray>()) {
      request->send(400, "application/json", "{\"error\":\"items array is required\"}");
      return;
    }
    
    // Clear existing items
    config.items.clear();
    
    // Add new items
    for (JsonObject itemObj : doc["items"].as<JsonArray>()) {
      DisplayItem item;
      
      item.mode = itemObj["mode"] | "text";
      item.text = itemObj["text"] | "New Item";
      
      String alignment = itemObj["alignment"] | "scroll_left";
      if (alignment == "left") {
        item.alignment = PA_LEFT;
      } else if (alignment == "right") {
        item.alignment = PA_RIGHT;
      } else if (alignment == "center") {
        item.alignment = PA_CENTER;
      } else if (alignment == "scroll_left") {
        item.alignment = PA_SCROLL_LEFT;
      } else if (alignment == "scroll_right") {
        item.alignment = PA_SCROLL_RIGHT;
      } else {
        item.alignment = PA_SCROLL_LEFT;
      }
      
      item.invert = itemObj["invert"] | false;
      item.brightness = itemObj["brightness"] | DEFAULT_BRIGHTNESS;
      item.scrollSpeed = itemObj["scrollSpeed"] | DEFAULT_SCROLL_SPEED;
      item.pauseTime = itemObj["pauseTime"] | DEFAULT_PAUSE_TIME;
      item.twinkleDensity = itemObj["twinkleDensity"] | DEFAULT_TWINKLE_DENSITY;
      item.twinkleMinSpeed = itemObj["twinkleMinSpeed"] | DEFAULT_TWINKLE_MIN_SPEED;
      item.twinkleMaxSpeed = itemObj["twinkleMaxSpeed"] | DEFAULT_TWINKLE_MAX_SPEED;
      item.duration = itemObj["duration"] | 0;
      item.playCount = itemObj["playCount"] | 0;
      item.maxPlays = itemObj["maxPlays"] | 0;
      item.deleteAfterPlay = itemObj["deleteAfterPlay"] | false;
      
      config.items.push_back(item);
    }
    
    // If no items were added, add a default one
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
      defaultItem.duration = 0;
      defaultItem.playCount = 0;
      defaultItem.maxPlays = 0;
      defaultItem.deleteAfterPlay = false;
      
      config.items.push_back(defaultItem);
    }
    
    // Reset to the first item
    config.currentItemIndex = 0;
    config.itemStartTime = 0;
    textNeedsUpdate = true;
    
    // Save the config
    saveConfig();
    
    // Send response
    JsonDocument responseDoc;
    responseDoc["status"] = "success";
    responseDoc["message"] = "Items replaced successfully";
    responseDoc["count"] = config.items.size();
    
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
  });
  
  // Delete a specific item
  server.on("/items/delete", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Check if index parameter is provided
    if (!doc["index"].is<int>()) {
      request->send(400, "application/json", "{\"error\":\"index parameter is required\"}");
      return;
    }
    
    int itemIndex = doc["index"].as<int>();
    
    // Validate index
    if (itemIndex < 0 || itemIndex >= config.items.size()) {
      request->send(400, "application/json", "{\"error\":\"Invalid item index\"}");
      return;
    }
    
    // Delete the item
    config.items.erase(config.items.begin() + itemIndex);
    
    // If we deleted the current item or an item before it, adjust current index
    if (itemIndex <= config.currentItemIndex) {
      if (config.currentItemIndex > 0) {
        config.currentItemIndex--;
      }
    }
    
    // If we deleted all items, add a default one
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
      defaultItem.duration = 0;
      defaultItem.playCount = 0;
      defaultItem.maxPlays = 0;
      defaultItem.deleteAfterPlay = false;
      
      config.items.push_back(defaultItem);
      config.currentItemIndex = 0;
    }
    
    // Force update
    textNeedsUpdate = true;
    config.itemStartTime = 0;
    
    // Save the config
    saveConfig();
    
    // Send response
    JsonDocument responseDoc;
    responseDoc["status"] = "success";
    responseDoc["message"] = "Item deleted successfully";
    responseDoc["remaining"] = config.items.size();
    
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
  });
  
  // Security settings endpoint
  server.on("/security", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key - this is a sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    JsonDocument doc;
    doc["apName"] = securityConfig.apName;
    doc["hostname"] = securityConfig.hostname;
    // Don't send the actual API key, just acknowledgment it exists
    doc["apiKeySet"] = true;
    
    String response;
    serializeJson(doc, response);
    Serial.println("✅ Security settings requested via API");
    request->send(200, "application/json", response);
  });
  
  // Update security settings endpoint
  server.on("/security", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key - this is a sensitive operation
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    bool configChanged = false;
    
    if (doc["apName"].is<String>() && doc["apName"].as<String>().length() > 0) {
      securityConfig.apName = doc["apName"].as<String>();
      configChanged = true;
    }
    
    if (doc["hostname"].is<String>() && doc["hostname"].as<String>().length() > 0) {
      securityConfig.hostname = doc["hostname"].as<String>();
      configChanged = true;
    }
    
    if (doc["apiKey"].is<String>() && doc["apiKey"].as<String>().length() >= 8) {
      securityConfig.apiKey = doc["apiKey"].as<String>();
      configChanged = true;
    }
    
    if (configChanged) {
      saveSecurityConfig();
      Serial.println("✅ Security settings updated via API");
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Security settings updated\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"No valid settings provided\"}");
    }
  });
  
  // Get specific setting
// Get specific setting
server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Validate API key
  if (!validateApiKey(request)) {
    request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
    return;
  }
  
  String paramName = "";
  String response = "{";
  bool paramFound = false;
  
  // Check if a specific parameter was requested
  if (request->hasParam("param")) {
    paramName = request->getParam("param")->value();
    paramFound = true;
    
    if (paramName == "displayOn") {
      response += "\"displayOn\":" + String(config.displayOn ? "true" : "false");
    } else if (paramName == "loopItems") {
      response += "\"loopItems\":" + String(config.loopItems ? "true" : "false");
    } else if (paramName == "currentItemIndex") {
      response += "\"currentItemIndex\":" + String(config.currentItemIndex);
    } else if (paramName == "numItems") {
      response += "\"numItems\":" + String(config.items.size());
    } else if (paramName == "apName") {
      response += "\"apName\":\"" + securityConfig.apName + "\"";
    } else if (paramName == "hostname") {
      response += "\"hostname\":\"" + securityConfig.hostname + "\"";
    } else if (config.items.size() > 0 && config.currentItemIndex < config.items.size()) {
      // Get parameters from the current item
      DisplayItem& currentItem = config.items[config.currentItemIndex];
      
      if (paramName == "mode") {
        response += "\"mode\":\"" + currentItem.mode + "\"";
      } else if (paramName == "text") {
        response += "\"text\":\"" + currentItem.text + "\"";
      } else if (paramName == "alignment") {
        String align = currentItem.alignment == PA_LEFT ? "left" : 
                      (currentItem.alignment == PA_RIGHT ? "right" : 
                      (currentItem.alignment == PA_CENTER ? "center" : 
                      (currentItem.alignment == PA_SCROLL_LEFT ? "scroll_left" : "scroll_right")));
        response += "\"alignment\":\"" + align + "\"";
      } else if (paramName == "invert") {
        response += "\"invert\":" + String(currentItem.invert ? "true" : "false");
      } else if (paramName == "brightness") {
        response += "\"brightness\":" + String(currentItem.brightness);
      } else if (paramName == "scrollSpeed") {
        response += "\"scrollSpeed\":" + String(currentItem.scrollSpeed);
      } else if (paramName == "pauseTime") {
        response += "\"pauseTime\":" + String(currentItem.pauseTime);
      } else if (paramName == "twinkleDensity") {
        response += "\"twinkleDensity\":" + String(currentItem.twinkleDensity);
      } else if (paramName == "twinkleMinSpeed") {
        response += "\"twinkleMinSpeed\":" + String(currentItem.twinkleMinSpeed);
      } else if (paramName == "twinkleMaxSpeed") {
        response += "\"twinkleMaxSpeed\":" + String(currentItem.twinkleMaxSpeed);
      } else if (paramName == "duration") {
        response += "\"duration\":" + String(currentItem.duration);
      } else if (paramName == "playCount") {
        response += "\"playCount\":" + String(currentItem.playCount);
      } else if (paramName == "maxPlays") {
        response += "\"maxPlays\":" + String(currentItem.maxPlays);
      } else if (paramName == "deleteAfterPlay") {
        response += "\"deleteAfterPlay\":" + String(currentItem.deleteAfterPlay ? "true" : "false");
      } else {
        paramFound = false;
      }
    } else {
      paramFound = false;
    }
  }
  
  // If no valid parameter was specified, return an error
  if (!paramFound) {
    response = "{\"error\":\"Invalid parameter. Available parameters: displayOn, loopItems, currentItemIndex, numItems, mode, text, alignment, invert, brightness, scrollSpeed, pauseTime, twinkleDensity, twinkleMinSpeed, twinkleMaxSpeed, duration, playCount, maxPlays, deleteAfterPlay, apName, hostname\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  response += "}";
  Serial.println("✅ Parameter '" + paramName + "' requested via API");
  request->send(200, "application/json", response);
});
  // Status endpoint
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    // No API key required for status endpoint
    JsonDocument doc;
    doc["status"] = "online";
    doc["ip"] = WiFi.localIP().toString();
    doc["hostname"] = securityConfig.hostname;
    doc["version"] = "1.0";
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Factory reset endpoint
  server.on("/factory_reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key - this is a sensitive operation
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Factory reset initiated\"}");
    // Slight delay to allow response to be sent
    delay(500);
    factoryReset();
  });
  
  // API key change endpoint
  server.on("/change_api_key", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Must validate with current API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    if (!doc["new_key"].is<String>()) {
      request->send(400, "application/json", "{\"error\":\"new_key parameter required\"}");
      return;
    }
    
    String newKey = doc["new_key"].as<String>();
    if (newKey.length() < 8) {
      request->send(400, "application/json", "{\"error\":\"API key must be at least 8 characters\"}");
      return;
    }
    
    changeApiKey(newKey);
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"API key updated\"}");
  });

  server.on("/update_display", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }
  
      bool configChanged = false;
      
      // Global settings
      if (doc["displayOn"].is<bool>()) {
        config.displayOn = doc["displayOn"].as<bool>();
        configChanged = true;
      }
      
      if (doc["loopItems"].is<bool>()) {
        config.loopItems = doc["loopItems"].as<bool>();
        configChanged = true;
      }
      
      // Check if there are any items
      if (config.items.empty()) {
        // Add a default item
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
        config.currentItemIndex = 0;
      }
      
      // Ensure valid current item index
      if (config.currentItemIndex >= config.items.size()) {
        config.currentItemIndex = 0;
      }
      
      // Get reference to current item
      DisplayItem& currentItem = config.items[config.currentItemIndex];
      
      String oldMode = currentItem.mode;
  
      // Check for each parameter and update if present
      if (doc["mode"].is<String>()) {
        String newMode = doc["mode"].as<String>();
        // Only clear display if mode is actually changing
        if (oldMode != newMode) {
          clearDisplayForModeChange(oldMode, newMode);
          currentItem.mode = newMode;
          // Force a complete restart of the display
          disp.begin();
          disp.setIntensity(currentItem.brightness);
          disp.setSpeed(currentItem.scrollSpeed);
          disp.setPause(currentItem.pauseTime);
        }
        configChanged = true;
      }
  
      if (doc["text"].is<String>()) {
        currentItem.text = doc["text"].as<String>();
        configChanged = true;
      }
  
      if (doc["alignment"].is<String>()) {
        String alignment = doc["alignment"].as<String>();
        if (alignment == "left") {
          currentItem.alignment = PA_LEFT;
        } else if (alignment == "right") {
          currentItem.alignment = PA_RIGHT;
        } else if (alignment == "center") {
          currentItem.alignment = PA_CENTER;
        } else if (alignment == "scroll_left") {
          currentItem.alignment = PA_SCROLL_LEFT;
        } else if (alignment == "scroll_right") {
          currentItem.alignment = PA_SCROLL_RIGHT;
        }
        configChanged = true;
        Serial.println("Alignment changed to: " + alignment);
      }
  
      if (doc["invert"].is<bool>()) {
        currentItem.invert = doc["invert"].as<bool>();
        configChanged = true;
      }
  
      if (doc["brightness"].is<int>()) {
        currentItem.brightness = doc["brightness"].as<int>();
        disp.setIntensity(currentItem.brightness);
        configChanged = true;
      }
  
      if (doc["scrollSpeed"].is<int>()) {
        currentItem.scrollSpeed = doc["scrollSpeed"].as<int>();
        disp.setSpeed(currentItem.scrollSpeed);
        configChanged = true;
      }
  
      if (doc["pauseTime"].is<int>()) {
        currentItem.pauseTime = doc["pauseTime"].as<int>();
        disp.setPause(currentItem.pauseTime);
        configChanged = true;
      }
      
      if (doc["twinkleDensity"].is<int>()) {
        // Constrain the value to prevent crashes
        currentItem.twinkleDensity = constrain(doc["twinkleDensity"].as<int>(), 1, 50);
        configChanged = true;
      }
      
      if (doc["twinkleMinSpeed"].is<int>()) {
        // Constrain min speed to reasonable values
        currentItem.twinkleMinSpeed = constrain(doc["twinkleMinSpeed"].as<int>(), 10, 1000);
        configChanged = true;
      }
      
      if (doc["twinkleMaxSpeed"].is<int>()) {
        // Make sure max speed is always >= min speed
        int minSpeed = currentItem.twinkleMinSpeed;
        currentItem.twinkleMaxSpeed = constrain(doc["twinkleMaxSpeed"].as<int>(), minSpeed, 2000);
        configChanged = true;
      }
      
      if (doc["duration"].is<int>()) {
        currentItem.duration = doc["duration"].as<int>();
        configChanged = true;
      }
      
      if (doc["maxPlays"].is<int>()) {
        currentItem.maxPlays = doc["maxPlays"].as<int>();
        configChanged = true;
      }
      
      if (doc["deleteAfterPlay"].is<bool>()) {
        currentItem.deleteAfterPlay = doc["deleteAfterPlay"].as<bool>();
        configChanged = true;
      }
      
      if (configChanged) {
        Serial.println("✅ Display settings updated via API");
        Serial.println("Mode: " + currentItem.mode);
        if (currentItem.mode == "text") {
          Serial.println("Text: " + currentItem.text);
        }
        saveConfig();
        
        // Signal that display needs to be updated
        textNeedsUpdate = true;
        
        // If switching to twinkle mode, initialize new twinkle states
        if (currentItem.mode == "twinkle") {
          // Use our safe reinitialization function
          safeReinitTwinkle();
        }
      }
      
      request->send(200, "application/json", "{\"status\":\"success\"}");
    });

  // Download config file endpoint
  server.on("/download_config", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    // Try to open the file
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file || file.size() == 0) {
      request->send(404, "application/json", "{\"error\":\"Config file not found or empty\"}");
      return;
    }
    
    // Send the file as a download
    request->send(SPIFFS, CONFIG_FILE, "application/json", true);
    Serial.println("✅ Config file downloaded via API");
  });
  
  // Download security config file endpoint
  server.on("/download_security_config", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    // Try to open the file
    File file = SPIFFS.open(SECURITY_FILE, "r");
    if (!file || file.size() == 0) {
      request->send(404, "application/json", "{\"error\":\"Security config file not found or empty\"}");
      return;
    }
    
    // Send the file as a download
    request->send(SPIFFS, SECURITY_FILE, "application/json", true);
    Serial.println("✅ Security config file downloaded via API");
  });
  
  // List all files in SPIFFS
  server.on("/list_files", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    JsonDocument doc;
    JsonArray files = doc.createNestedArray("files");
    
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
      JsonObject fileInfo = files.createNestedObject();
      fileInfo["name"] = String(file.name());
      fileInfo["size"] = file.size();
      file = root.openNextFile();
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    Serial.println("✅ File list requested via API");
  });
  
  // Add a manual factory reset endpoint
  server.on("/manual_factory_reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    // Send success response first so client gets it before we reset
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Manual factory reset initiated\"}");
    
    // Short delay to allow response to be sent
    delay(500);
    
    // Perform a simplified factory reset
    Serial.println("⚠️ MANUAL FACTORY RESET INITIATED ⚠️");
    
    // Reset WiFi settings first (most important part)
    WiFi.disconnect(true, true);
    Serial.println("WiFi credentials cleared");
    
    // Restart the device
    ESP.restart();
  });


  // Update WiFi settings endpoint
  server.on("/update_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Check if required parameters are provided
    if (!doc["ssid"].is<String>() || !doc["password"].is<String>()) {
      request->send(400, "application/json", "{\"error\":\"SSID and password are required\"}");
      return;
    }
    
    String ssid = doc["ssid"].as<String>();
    String password = doc["password"].as<String>();
    
    // Validate inputs
    if (ssid.length() == 0) {
      request->send(400, "application/json", "{\"error\":\"SSID cannot be empty\"}");
      return;
    }
    
    if (password.length() < 8 && password.length() != 0) {
      request->send(400, "application/json", "{\"error\":\"Password must be at least 8 characters or empty for open networks\"}");
      return;
    }
    
    // Send response before attempting to reconnect
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"WiFi settings updated, reconnecting...\"}");
    
    // Wait a moment to ensure response is sent
    delay(500);
    
    // Configure the WiFi with new credentials
    WiFi.disconnect();
    delay(1000);
    
    // Store the credentials in WiFi (they will be automatically used on restart)
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for a connection attempt
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Connected to WiFi with new credentials");
      Serial.println("IP Address: " + WiFi.localIP().toString());
      
      // Set up the temporary IP display
      ipDisplayConfig.active = true;
      ipDisplayConfig.text = "WiFi: " + ssid + " - IP: " + WiFi.localIP().toString();
      ipDisplayConfig.startTime = millis();
      
      // Update display with the new connection info
      disp.displayClear();
      disp.setTextAlignment(PA_LEFT);
      disp.setSpeed(40);  // Slightly faster for IP message
      disp.displayText(ipDisplayConfig.text.c_str(), PA_LEFT, 40, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    } else {
      Serial.println("\n❌ Failed to connect with new credentials");
      Serial.println("Will revert to Access Point mode on next restart");
    }
  });

  // Update hostname endpoint
  server.on("/update_hostname", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key for this sensitive endpoint
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Check if hostname parameter is provided
    if (!doc["hostname"].is<String>()) {
      request->send(400, "application/json", "{\"error\":\"hostname parameter is required\"}");
      return;
    }
    
    String newHostname = doc["hostname"].as<String>();
    
    // Validate hostname (alphanumeric + hyphen, max 32 chars)
    if (newHostname.length() == 0 || newHostname.length() > 32) {
      request->send(400, "application/json", "{\"error\":\"Hostname must be between 1 and 32 characters\"}");
      return;
    }
    
    for (size_t i = 0; i < newHostname.length(); i++) {
      char c = newHostname.charAt(i);
      if (!(isalnum(c) || c == '-')) {
        request->send(400, "application/json", "{\"error\":\"Hostname must contain only alphanumeric characters and hyphens\"}");
        return;
      }
    }
    
    // Update hostname in the config
    securityConfig.hostname = newHostname;
    saveSecurityConfig();
    
    // Update mDNS
    if (MDNS.begin(newHostname.c_str())) {
      Serial.println("✅ mDNS responder restarted with new hostname: " + newHostname);
    } else {
      Serial.println("⚠️ Failed to restart mDNS responder with new hostname");
    }
    
    // Send success response
    JsonDocument responseDoc;
    responseDoc["status"] = "success";
    responseDoc["message"] = "Hostname updated to " + newHostname;
    responseDoc["hostname"] = newHostname;
    
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
    
    Serial.println("✅ Hostname updated to: " + newHostname);
  });

  // Reboot device endpoint
  server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Validate API key for this operation
    if (!validateApiKey(request)) {
      request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
      return;
    }
    
    // Send success response before rebooting
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Device is rebooting\"}");
    
    // Short delay to allow response to be sent
    delay(500);
    
    // Display reboot message
    disp.displayClear();
    disp.setTextAlignment(PA_CENTER);
    disp.print("REBOOTING");
    
    // Log reboot
    Serial.println("⚠️ Device reboot initiated via API");
    
    // Small delay to show the message
    delay(1000);
    
    // Restart the device
    ESP.restart();
  });

// Add this to api.cpp in the setupApiEndpoints() function
server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Validate API key
  if (!validateApiKey(request)) {
    request->send(401, "application/json", "{\"error\":\"Unauthorized. Valid API key required.\"}");
    return;
  }
  
  JsonDocument doc;
  doc["currentTime"] = millis();
  doc["currentItemIndex"] = config.currentItemIndex;
  doc["itemStartTime"] = config.itemStartTime;
  
  if (config.items.size() > 0 && config.currentItemIndex < config.items.size()) {
    DisplayItem& currentItem = config.items[config.currentItemIndex];
    doc["currentItemDuration"] = currentItem.duration;
    doc["timeElapsed"] = millis() - config.itemStartTime;
    doc["timeRemaining"] = (config.itemStartTime + currentItem.duration) - millis();
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
});  
  // Make sure to begin the server at the end of setup
  Serial.println("Starting web server on port 80...");
  server.begin();
  Serial.println("✅ Web server started");
}