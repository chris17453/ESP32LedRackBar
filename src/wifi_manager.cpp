#include <WiFiManager.h>
#include "wifi_manager.h"
#include "display.h"
#include <ESPAsyncWebServer.h>

void setupWiFi() {
  // WiFi Manager Setup
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  wm.setCustomHeadElement("<style>h1 { font-size: 22px; color: white; text-align: center; }</style>");
  wm.setAPCallback([](WiFiManager* wm) {
    Serial.println("Started WiFi Manager Portal");
    disp.displayClear();
    
    // Start scrolling the portal address
    scrollPortalAddress();
  });

  // Additional debugging before autoConnect
  Serial.println("Attempting to connect to WiFi...");
  Serial.println("AP Name: " + securityConfig.apName);
  Serial.println("If connection fails, connect to this AP to configure WiFi");
  
  // Clear any previous connection issues
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(1000);

  // Try to connect to WiFi
  bool connected = wm.autoConnect(securityConfig.apName.c_str());
  
  if (!connected) {
    Serial.println("WiFi Failed, restarting...");
    // Wait a bit before restarting
    delay(3000);
    ESP.restart();
  }
  
  // Connected to WiFi - Display connection details
  String ip = WiFi.localIP().toString();
  String ssid = WiFi.SSID();
  String hostname = securityConfig.hostname;
  
  Serial.println("============= CONNECTION INFO =============");
  Serial.println("✅ Connected to WiFi: " + ssid);
  Serial.println("✅ IP Address: " + ip);
  Serial.println("✅ MAC Address: " + WiFi.macAddress());
  Serial.println("✅ Hostname: " + hostname + ".local");
  Serial.println("✅ Signal Strength: " + String(WiFi.RSSI()) + " dBm");
  Serial.println("✅ Web Interface: http://" + ip);
  Serial.println("===========================================");
  
  // Start mDNS responder
  if (MDNS.begin(hostname.c_str())) {
    Serial.println("✅ mDNS responder started. You can access at http://" + hostname + ".local");
  } else {
    Serial.println("⚠️ mDNS responder failed to start");
  }
  
  // Set up the temporary IP display
  ipDisplayConfig.active = true;
  ipDisplayConfig.text = "WiFi: " + ssid + " - IP: " + ip;
  ipDisplayConfig.startTime = millis();
  
  // Clear the display for a fresh start
  disp.displayClear();
  disp.setTextAlignment(PA_LEFT);
  disp.setSpeed(40);  // Slightly faster for IP message
  disp.displayText(ipDisplayConfig.text.c_str(), PA_LEFT, 40, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  
  // Load the user's actual config in the background
  // (will be used after IP display timeout)
  loadConfig();
}

bool validateApiKey(AsyncWebServerRequest *request) {
  // Check for API key in header
  if (request->hasHeader("X-API-Key")) {
    String providedKey = request->header("X-API-Key");
    if (providedKey == securityConfig.apiKey) {
      return true;
    }
  }
  
  // Check for API key in query parameter
  if (request->hasParam("api_key")) {
    String providedKey = request->getParam("api_key")->value();
    if (providedKey == securityConfig.apiKey) {
      return true;
    }
  }
  
  // No valid API key found
  return false;
}