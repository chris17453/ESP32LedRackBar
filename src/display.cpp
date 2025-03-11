#include "display.h"
#include "defaults.h"
#include "effects.h"

// Initialize global display object
MD_Parola disp = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void initDisplay() {
  // Initialize the display
  disp.begin();
  disp.setIntensity(DEFAULT_BRIGHTNESS);
  disp.setSpeed(DEFAULT_SCROLL_SPEED);
  disp.setPause(DEFAULT_PAUSE_TIME);
  
  // Clear the display and any potential garbage
  disp.displayClear();
  
  // Clear display buffer more thoroughly
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < MAX_DEVICES * 8; col++) {
      disp.getGraphicObject()->setPoint(row, col, false);
    }
  }
  
  // Force update to ensure clean state
  disp.getGraphicObject()->update();
  
  // Small delay to ensure display is ready
  delay(50);
  
  // Show a simple "Starting" message
  disp.setTextAlignment(PA_CENTER);
  disp.print("Starting");
}

void clearDisplayForModeChange(String oldMode, String newMode) {
  // Ensure we have valid modes even if empty strings are passed
  if (oldMode.length() == 0) oldMode = "unknown";
  if (newMode.length() == 0) newMode = "text";  // Default to text mode if empty
  
  Serial.println("Mode changing from " + oldMode + " to " + newMode);
  
  // Complete reset of the display
  disp.displayClear();
  disp.setTextAlignment(PA_CENTER);  // Reset to a standard alignment
  
  // Force display refresh and wait for it to complete
  disp.displayReset();
  
  // If coming from twinkle mode, we need to do additional cleanup
  if (oldMode == "twinkle") {
    // Explicitly zero out every LED to ensure nothing is left behind
    for (uint8_t row = 0; row < matrixRows; row++) {
      for (uint8_t col = 0; col < matrixCols; col++) {
        disp.getGraphicObject()->setPoint(row, col, false);
      }
    }
    
    // Force the display to refresh completely
    disp.getGraphicObject()->clear();
    disp.getGraphicObject()->update();  // Use update() instead of refreshDisplay()
    
    // Reset all twinkle states
    for (int i = 0; i < MAX_ACTIVE_TWINKLES; i++) {
      twinkleStates[i].active = false;
    }
  }
  
  // Wait a bit longer to ensure the display is fully cleared and ready
  delay(200);
  
  // One final clear to be absolutely sure
  disp.displayClear();
  delay(50);
}

void updateDisplay() {
  // Mark that we need to update the display text on next loop
  textNeedsUpdate = true;
}

void scrollPortalAddress() {
  String portalMsg = "Connect to WiFi: " + securityConfig.apName + " - Visit: 192.168.4.1";
  
  disp.displayClear();
  disp.setTextAlignment(PA_LEFT);
  disp.setSpeed(40); // Slightly faster for setup message
  disp.displayText(portalMsg.c_str(), PA_LEFT, 40, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  
  // Keep scrolling the message while in portal mode
  // This will be interrupted when WiFi setup completes
  unsigned long startTime = millis();
  while (millis() - startTime < 60000) { // Show for up to 60 seconds
    if (disp.displayAnimate()) {
      disp.displayReset();
    }
    yield(); // Allow WiFiManager to process
  }
}

void showUpdatingMessage() {
  // Clear the current display
  disp.displayClear();
  
  // Set text alignment to center
  disp.setTextAlignment(PA_CENTER);
  
  // Set a standard brightness
  disp.setIntensity(10);
  
  // Show the message
  disp.print("UPDATING");
  
  // Force display refresh
  disp.getGraphicObject()->update();
}