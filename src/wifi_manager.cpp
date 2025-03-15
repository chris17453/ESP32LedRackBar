#include <WiFiManager.h>
#include "includes/wifi_manager.h"
#include "includes/display.h"
#include "includes/utils.h"
#include <ESPAsyncWebServer.h>

// Add to wifi_manager.cpp

WiFiSetupState wifiState = WIFI_INIT;
WiFiManager wifiManager;
unsigned long wifiOperationStartTime = 0;
unsigned long lastPortalAnimationTime = 0;


void startWiFiSetup() {
  Serial.println("Starting WiFi setup...");
  wifiState = WIFI_INIT;

  // Ensure WiFi settings are stored persistently
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  // Configure WiFiManager
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setCustomHeadElement("<style>h1 { font-size: 22px; color: white; text-align: center; }</style>");

  // Callback when portal starts
  wifiManager.setAPCallback([](WiFiManager* wm) {
      Serial.println("Started WiFi Manager Portal");
      disp.displayClear();
      
      String portalMsg = "Connect to WiFi: " + securityConfig.apName + " - Visit: 192.168.4.1";
      disp.displayClear();
      disp.setTextAlignment(PA_LEFT);
      disp.setSpeed(40);
      disp.displayText(portalMsg.c_str(), PA_LEFT, 40, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

      wifiState = WIFI_PORTAL_ACTIVE;
      wifiOperationStartTime = millis();
  });

  // Save credentials callback
  wifiManager.setSaveConfigCallback([]() {
      Serial.println("WiFi configuration saved, ensuring persistence...");

      WiFi.persistent(true); // Save credentials to flash
      WiFi.setAutoConnect(true);
      WiFi.begin(); // Apply credentials immediately

      Serial.println("Waiting 2 seconds before restarting...");
      delayWithWatchdog(2000);
  });

  // Try auto-connect before launching portal
  if (wifiManager.autoConnect(securityConfig.apName.c_str())) {
      Serial.println("Connected to WiFi!");
      wifiState = WIFI_CONNECTED;
      setupPostWiFiConnection();
      return;
  }

  Serial.println("No WiFi credentials found - starting portal");

  // If autoConnect() fails, start portal mode
  if (wifiManager.startConfigPortal(securityConfig.apName.c_str())) {
      Serial.println("Portal configuration successful!");
      wifiState = WIFI_CONNECTED;
  } else {
      Serial.println("Portal timeout - Restarting WiFi Manager");
      wifiState = WIFI_PORTAL_ACTIVE;
      wifiOperationStartTime = millis();
  }
}


void processWiFiSetup() {
    switch (wifiState) {
    case WIFI_INIT:
      // Should not happen, but just in case
      startWiFiSetup();
      break;
      
    case WIFI_CONNECTING:
      // Check if connected
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        wifiState = WIFI_CONNECTED;
        
        // Set up post-connection stuff
        setupPostWiFiConnection();
      } else if (millis() - wifiOperationStartTime > 20000) {
        // Timeout after 20 seconds of connection attempts
        Serial.println("Connection timed out, starting portal");
        
        // Start the portal in non-blocking mode
        if (wifiManager.startConfigPortal(securityConfig.apName.c_str())) {
          // This will only execute if the portal config is saved
          wifiState = WIFI_CONNECTED;
        } else {
          // Portal started, waiting for user input
          wifiState = WIFI_PORTAL_ACTIVE;
          wifiOperationStartTime = millis();
        }
      }
      break;
      
    case WIFI_PORTAL_ACTIVE:
      // Process the portal
      wifiManager.process();
      
      // Animate the display
      if (millis() - lastPortalAnimationTime > 50) {  // Control animation speed
        lastPortalAnimationTime = millis();
        if (disp.displayAnimate()) {
          disp.displayReset();
        }
      }
      
      // Check for timeout
      if (millis() - wifiOperationStartTime > 180000) {  // 3 minutes timeout
        Serial.println("Portal timed out");
        wifiState = WIFI_FAILED;
      }
      
      // Check if connected (portal was successful)
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi via portal!");
        wifiState = WIFI_CONNECTED;
        
        // Set up post-connection stuff
        setupPostWiFiConnection();
      }
      break;
      
    case WIFI_CONNECTED:
      // Already connected, nothing to do
      break;
      
    case WIFI_FAILED:
      // Already failed, nothing to do
      // You could restart here if needed
      break;
  }
}

void setupPostWiFiConnection() {
    // This contains the code that runs after a successful WiFi connection
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
  loadConfig();
}

bool isWiFiSetupComplete() {
    return (wifiState == WIFI_CONNECTED || wifiState == WIFI_FAILED);
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