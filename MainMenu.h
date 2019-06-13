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
    void nextProgram();

    void buildMenu(int = -1);
    void reload();
    void reloadNoRemount();
    bool loadPatch(const char* patchname);
    void runShutdown(const char* name,const char* arg);

    enum MenuMode {
        M_MAIN,
        M_STORAGE,
        M_SETTINGS,
        M_EXTRA,
        M_MAX_ENTRIES
    };

private:
    void drawPatchList(void);
    int  checkFileExists (const std::string& file);
    int  execShell(const std::string& cmd, const std::string& wd);
    int  execScript(const std::string& script);
    int  execPython(const std::string& pyscript, const std::string& wd);

    void setEnv(const std::string& workdir);
    bool isMotherPdCompatible(const std::string& motherpd);

    void addMenuItem(int i, const char* name, const char* arg, void (MainMenu::* func) (const char*, const char*));
    std::string getCmdOptions(const std::string& file);

    struct MenuItem {
        char name[22];
        char arg[256];
        void (MainMenu::* func) (const char*, const char*);
    };


    void runPatch(const char* name,const char*);
    void runScriptCommand(const char* name,const char* arg);
    void runScriptPython(const char* name,const char* arg);
    void runSystemCommand(const char* name,const char*);
    void runSystemPython(const char* name,const char*);
    void runDoNothing(const char* name,const char*);

    void runReload(const char* name,const char* arg);
    void runEject(const char* name,const char* arg);
    void runCdPatchDirectory(const char* name,const char*);
    void runCdPatchHome(const char* name,const char*);
    void runCdSystemDirectory(const char* name,const char*);
    void runCdSystemHome(const char* name,const char*);
    void runInstaller(const char* name,const char*);

    void runFavourite(const char* name,const char*);
    void runToggleFavourites(const char* name,const char*);

    void runCdMenu(const char*,const char*);


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

    MenuMode currentMenu;

    bool favouriteMenu;

    void executeAction(void (MainMenu::*)(const char*, const char*),const char*, const char*);

};


#endif
