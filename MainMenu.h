#ifndef MAINMENU_H
#define MAINMENU_H


#include "OledScreen.h"

#include <stdint.h> 
#include <string>


class MainMenu
{
    public:
        MainMenu();
        int num_patches;
        char patches[128][256];  // holds names of patches
        int selected_patch;
        int patchlist_offset;
        int curser_offset;
        char current_patch[256];
        int patchIsRunning;

        void encoderPress(void);
        void encoderRelease(void);

        void getPatchList(void);
        void drawPatchList(OledScreen &screen);
        void encoderUp(void);
        void encoderDown(void);

};


#endif
