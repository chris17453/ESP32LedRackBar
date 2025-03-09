#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

// Global display object
extern MD_Parola disp;

// Function declarations
void initDisplay();
void clearDisplayForModeChange(String oldMode, String newMode);
void updateDisplay();
void scrollPortalAddress();

#endif // DISPLAY_H