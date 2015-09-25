#ifndef PATCHMENU_H
#define PATCHMENU_H


#include "OledScreen.h"

#include <stdint.h> 
#include <string>


class PatchMenu
{
    public:
        PatchMenu();
        int num_patches;
        char patches[128][256];  // holds names of patches
        int selected_patch;
        int patchlist_offset;
        int curser_offset;
        char current_patch[256];

        void encoderPress(void);
        void encoderRelease(void);

        void getPatchList(void);
        void drawPatchList(OledScreen &screen);
        void encoderUp(void);
        void encoderDown(void);

};


#endif
