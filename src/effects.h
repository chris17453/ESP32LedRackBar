#ifndef EFFECTS_H
#define EFFECTS_H

#include "config.h"

// Structure to track individual LED twinkle state
struct TwinkleState {
  bool active;            // Whether this LED is currently twinkling
  unsigned long startTime; // When this twinkle started
  unsigned long duration;  // How long this twinkle lasts (ms)
  uint8_t maxBrightness;  // Maximum brightness this twinkle will reach (0-15)
  uint8_t row;            // Row position of this twinkle
  uint8_t col;            // Column position of this twinkle
};

// Define max number of concurrent twinkles
#define MAX_ACTIVE_TWINKLES 100  // Adjust based on your needs

// External variables for effects
extern TwinkleState twinkleStates[MAX_ACTIVE_TWINKLES];
extern uint8_t matrixRows;
extern uint8_t matrixCols;

// Function declarations
void initTwinkleStates();
void updateTwinkleEffect(const DisplayItem& item);
void safeReinitTwinkle();

#endif // EFFECTS_H