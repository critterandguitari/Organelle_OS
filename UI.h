#ifndef UI_H
#define UI_H


#include "OledScreen.h"

#include <stdint.h> 
#include <string>

#define ALERT 1
#define MENU 2
#define PATCH 3

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
        int currentScreen;

        void encoderPress(void);
        void encoderRelease(void);

        void getPatchList(void);
        void drawPatchList(void);
        void encoderUp(void);
        void encoderDown(void);

        OledScreen menuScreen;
        OledScreen patchScreen;
        OledScreen alertScreen;

};


#endif
