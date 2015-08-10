#ifndef PATCHMENU_H
#define PATCHMENU_H


#include "OledScreen.h"

#include <stdint.h> 
#include <string>


class PatchMenu
{
    public:
        int num_patches;
        char patches[128][256];  // holds names of patches
        int selected_patch;
        int patchlist_offset;
        char current_patch[256];


        void getPatchList(void);
        void drawPatchList(OledScreen &screen);
        void up(void);
        void down(void);

};


#endif
