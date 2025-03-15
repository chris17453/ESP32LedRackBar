#ifndef LOOP_FUNCTIONS_H
#define LOOP_FUNCTIONS_H

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "effects.h"

// Function declarations for main loop operations

// Check system memory usage
void checkSystemMemory(int force);

// Handle system update process
bool handleUpdateProcess();

// Handle IP display mode
bool handleIpDisplayMode();

// Check if display should be active
bool checkDisplayActive();

// Validate current item settings
void validateCurrentItem();

// Check if it's time to move to the next item
bool checkForItemTransition();

// Process item transition - handles item switching, deletion if needed
void processItemTransition();

// Handle item deletion if needed
bool handleItemDeletion();

// Create a default item when none exist
void createDefaultItem();

// Move to the next item in the playlist
void moveToNextItem();

// Handle display mode transition, updating settings as needed
void handleDisplayModeTransition(const String& oldMode, DisplayItem& newItem);

// Update display based on current item mode
void updateDisplayContent();

// Update text display mode
void updateTextDisplay(DisplayItem& currentItem);

// Checks for wifi and api init and runs... only needs to be run once
void wifi_api_setup();

#endif // LOOP_FUNCTIONS_H