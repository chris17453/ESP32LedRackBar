#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "config.h"

// Forward declaration
class AsyncWebServerRequest;

// Function declarations
void setupWiFi();
bool validateApiKey(AsyncWebServerRequest *request);

#endif // WIFI_MANAGER_H