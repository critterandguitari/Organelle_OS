#ifndef UI_H
#define UI_H


#include "OledScreen.h"

#include <stdint.h> 
#include <string>

// the UI provides 3 screens
#define AUX 1       // for alerts and info 
#define MENU 2      // for main menu
#define PATCH 3     // for patch details

#define MENU_TIMEOUT 2000  // m sec timeout when screen switches back to patch detail

class UI
{
    public:
        UI();
        int numPatches;
        char patches[128][256];     // holds names of patches
        int selectedPatch;          // index in patches
        int patchlistOffset;        // position of cursor
        int cursorOffset;
        char currentPatch[256];
        int patchIsRunning;         // if an actual patch is running
        int newScreen;              // flag indicating screen changed and needs to be sent to oled
        int currentScreen;          // the current screen (AUX, MENU or PATCH)
        int encoderEnabled;         // when 1, encoder input is ignored
        int menuScreenTimeout;

        // encoder events
        void encoderPress(void);
        void encoderRelease(void);
        void encoderUp(void);
        void encoderDown(void);


        void getPatchList(void);
        void drawPatchList(void);

        OledScreen menuScreen;
        OledScreen patchScreen;
        OledScreen auxScreen;

        // other stuff...
        int checkFileExists (const char * filename);
};


#endif
