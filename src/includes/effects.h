#ifndef EFFECTS_H
#define EFFECTS_H

#include "config.h"
// Define max number of concurrent twinkles
#define MAX_ACTIVE_TWINKLES 100  // Adjust based on your needs
#define SINE_SAMPLES 64     // Number of samples in the wave
#define SINE_AMPLITUDE 3    // Maximum height of the wave (in LEDs)
#define SINE_PHASES 3       // Number of different sine waves to combine

typedef struct {
  bool active;            // Whether this LED is currently twinkling
  unsigned long startTime; // When this twinkle started
  unsigned long duration;  // How long this twinkle lasts (ms)
  uint8_t maxBrightness;  // Maximum brightness this twinkle will reach (0-15)
  uint8_t row;            // Row position of this twinkle
  uint8_t col;            // Column position of this twinkle
} TwinkleState;

typedef struct {
  int position;
  int direction;
  unsigned long lastUpdateTime;
  int updateInterval;
  int tailLength;
} KnightRiderState;

typedef struct {
  float x;          // x position (can be fractional for smooth movement)
  float y;          // y position
  float speedX;     // horizontal speed
  float speedY;     // vertical speed
  unsigned long lastUpdateTime;
  int updateInterval;
} PongState;


typedef struct {
  float phase[SINE_PHASES];    // Current phase of each sine wave
  float frequency[SINE_PHASES]; // Frequency of each sine wave
  float amplitude[SINE_PHASES]; // Amplitude of each sine wave
  unsigned long lastUpdateTime;
  int updateInterval;
} SineWaveState;

// External variables for effects
extern TwinkleState twinkleStates[MAX_ACTIVE_TWINKLES];
extern PongState pongState;
extern SineWaveState sineWaveState;
extern uint8_t matrixRows;
extern uint8_t matrixCols;


// Function declarations
void initTwinkleStates();
void updateTwinkleEffect(const DisplayItem& item);

void initKnightRiderState();
void updateKnightRiderEffect(const DisplayItem& item);

void initPongState();
void updatePongEffect(const DisplayItem& item);

void initSineWaveState();
void updateSineWaveEffect(const DisplayItem& item);

void initializeEffects();
void updateEffects(const DisplayItem& item);

#endif // EFFECTS_H