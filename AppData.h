#ifndef APPDATA_H
#define APPDATA_H

#include <stdint.h> 
#include <string>

#include "OledScreen.h"

// the UI provides 3 screens
#define AUX 1       // for alerts and info 
#define MENU 2      // for main menu
#define PATCH 3     // for patch details

#define MENU_TIMEOUT 2000  // m sec timeout when screen switches back to patch detail

// location of Patches folder 
#define PATCHES_PATH "/usbdrive/Patches"
#define SYSTEMS_PATH "/usbdrive/System"

class AppData
{
    public:
        AppData();
        char currentPatch[256];
        int patchIsRunning;         // if an actual patch is running
        int newScreen;              // flag indicating screen changed and needs to be sent to oled
        int currentScreen;          // the current screen (AUX, MENU or PATCH)
        int patchScreenEncoderOverride;  // when 1, encoder input is ignored in menu scree, routed to patch
        int auxScreenEncoderOverride; // when 1, encoder input is routed to aux
        int menuScreenTimeout;

        OledScreen menuScreen;
        OledScreen patchScreen;
        OledScreen auxScreen;

};


#endif
