#ifndef CONFIG_H
#define CONFIG_H

// Basic includes that don't conflict
#include <WiFi.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include "SPI.h"
#include <ESPmDNS.h>
#include <vector>

// Forward declarations for classes we'll use
class WiFiManager;
class AsyncWebServer;
class AsyncWebServerRequest;

// Hardware definitions
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 12
#define CS_PIN 5

// Config file locations
#define CONFIG_FILE "/config.json"
#define SECURITY_FILE "/security.json"

// External declarations for shared objects
extern MD_Parola disp;
extern AsyncWebServer server;
extern Preferences preferences;
extern bool updateInProgress;


// Define a structure for a single display item
struct DisplayItem {
  String mode;              // "text" or "twinkle"
  String text;              // Text content (for text mode)
  int alignment;            // Text alignment
  bool invert;              // Whether display is inverted
  int brightness;           // Display brightness (0-15)
  int scrollSpeed;          // Text scroll speed
  int pauseTime;            // Pause time at end of scroll
  unsigned long duration;   // How long to show this item (ms, 0 = forever)
  int playCount;            // Number of times item has been played
  int maxPlays;             // Maximum times to play (0 = unlimited)
  bool deleteAfterPlay;     // Whether to delete after playing


  // Twinkle effect parameters
  int twinkleDensity;       // Twinkle density (for twinkle mode)
  int twinkleMinSpeed;      // Min twinkle speed (for twinkle mode)
  int twinkleMaxSpeed;      // Max twinkle speed (for twinkle mode)

  // Knight Rider effect parameters
  int knightRiderSpeed;     // Update interval in ms
  int knightRiderTailLength; // Length of the tail
  
  // Pong effect parameters
  int pongSpeed;            // Update interval in ms
  float pongBallSpeedX;     // Horizontal ball speed
  float pongBallSpeedY;     // Vertical ball speed
  
  // Sine wave effect parameters
  int sineWaveSpeed;        // Update interval in ms
  int sineWaveAmplitude;    // Wave amplitude
  int sineWavePhases;       // Number of overlapping waves
};





// Main configuration structure with array of display items
struct DisplayConfig {
  bool displayOn;           // Global display on/off
  bool loopItems;           // Whether to loop through items
  int currentItemIndex;     // Current item being displayed
  unsigned long itemStartTime; // When current item started
  std::vector<DisplayItem> items; // Array of display items
};

// Structure for temporary IP display mode
struct TempIPConfig {
  bool active;              // Whether this temporary config is active
  String text;              // Text to display (IP address, etc)
  unsigned long startTime;  // When this mode was started
  unsigned long duration;   // How long to show this info
};

// Security configuration
struct SecurityConfig {
  String apiKey;
  String apName;
  String hostname;
};

// Global variables
extern DisplayConfig config;
extern SecurityConfig securityConfig;
extern TempIPConfig ipDisplayConfig;
extern bool textNeedsUpdate;

// Function declarations
void loadConfig();
void saveConfig();
void resetConfig();
void loadSecurityConfig();
void saveSecurityConfig();
void changeApiKey(String newKey);
void factoryReset();
void checkFactoryResetCondition();

#endif // CONFIG_H