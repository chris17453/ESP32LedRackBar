#include "effects.h"
#include "defaults.h"
#include "display.h"

// Initialize global variables
TwinkleState twinkleStates[MAX_ACTIVE_TWINKLES];
uint8_t matrixRows = 8;
uint8_t matrixCols;

void initTwinkleStates() {
  matrixCols = MAX_DEVICES * 8;
  
  Serial.println("Initializing twinkle states array...");
  
  // Initialize all states to inactive
  for (int i = 0; i < MAX_ACTIVE_TWINKLES; i++) {
    twinkleStates[i].active = false;
    twinkleStates[i].startTime = 0;
    twinkleStates[i].duration = 0;
    twinkleStates[i].maxBrightness = 0;
    twinkleStates[i].row = 0;
    twinkleStates[i].col = 0;
  }
  
  Serial.println("✅ Twinkle states initialized successfully");
}

void updateTwinkleEffect(const DisplayItem& item) {
  if (item.mode != "twinkle") return;
  
  // Get current time
  unsigned long currentTime = millis();
  
  // Clear the display buffer but don't update the display yet
  disp.displayClear();
  
  // Cap twinkle values to reasonable ranges to prevent crashes
  int safeDensity = constrain(item.twinkleDensity, 1, 50);
  int safeMinSpeed = constrain(item.twinkleMinSpeed, 10, 1000);
  int safeMaxSpeed = constrain(item.twinkleMaxSpeed, safeMinSpeed, 2000);
  
  // 1. Start new twinkles for inactive slots
  int inactiveCount = 0;
  
  // First count how many inactive slots we have
  for (int i = 0; i < MAX_ACTIVE_TWINKLES; i++) {
    if (!twinkleStates[i].active) {
      inactiveCount++;
    }
  }
  
  // Calculate how many new twinkles to create based on density
  int newTwinkles = safeDensity / 5;  // Adjust this divisor to control density
  newTwinkles = constrain(newTwinkles, 0, inactiveCount);
  
  // Activate new twinkles in random positions
  for (int i = 0; i < newTwinkles; i++) {
    // Find an inactive slot
    int slot = -1;
    for (int j = 0; j < MAX_ACTIVE_TWINKLES; j++) {
      if (!twinkleStates[j].active) {
        slot = j;
        break;
      }
    }
    
    if (slot >= 0) {
      twinkleStates[slot].active = true;
      twinkleStates[slot].startTime = currentTime;
      // Random duration between minSpeed and maxSpeed
      twinkleStates[slot].duration = random(safeMinSpeed, safeMaxSpeed + 1);
      // Random max brightness between 5 and DEFAULT_MAX_INTENSITY
      twinkleStates[slot].maxBrightness = random(5, DEFAULT_MAX_INTENSITY + 1);
      // Random position on the matrix
      twinkleStates[slot].row = random(matrixRows);
      twinkleStates[slot].col = random(matrixCols);
    }
  }
  
  // 2. Update currently active twinkles
  for (int i = 0; i < MAX_ACTIVE_TWINKLES; i++) {
    if (twinkleStates[i].active) {
      unsigned long elapsed = currentTime - twinkleStates[i].startTime;
      
      // Check if twinkle has completed its cycle
      if (elapsed >= twinkleStates[i].duration) {
        twinkleStates[i].active = false;
        continue;
      }
      
      // Calculate current brightness based on time elapsed
      float progress = (float)elapsed / twinkleStates[i].duration;
      float brightness = 0;
      
      // Fade in and out using a sine wave
      // sin(π * progress) gives a value from 0 to 1 to 0 as progress goes from 0 to 1
      brightness = sin(PI * progress) * twinkleStates[i].maxBrightness;
      
      // Set the LED with the calculated brightness
      if (brightness > 0) {
        // Check if the row and column are valid
        if (twinkleStates[i].row < matrixRows && twinkleStates[i].col < matrixCols) {
          // Using a simulated brightness by controlling whether the LED is on
          if (random(DEFAULT_MAX_INTENSITY) < brightness) {
            disp.getGraphicObject()->setPoint(twinkleStates[i].row, twinkleStates[i].col, true);
          }
        }
      }
    }
  }
}

void safeReinitTwinkle() {
  Serial.println("Safely reinitializing twinkle...");
  
  // Initialize all states to inactive
  for (int i = 0; i < MAX_ACTIVE_TWINKLES; i++) {
    twinkleStates[i].active = false;
    twinkleStates[i].startTime = 0;
    twinkleStates[i].duration = 0;
    twinkleStates[i].maxBrightness = 0;
    twinkleStates[i].row = 0;
    twinkleStates[i].col = 0;
  }
  
  Serial.println("✅ Twinkle states safely reinitialized");
}