#include "includes/effects.h"
#include "includes/defaults.h"
#include "includes/display.h"

// Initialize global variables
TwinkleState twinkleStates[MAX_ACTIVE_TWINKLES];
PongState pongState;
SineWaveState sineWaveState;
KnightRiderState knightRiderState;

uint8_t matrixRows = 8;
uint8_t matrixCols=MAX_DEVICES * 8;



void initTwinkleStates() {
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


void initKnightRiderState() {
  Serial.println("Initializing Knight Rider effect...");
  knightRiderState.position = 0;
  knightRiderState.direction = 1;
  knightRiderState.lastUpdateTime = 0;
  knightRiderState.updateInterval = 50; // ms between updates
  knightRiderState.tailLength = 8;      // how many LEDs in the "tail"
  Serial.println("✅ Knight Rider effect initialized successfully");
}

void updateKnightRiderEffect(const DisplayItem& item) {
  if (item.mode != "knightrider") return;
  
  unsigned long currentTime = millis();
  
  // Only update at specified intervals
  if (currentTime - knightRiderState.lastUpdateTime < knightRiderState.updateInterval) {
    return;
  }
  
  // Clear the display buffer
  disp.displayClear();
  
  // Update position
  knightRiderState.position += knightRiderState.direction;
  
  // Check if we need to change direction
  if (knightRiderState.position >= matrixCols - 1) {
    knightRiderState.direction = -1;
    knightRiderState.position = matrixCols - 1;
  } else if (knightRiderState.position <= 0) {
    knightRiderState.direction = 1;
    knightRiderState.position = 0;
  }
  
  // Draw the "eye" and its tail
  for (int i = 0; i < knightRiderState.tailLength; i++) {
    int pos = knightRiderState.position - (i * knightRiderState.direction);
    
    // Check if position is within the matrix
    if (pos >= 0 && pos < matrixCols) {
      // Progressively dimmer tail (only middle row lit for a line effect)
      int row = matrixRows / 2;
      // For the main point
      if (i == 0) {
        disp.getGraphicObject()->setPoint(row, pos, true);
      } 
      // For the tail, we randomly turn on based on distance
      else if (random(i + 1) == 0) {
        disp.getGraphicObject()->setPoint(row, pos, true);
      }
    }
  }
  
  knightRiderState.lastUpdateTime = currentTime;
}

void initPongState() {
  Serial.println("Initializing Pong effect...");
  pongState.x = matrixCols / 2;
  pongState.y = matrixRows / 2;
  pongState.speedX = 0.5;  // Slower speed for smoother movement
  pongState.speedY = 0.25;
  pongState.lastUpdateTime = 0;
  pongState.updateInterval = 100; // ms between updates
  Serial.println("✅ Pong effect initialized successfully");
}

void updatePongEffect(const DisplayItem& item) {
  if (item.mode != "pong") return;
  
  unsigned long currentTime = millis();
  
  // Only update at specified intervals
  if (currentTime - pongState.lastUpdateTime < pongState.updateInterval) {
    return;
  }
  
  // Clear the display buffer
  disp.displayClear();
  
  // Update position
  pongState.x += pongState.speedX;
  pongState.y += pongState.speedY;
  
  // Check for collisions with the walls
  if (pongState.x >= matrixCols - 1) {
    pongState.speedX = -abs(pongState.speedX); // Ensure we bounce left
    pongState.x = matrixCols - 1;
  } else if (pongState.x <= 0) {
    pongState.speedX = abs(pongState.speedX); // Ensure we bounce right
    pongState.x = 0;
  }
  
  if (pongState.y >= matrixRows - 1) {
    pongState.speedY = -abs(pongState.speedY); // Ensure we bounce up
    pongState.y = matrixRows - 1;
  } else if (pongState.y <= 0) {
    pongState.speedY = abs(pongState.speedY); // Ensure we bounce down
    pongState.y = 0;
  }
  
  // Draw the ball at its current position
  disp.getGraphicObject()->setPoint(round(pongState.y), round(pongState.x), true);
  
  pongState.lastUpdateTime = currentTime;
}

void initSineWaveState() {
  Serial.println("Initializing Sine Wave effect...");
  
  // Set different frequencies and amplitudes for each wave component
  for (int i = 0; i < SINE_PHASES; i++) {
    sineWaveState.phase[i] = random(0, 628) / 100.0; // Random start phase (0-2π)
    sineWaveState.frequency[i] = (i + 1) * 0.05;     // Different frequencies
    sineWaveState.amplitude[i] = SINE_AMPLITUDE / (i + 1); // Decreasing amplitudes
  }
  
  sineWaveState.lastUpdateTime = 0;
  sineWaveState.updateInterval = 50; // ms between updates
  
  Serial.println("✅ Sine Wave effect initialized successfully");
}

void updateSineWaveEffect(const DisplayItem& item) {
  if (item.mode != "sinewave") return;
  
  unsigned long currentTime = millis();
  
  // Only update at specified intervals
  if (currentTime - sineWaveState.lastUpdateTime < sineWaveState.updateInterval) {
    return;
  }
  
  // Clear the display buffer
  disp.displayClear();
  
  // Update phases
  for (int i = 0; i < SINE_PHASES; i++) {
    sineWaveState.phase[i] += sineWaveState.frequency[i];
    if (sineWaveState.phase[i] > TWO_PI) {
      sineWaveState.phase[i] -= TWO_PI;
    }
  }
  
  // Draw the sine wave visualization
  for (int col = 0; col < matrixCols; col++) {
    // Calculate base position (middle of display)
    float basePos = matrixRows / 2.0;
    
    // Calculate combined sine wave value for this column
    float waveHeight = 0;
    for (int i = 0; i < SINE_PHASES; i++) {
      // Create a sine wave with phase offset based on column
      float colPhase = col * 0.3 + sineWaveState.phase[i];
      waveHeight += sin(colPhase) * sineWaveState.amplitude[i];
    }
    
    // Calculate final position
    int rowPos = round(basePos + waveHeight);
    
    // Draw the point
    if (rowPos >= 0 && rowPos < matrixRows) {
      disp.getGraphicObject()->setPoint(rowPos, col, true);
    }
    
    // Draw a slightly faded second point to make the line thicker if needed
    int secondaryRow = waveHeight > 0 ? rowPos - 1 : rowPos + 1;
    if (secondaryRow >= 0 && secondaryRow < matrixRows && random(3) == 0) {
      disp.getGraphicObject()->setPoint(secondaryRow, col, true);
    }
  }
  
  sineWaveState.lastUpdateTime = currentTime;
}

// Add these to your initialization function
void initializeEffects() {
  initTwinkleStates();
  initKnightRiderState();
  initPongState();
  initSineWaveState();
}

// Add these to your main update loop
void updateEffects(const DisplayItem& item) {
  updateTwinkleEffect(item);
  updateKnightRiderEffect(item);
  updatePongEffect(item);
  updateSineWaveEffect(item);
}