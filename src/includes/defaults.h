#ifndef DEFAULTS_H
#define DEFAULTS_H

// Default display settings
#define DEFAULT_BRIGHTNESS 5
#define DEFAULT_SCROLL_SPEED 50  // Lower value = faster scrolling
#define DEFAULT_PAUSE_TIME 2000  // Pause time in milliseconds at the end of scrolling

// Default twinkle parameters
#define DEFAULT_TWINKLE_DENSITY 15     // Higher = more LEDs active (percentage, 0-100)
#define DEFAULT_TWINKLE_MIN_SPEED 50   // Minimum LED cycle speed (ms)
#define DEFAULT_TWINKLE_MAX_SPEED 300  // Maximum LED cycle speed (ms)
#define DEFAULT_MAX_INTENSITY 15       // Maximum brightness level (0-15)

// Power-cycle reset parameters
#define RESET_WINDOW_MS 30000          // Window of time for multiple resets (30 seconds)
#define RESET_COUNT_THRESHOLD 3        // Number of resets required to factory reset

// Default AP settings
#define DEFAULT_AP_NAME "WatkinsLabsLEDRackBAR"
#define DEFAULT_API_KEY "WatkinsLabsLEDRack2025"  // Default API key
#define DEFAULT_HOSTNAME "ledmatrix"               // Default mDNS hostname

// Display timing parameters
#define IP_DISPLAY_DURATION 15000      // Show IP for 15 seconds

#define WIFI_ENABLED true
#define WATCHDOG_RESET_TIMEOUT 8        // in seconds
#define FACTORY_RESET_DISABLED false     //prevents config from be erased after 3 reboots in 30 seconds

#endif // DEFAULTS_H