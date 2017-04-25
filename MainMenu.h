#ifndef MainMenu_H
#define MainMenu_H


#include "AppData.h"

#include <stdint.h> 
#include <string>

#define MAX_MENU_ENTRIES 128

class MainMenu
{
    public:
        MainMenu();
        int numMenuEntries;
        char menuItems[128][256];     // holds names of patches
        int selectedEntry;          // index in patches
        int menuOffset;        // position of cursor
        int cursorOffset;
        int systemMenuOffset;
        int systemUserMenuOffset;
        int patchMenuOffset;
        int presetMenuOffset;
        int numSystemItems;
        int numPatches;
        int numPresets; 

        // encoder events
        void encoderPress(void);
        void encoderRelease(void);
        void encoderUp(void);
        void encoderDown(void);

        void runPatch(void);
        void runSystemCommand(void);
        void programChange(int pgm);

        void buildMenu(void);
        void drawPatchList(void);
        // other stuff...
        int checkFileExists (const char * filename);


};


#endif
