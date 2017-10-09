#ifndef MainMenu_H
#define MainMenu_H


#include "AppData.h"

#include <stdint.h> 
#include <string>
#include <vector>

#define MAX_MENU_ENTRIES 1024

class MainMenu
{
public:
    MainMenu();

    // encoder events

    void encoderPress(void);
    void encoderRelease(void);
    void encoderUp(void);
    void encoderDown(void);
    void programChange(int pgm);

    void buildMenu(void);
    bool loadPatch(const char* patchname);
    void runShutdown(const char* name,const char* arg);
private:
    void drawPatchList(void);
    int  checkFileExists (const char * filename);
    int  execScript(const char*);
    void setEnv(const char* workdir);
    bool isMotherPdCompatible(const char* motherpd);

    void addMenuItem(int i, const char* name, const char* arg, void (MainMenu::* func) (const char*, const char*));
    std::string getPDOptions(const std::string& file);

    struct MenuItem {
        char name[22];
        char arg[256];
        void (MainMenu::* func) (const char*, const char*);
    };


    void runPatch(const char* name,const char*);
    void runReload(const char* name,const char* arg);
    void runInfo(const char* name,const char* arg);
    void runEject(const char* name,const char* arg);
    void runMidiChannel(const char* name,const char* arg);
    void runSave(const char* name,const char* arg);
    void runSaveNew(const char* name,const char* arg);
    void runSystemCommand(const char* name,const char*);
    void runDoNothing(const char* name,const char*);
    void runCdPatchDirectory(const char* name,const char*);
    void runCdPatchHome(const char* name,const char*);

    void runFavourite(const char* name,const char*);
    void runToggleFavourites(const char* name,const char*);

    void runAddToFavourite(const char* name,const char*);
    void runDelFromFavourite(const char* name,const char*);

    MenuItem menuItems[MAX_MENU_ENTRIES];

    int numMenuEntries;
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

    bool actionTrigger;

    bool favouriteMenu;

    void executeAction(void (MainMenu::*)(const char*, const char*),const char*, const char*);
};


#endif
