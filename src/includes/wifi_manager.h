#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "config.h"

// Add to wifi_manager.h
enum WiFiSetupState {
    WIFI_INIT,
    WIFI_CONNECTING,
    WIFI_PORTAL_ACTIVE,
    WIFI_CONNECTED,
    WIFI_FAILED
  };
// Forward declaration
class AsyncWebServerRequest;
  
extern WiFiSetupState wifiState;
extern WiFiManager wifiManager;
extern unsigned long wifiOperationStartTime;


void startWiFiSetup();
void processWiFiSetup();
void setupPostWiFiConnection();
bool isWiFiSetupComplete();
bool validateApiKey(AsyncWebServerRequest *request);


  

#endif // WIFI_MANAGER_H