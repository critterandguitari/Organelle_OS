
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include <clocale>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>

#include <chrono>
#include <thread>

#include "MainMenu.h"

extern AppData app;

std::string getSystemFile(  const std::vector<std::string>& paths,
                            const std::string& filename) {
// look for file in set of paths, in preference order
    struct stat st;
    for (std::string path : paths) {
        std::string fp = path + "/" + filename;
        if (stat(fp.c_str(), &st) == 0) {
            return fp;
        }
    }

    // none found return empty string
    return "";
}


std::string escapePath(std::string path) {
    return std::string("\"") + path + "\"";
}



static const char* MM_STR[MainMenu::MenuMode::M_MAX_ENTRIES] = {
    "MAIN",
    "Storage",
    "Settings",
    "Extra"
};
static const char* MM_TITLE[MainMenu::MenuMode::M_MAX_ENTRIES] = {
    "------ SYSTEM -------",
    "------ Storage ------",
    "----- Settings ------",
    "------- Extra -------"
};

MainMenu::MainMenu() {
    numPatches = 0;
    numMenuEntries = 0;
    menuOffset = 0;
    cursorOffset = 1;
    favouriteMenu = false;
    actionTrigger = false;
    currentMenu = MenuMode::M_MAIN;
}

static const std::string MOTHER_PD_VERSION ="1.2";

bool MainMenu::isMotherPdCompatible(const std::string& motherpd) {
    std::string cmd = std::string ("check-mother-pd.sh \"") + motherpd + "\" " + MOTHER_PD_VERSION; 
    return ! execScript(cmd);
}

int MainMenu::checkFileExists (const std::string& filename) {
    struct stat st;
    int result = stat(filename.c_str(), &st);
    return result == 0;
}

void MainMenu::encoderUp(void) {
    if (cursorOffset == 4) {
        if (!(menuOffset >= (numMenuEntries - 1))) menuOffset++;
    }
    if (!(cursorOffset >= 4)) cursorOffset++;

    selectedEntry = menuOffset + cursorOffset;
    drawPatchList();

}

void MainMenu::encoderDown(void) {
    if (cursorOffset == 0) {
        if (!(menuOffset < 1)) menuOffset--;
    }
    if (!(cursorOffset < 1)) cursorOffset--;

    selectedEntry = menuOffset + cursorOffset;
    drawPatchList();
}

void MainMenu::executeAction(void (MainMenu::* func)(const char*, const char*), const char* name, const char* arg)  {
    (this->*func)(name, arg);
}

void MainMenu::encoderPress(void) {
    actionTrigger = true;
}

void MainMenu::encoderRelease(void) {
    if (actionTrigger) {
        selectedEntry = menuOffset + cursorOffset;
        std::cout << "Menu Selection: " << selectedEntry  << ":" << menuItems[selectedEntry].name << std::endl;
        executeAction(menuItems[selectedEntry].func, menuItems[selectedEntry].name, menuItems[selectedEntry].arg);
    }
    actionTrigger = false;
}

void MainMenu::runDoNothing(const char* name, const char* ) {
}

void MainMenu::runReload(const char* name, const char* arg) {
    reload();
}

void MainMenu::reload(void) {
    std::cout <<"Reloading... \n" << std::endl;
    execScript("mount.sh");
    // set patch and user dir to defaults
    app.setPatchDir(NULL);
    app.setUserDir(NULL);
    currentMenu = MenuMode::M_MAIN;
    buildMenu();
}

void MainMenu::reloadNoRemount(void) {
    std::cout <<"Reloading without remounting... \n" << std::endl;
    // set patch and user dir to defaults
    app.setPatchDir(NULL);
    app.setUserDir(NULL);
    currentMenu = MenuMode::M_MAIN;
    buildMenu();
}


void MainMenu::runScriptCommand(const char* name,const char* arg) {
    std::string cmd = std::string(arg) + " &";
    std::cout << "running : " << cmd << std::endl;
    execScript(cmd);
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runScriptPython(const char* name,const char* arg) {
    std::string cmd = app.getFirmwareDir() + "/scripts/" + arg + " &";
    std::cout << "running : " << cmd << std::endl;
    execPython(cmd,app.getUserDir());
    currentMenu = MenuMode::M_MAIN;
}


void MainMenu::runShutdown(const char* name, const char* arg) {
    std::cout << "Shutting down..." << std::endl;
    execScript("shutdown.sh &");
}

void MainMenu::runEject(const char* name, const char* arg) {
    std::cout << "Ejecting USB drive... " << std::endl;
    execScript("eject.sh &");
    app.setPatchRunning(false);
    app.setPatchScreenEncoderOverride(false);
    currentMenu = MenuMode::M_MAIN;
}


void MainMenu::runSystemCommand(const char* name, const char* arg) {
    std::string location = app.getSystemDir() + "/" + std::string(arg);
    std::string cmd = std::string("\"") + location + "/run.sh\" & ";
    std::cout << "running shell :" << cmd << std::endl;
    execShell(cmd, location);
    currentMenu = MenuMode::M_MAIN;
}


void MainMenu::runSystemPython(const char* name, const char* arg) {
    std::string location = app.getSystemDir() + "/" + std::string(arg);
    std::string cmd = std::string("\"") + location + "/main.py\" &";
    std::cout << "running python:" << cmd << std::endl;
    execPython(cmd, location);
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runPatch(const char* name, const char* arg) {
    char buf[256];
    char buf2[256];

    if (strcmp(arg, "") == 0) {
        std::cerr << "Empty menu entry " <<  std::endl;
        return;
    }

    std::string patchlocation = app.getPatchDir() + "/" + arg;
    std::string pdfile = patchlocation + "/main.pd";
    std::string scfile = patchlocation + "/main.scd";
    std::string pyfile = patchlocation + "/main.py";
    std::string shellfile = patchlocation + "/run.sh";
    std::cout << "Checking for patch Files  : " << patchlocation << std::endl;

    // note this is the order of precedence
    bool isPD = checkFileExists(pdfile);
    bool isSC = checkFileExists(scfile);
    bool isPy = checkFileExists(pyfile);
    bool isShell = checkFileExists(shellfile);

    if (isPD || isSC || isPy ||isShell) {
        std::vector<std::string> paths;
        paths.push_back(patchlocation);
        paths.push_back(app.getSystemDir());
        paths.push_back(app.getFirmwareDir());

        // first kill any other patches
        execScript("killpatch.sh");

        // remove previous symlink, make new one
        system("rm /tmp/patch");
        sprintf(buf2, "ln -s \"%s\" /tmp/patch", patchlocation.c_str());
        system(buf2);

        // save the name
        system("rm -fr /tmp/curpatchname");
        sprintf(buf2, "mkdir -p /tmp/curpatchname/\"%s\"", arg);
        system(buf2);

        // setup /tmp/media and /tmp/data
        std::vector<std::string> userPaths;
        userPaths.push_back(app.getUserDir());
        std::string mediaPath = getSystemFile(userPaths,"media");
        // setup media path
        if(mediaPath.length()>0) {
            system("rm /tmp/media");
            std::string lncmd = std::string("ln -s ") + mediaPath + " /tmp/media";
            std::cout << "linking : " << lncmd << std::endl;
            system(lncmd.c_str());
        }

        // setup data path
        std::string dataPath = getSystemFile(userPaths,"data");
        if(dataPath.length()>0) {
            std::string lncmd = std::string("ln -s ") + dataPath + " /tmp/data";
            std::cout << "linking : " << lncmd << std::endl;
            system(lncmd.c_str());
        }


        // disable encoder override
        app.setPatchScreenEncoderOverride(false);

        // set environment for patch to run in
        setEnv(patchlocation);

        if (isPD) {

#ifdef FIX_ABL_LINK
            // get rid of incompatible abl_link external in patch
            std::cout << "Checking for abl_link external in patch directory: " << patchlocation << std::endl;
            sprintf(buf2, "find \"%s\" -type f -name 'abl_link~.pd_linux' -exec mv {} {}.orig \\;", patchlocation.c_str());
            printf("running: %s \n", buf2);
            system(buf2);
#endif 

            std::string mother = getSystemFile(paths, "mother.pd");
            if (mother.length() == 0) {
                mother = app.getFirmwareDir() + "/mother.pd";
            } else if (isMotherPdCompatible(mother) == false) {
                std::cerr << mother << " incompatible please review, using system mother.pd";
                sprintf(buf2, "mv %s %s.review", mother.c_str(), mother.c_str());
                system(buf2);
            }

            std::string optsfile = getSystemFile(paths, "pd-opts.txt");
            std::string opts;
            if (optsfile.length() > 0) {
                opts = getCmdOptions(optsfile);
            }

            std::string args = "-rt ";
            bool guimode = execScript("check-for-x.sh");
            if (guimode) {
                args += " -audiobuf 10";
            } else {
                args += " -nogui -audiobuf 6";
            }

            args += std::string(" -path ") + app.getUserDir() + "/PdExtraLibs";

            args += opts;

            // prepare cmd line
            sprintf(buf, "( cd /tmp/patch ; /usr/bin/pd %s \"%s\" main.pd )&",
                    args.c_str(),
                    mother.c_str());

            // start pure data with mother and patch
            std::cout << "starting Pure Data : " << buf << std::endl;
            system(buf);

        } else if (isSC) {
            std::cout << "starting jack " << std::endl;
            execScript("start-jack.sh");

            std::string mother = getSystemFile(paths, "mother.scd");
            if (mother.length() == 0) {
                mother = app.getFirmwareDir() + "/mother.scd";
            }

            std::string optsfile = getSystemFile(paths, "sc-opts.txt");
            std::string opts;
            if (optsfile.length() > 0) {
                opts = getCmdOptions(optsfile);
            }

            std::string args = opts;
            sprintf(buf, "( cd /tmp/patch ; echo "" | /usr/local/bin/sclang %s \"%s\" & echo $! > /tmp/pids/sclang.pid )",
                    args.c_str(),
                    mother.c_str()
                   );

            std::cout << "starting SuperCollider : " << buf << std::endl;
            execShell(buf, app.getPatchDir());
        } else if (isPy) {
            // std::string mother = getSystemFile(paths, "mother.py");
            // if (mother.length() == 0) {
            //     mother = app.getFirmwareDir() + "/mother.py";
            // }

            // whilst we dont have a mother.py, lets just start the main patch file
            std::string mother = "main.py";

            std::string optsfile = getSystemFile(paths, "py-opts.txt");
            std::string opts;
            if (optsfile.length() > 0) {
                opts = getCmdOptions(optsfile);
            }

            std::string args = opts;
            sprintf(buf, "( cd /tmp/patch ; python2 %s \"%s\" & echo $! > /tmp/pids/patchpy.pid ) ",
                    args.c_str(),
                    mother.c_str()
                   );
            std::cout << "starting python2 : " << buf << std::endl;
            execShell(buf, app.getPatchDir());
        } else if (isShell) {
            std::string optsfile = getSystemFile(paths, "run-opts.txt");
            std::string opts;
            if (optsfile.length() > 0) {
                opts = getCmdOptions(optsfile);
            }

            std::string args = opts;
            sprintf(buf, "( cd /tmp/patch ; ./run.sh %s & echo $! > /tmp/pids/patchsh.pid ) ",
                    args.c_str()
                   );
            std::cout << "starting shell : " << buf << std::endl;
            execShell(buf, app.getPatchDir());
        }

        // update stuff
        app.setPatchLoading(true);
        app.oled(AppData::PATCH).showInfoBar = true;
        app.oled(AppData::PATCH).clear();
        app.currentScreen = AppData::PATCH;
        app.oled(AppData::PATCH).newScreen = 1;
        strcpy(app.currentPatch, arg);
        strcpy(app.currentPatchPath, app.getPatchDir().c_str());

    } else {
        std::cerr << "No patch found: " << patchlocation << std::endl;
    }
}

void MainMenu::runFavourite(const char* name, const char* arg) {
    app.setPatchDir(arg);
    runPatch(name, name);
}
void MainMenu::runToggleFavourites(const char* name, const char*) {
    favouriteMenu = ! favouriteMenu;
    app.setPatchDir(NULL);
    buildMenu();
}

void MainMenu::runAddToFavourite(const char*, const char*) {
    bool exists = false;
    // check to see if exists
    char favfile[256];

    sprintf(favfile, "%s/Favourites.txt", app.getUserDir().c_str());
    printf("adding to favs %s\n", favfile);
    std::ifstream infile(favfile);
    std::string line;
    while (std::getline(infile, line) && !exists)
    {
        if (line.length() > 0 ) {
            int sep = line.find(":");
            if (sep != std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path = line.substr(0, sep);
                std::string patch = line.substr(sep + 1, line.length());
                if (path.compare(app.getCurrentPatchPath()) == 0 && patch.compare(app.getCurrentPatch()) == 0 ) {
                    exists = true;
                }
            }
        }
    }
    infile.close();
    if (!exists) {
        std::ofstream outfile(favfile, std::ios_base::app);
        outfile << app.getCurrentPatchPath() << ":" << app.getCurrentPatch() << std::endl;
        outfile.close();
    }
    buildMenu();
}

void MainMenu::runDelFromFavourite(const char*, const char*) {
    bool exists = false;
    char favfile[256];
    char tmpfile[256];
    // check to see if exists
    sprintf(favfile, "%s/Favourites.txt", app.getUserDir().c_str());
    sprintf(tmpfile, "%s/Favourites.tmp", app.getUserDir().c_str());
    std::ofstream outfile(tmpfile);
    std::ifstream infile(favfile);
    std::string line;
    while (std::getline(infile, line) && !exists)
    {
        if (line.length() > 0 ) {
            int sep = line.find(":");
            if (sep != std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path = line.substr(0, sep);
                std::string patch = line.substr(sep + 1, line.length());
                if ( !(path.compare(app.getCurrentPatchPath()) == 0 && patch.compare(app.getCurrentPatch()) == 0) ) {
                    outfile << line << std::endl;
                }
            }
        }
    }
    infile.close();
    outfile.close();
    std::remove(favfile);
    std::rename(tmpfile, favfile);
    buildMenu();
}



void MainMenu::programChange(int pgm) {
    if (pgm < 0) { return;}
    printf("received pgmchange %d\n", pgm);
    bool exists = false;
    char favfile[256];
    sprintf(favfile, "%s/Favourites.txt", app.getUserDir().c_str());
    std::ifstream infile(favfile);
    std::string line;
    int idx = 0;
    exists = std::getline(infile, line).good();
    for (idx = 0; idx < (pgm - 1); idx++) {
        exists = std::getline(infile, line).good();
    }

    if (exists) {
        if (line.length() > 0 ) {
            int sep = line.find(":");
            if (sep != std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path = line.substr(0, sep);
                std::string patch = line.substr(sep + 1, line.length());
                printf("Program Change: %d, %s %s\n", pgm, path.c_str(), patch.c_str());
                favouriteMenu = true;
                app.setPatchDir(path.c_str());
                buildMenu();
                runFavourite(patch.c_str(), path.c_str());
            }
        }
    }
    infile.close();
}

void MainMenu::nextProgram() {
    printf("received next programChange\n");
    bool exists = false;
    bool found = false;
    bool next = false;
    char favfile[256];
    sprintf(favfile, "%s/Favourites.txt", app.getUserDir().c_str());
    std::ifstream infile(favfile);
    std::string line;
    std::string foundPath, foundPatch;
    int idx = 0;
    exists = std::getline(infile, line).good();
    while(exists && !found) {
        if (line.length() > 0 ) {
            int sep = line.find(":");
            if (sep != std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path = line.substr(0, sep);
                std::string patch = line.substr(sep + 1, line.length());
                if(next || idx==0) {
                    foundPath = path;
                    foundPatch = patch;
                }
                if(next) {
                    //printf("next patch: %d, %s %s\n", idx, path.c_str(), patch.c_str());
                    found = true;
                } else {
                    next = (path == app.getPatchDir() && patch == app.getCurrentPatch());
                    //if(next) printf("Found existing patch: %d, %s %s\n", idx, path.c_str(), patch.c_str());
                }
            }
        }
        exists = std::getline(infile, line).good();
        idx++;
    }

    if(foundPatch.length()>0) {
        printf("(next) Program Change: %s %s\n", foundPath.c_str(), foundPatch.c_str());
        favouriteMenu = true;
        app.setPatchDir(foundPath.c_str());
        buildMenu();
        runFavourite(foundPatch.c_str(), foundPath.c_str());
    }


    infile.close();

}

void MainMenu::drawPatchList(void) {
    char line[256];
    int i;
    int len;
    for (i = 0; i < 5; i++) {
        // truncate long file names
        len = strlen(menuItems[i + menuOffset].name);
        if (len > 21) {
            strcpy(line, menuItems[i + menuOffset].name);
            line[9] = '.';
            line[10] = '.';
            line[11] = '.';
            strcpy(&line[12], &menuItems[i + menuOffset].name[len - 9]);
        }
        else {
            sprintf(line, "%s", menuItems[i + menuOffset].name);
        }
        app.oled(AppData::MENU).setLine(i + 1, line);
    }

    // dont invert patch lines if there are no patches
    if ((selectedEntry >= patchMenuOffset) && !numPatches) {
    }
    else {
        app.oled(AppData::MENU).invertLine(cursorOffset + 1);
    }

    app.oled(AppData::MENU).newScreen = 1;
//    printf("c %d, p %d\n", cursorOffset, menuOffset);
}


void MainMenu::addMenuItem(int i, const char* name, const char* arg, void (MainMenu::* func) (const char*, const char*)) {
    strncpy(menuItems[i].name, name, 22);
    menuItems[i].name[21] = 0;
    strncpy(menuItems[i].arg, arg, 256);
    menuItems[i].arg[255] = 0;
    menuItems[i].func = func;
}

void MainMenu::buildMenu(signed mm_pos) {

    char buf[256];

    // find patches
    struct dirent **namelist;
    int n;
    int i;
    int mindex;

    // set locale so sorting happens in right order
    std::setlocale(LC_ALL, "en_US.UTF-8");
    std::setlocale(LC_COLLATE, "en_US.UTF-8");

    // clear em out
    for (i = 0; i < MAX_MENU_ENTRIES; i++) {
        menuItems[i].name[0] = 0;
        menuItems[i].arg[0] = 0;
        menuItems[i].func = &MainMenu::runDoNothing;
    }


    // OK  got three sections here
    //  System menu offset = index of 1st item
    //  Patch menu offset = same
    //  Preset menu offset = same

    systemMenuOffset = 0;
    patchMenuOffset = 0;
    presetMenuOffset = 0;
    numSystemItems = 0;
    numPatches = 0;
    numPresets = 0;
    numMenuEntries = 0; // total things

    // System menu
    addMenuItem(numMenuEntries++, MM_TITLE[currentMenu], "", &MainMenu::runDoNothing);
    
    systemMenuOffset = numMenuEntries;

#ifdef PWR_SWITCH
    // dont have shutdown menu item if there is a power switch
#else
    addMenuItem(numMenuEntries++, "Shutdown", "Shutdown", &MainMenu::runShutdown);
#endif
    
    
    switch (currentMenu) {
    case MenuMode::M_STORAGE: {
        addMenuItem(numMenuEntries++, "Eject", "Eject", &MainMenu::runEject);
        addMenuItem(numMenuEntries++, "Reload", "Reload", &MainMenu::runReload);
        addMenuItem(numMenuEntries++, "Save", "save-patch.sh", &MainMenu::runScriptCommand);
        addMenuItem(numMenuEntries++, "Save New", "save-new-patch.sh", &MainMenu::runScriptCommand);
        addMenuItem(numMenuEntries++, "<-- System", MM_STR[MenuMode::M_MAIN], &MainMenu::runCdMenu);
        break;
    }
    case MenuMode::M_SETTINGS: {
        addMenuItem(numMenuEntries++, "MIDI Setup", "midi_setup.py", &MainMenu::runScriptPython);
        addMenuItem(numMenuEntries++, "WiFi Setup", "wifi_setup.py", &MainMenu::runScriptPython);
        addMenuItem(numMenuEntries++, "Pedal Setup", "pedal_setup.py", &MainMenu::runScriptPython);
        addMenuItem(numMenuEntries++, "Info", "info.py", &MainMenu::runScriptPython);
        if (favouriteMenu) {
            addMenuItem(numMenuEntries++, "Show Patches", "Show Patches", &MainMenu::runToggleFavourites);
        } else {
            addMenuItem(numMenuEntries++, "Show Favourites", "Show Favourites", &MainMenu::runToggleFavourites);
        }
        addMenuItem(numMenuEntries++, "<-- System", MM_STR[MenuMode::M_MAIN], &MainMenu::runCdMenu);
        break;
    }
    case MenuMode::M_EXTRA: {
        if (checkFileExists(app.getSystemDir())) {
            n = scandir(app.getSystemDir().c_str(), &namelist, NULL, alphasort);
            if (n < 0)
                std::cerr << "scandir usercmds" << std::endl;
            else {
                for (i = 0; i < n; i++) {
                    if (namelist[i]->d_type == DT_DIR &&
                            strcmp (namelist[i]->d_name, "..") != 0
                            && strcmp (namelist[i]->d_name, ".") != 0) {

                        std::string patchlocation = app.getSystemDir() + "/" + namelist[i]->d_name;
                        std::string runsh = patchlocation + "/run.sh";
                        std::string mainpy = patchlocation + "/main.py";

                        if (checkFileExists(runsh)) {
                            addMenuItem(numMenuEntries++, namelist[i]->d_name , namelist[i]->d_name, &MainMenu::runSystemCommand);
                            // for the uncommon situation of having many system scripts
                            if (numMenuEntries > MAX_MENU_ENTRIES - 100) {
                                numMenuEntries = MAX_MENU_ENTRIES - 100;
                            }
                        } else if (checkFileExists(mainpy)) {
                            addMenuItem(numMenuEntries++, namelist[i]->d_name , namelist[i]->d_name, &MainMenu::runSystemPython);
                            // for the uncommon situation of having many system scripts
                            if (numMenuEntries > MAX_MENU_ENTRIES - 100) {
                                numMenuEntries = MAX_MENU_ENTRIES - 100;
                            }
                        } else {
                            char dirpath[255];
                            char name[22];
                            int len = strlen(namelist[i]->d_name);
                            strncpy(name, namelist[i]->d_name, 22);
                            if (len < 22) memset(name + len, ' ', 22 - len);
                            name[20] = '>';
                            name[21] = 0;

                            sprintf(dirpath, "%s/%s", app.getSystemDir().c_str(), namelist[i]->d_name);
                            addMenuItem(numMenuEntries++, name , dirpath, &MainMenu::runCdSystemDirectory);
                        }
                    }
                    free(namelist[i]);
                }
                free(namelist);
            }
        }
        if (!app.isSystemHome()) {
            addMenuItem(numMenuEntries++, "<-- Extra Home", "", &MainMenu::runCdSystemHome);
        }

        addMenuItem(numMenuEntries++, "<-- System", MM_STR[MenuMode::M_MAIN], &MainMenu::runCdMenu);
        break;
    }
    case MenuMode::M_MAIN:
    default: {
        addMenuItem(numMenuEntries++, "Storage             >", MM_STR[MenuMode::M_STORAGE], &MainMenu::runCdMenu);
        addMenuItem(numMenuEntries++, "Settings            >", MM_STR[MenuMode::M_SETTINGS], &MainMenu::runCdMenu);
        addMenuItem(numMenuEntries++, "Extra               >", MM_STR[MenuMode::M_EXTRA], &MainMenu::runCdMenu);
    }
    }

    if (favouriteMenu) {
        addMenuItem(numMenuEntries++, "---- FAVOURITES -----", "", &MainMenu::runDoNothing);
        if (app.isPatchRunning() || app.isPatchLoading()) {
            addMenuItem(numMenuEntries++, "Add Current", "", &MainMenu::runAddToFavourite);
            addMenuItem(numMenuEntries++, "Remove Current", "", &MainMenu::runDelFromFavourite);
        }

        patchMenuOffset = numMenuEntries;

        char favfile[256];
        sprintf(favfile, "%s/Favourites.txt", app.getUserDir().c_str());
        std::ifstream infile(favfile);
        std::string line;
        while (std::getline(infile, line).good())
        {
            if (line.length() > 0 ) {
                int sep = line.find(":");
                if (sep != std::string::npos && sep > 0 && line.length() - sep > 2) {
                    std::string path = line.substr(0, sep);
                    std::string patch = line.substr(sep + 1, line.length());
                    addMenuItem(numMenuEntries++, patch.c_str(), path.c_str(), &MainMenu::runFavourite);
                    numPatches++;
                    // for the uncommon situation of having many system scripts
                    if (numMenuEntries > MAX_MENU_ENTRIES - 10) {
                        numMenuEntries = MAX_MENU_ENTRIES - 10;
                    }

                } else {
                    std::cerr << "invalid line in favourites" << line << std::endl;
                }
            }
        }
    } else {
        addMenuItem(numMenuEntries++, "------ PATCHES ------", "", &MainMenu::runDoNothing);
        if (!app.isPatchHome()) {
            addMenuItem(numMenuEntries++, "<-- HOME", "", &MainMenu::runCdPatchHome);
        }

        patchMenuOffset = numMenuEntries;

        n = scandir(app.getPatchDir().c_str(), &namelist, NULL, alphasort);
        if (n < 0)
            std::cerr << "scandir patchlist" << std::endl;
        else {
            for (i = 0; i < n; i++) {
                char* fname = namelist[i]->d_name;
                if (fname[0]!='.') {
                    switch(namelist[i]->d_type) { 
                    case DT_DIR : {
                        std::string patchlocation = app.getPatchDir() + "/" + fname;
                        std::string mainpd = patchlocation + "/main.pd";
                        std::string scfile = patchlocation + "/main.scd";
                        std::string pyfile = patchlocation + "/main.py";
                        std::string shellfile = patchlocation + "/run.sh";
                        if (     checkFileExists(mainpd)
                                 ||  checkFileExists(scfile)
                                 ||  checkFileExists(pyfile)
                                 ||  checkFileExists(shellfile)
                           ) {
                            addMenuItem(numMenuEntries++, fname , fname, &MainMenu::runPatch);
                        } else {
                            char dirpath[255];
                            char name[22];
                            int len = strlen(fname);
                            strncpy(name, fname, 22);
                            if (len < 22) memset(name + len, ' ', 22 - len);
                            name[20] = '>';
                            name[21] = 0;

                            sprintf(dirpath, "%s/%s", app.getPatchDir().c_str(), fname);
                            addMenuItem(numMenuEntries++, name , dirpath, &MainMenu::runCdPatchDirectory);
                        }
                        numPatches++;
                        // for the uncommon situation of having many system scripts
                        if (numMenuEntries > MAX_MENU_ENTRIES - 10) {
                            numMenuEntries = MAX_MENU_ENTRIES - 10;
                        }
                        break;
                    } //DT_DIR
                    case DT_REG: {
                        // zip or zop file is for installation
                        int len = strlen(fname);
                        std::cout << fname << std::endl;
                        if(len>4) {
                            char ext[5];
                            ext[0] = fname[len-4];
                            ext[4] = 0;
                            for(int i = 1; i<4;i++) {
                                ext[i] = std::toupper(fname[len-4+i]);
                            }
                            if(strcmp(ext,".ZIP")==0
                            || strcmp(ext,".ZOP")==0) { 
                                numPatches++;
                                std::string itm = std::string("Install ") + fname;
                                addMenuItem(numMenuEntries++, itm.c_str() , fname, &MainMenu::runInstaller);
                            }
                        }
                        break;
                    } //DT_REG
                    default: 
                        break;

                    }// switch 
                }
                free(namelist[i]);
            }
            free(namelist);
        }

        // end patches

        for (i = 0; i < numMenuEntries; i++) {
            std::cout <<  "patch[" << i << "] " << menuItems[i].arg << std::endl;
        }

        std::cout << "num patches " <<  numPatches << std::endl;
        std::cout << "patch menu offset " << patchMenuOffset << std::endl;

        std::cout << "patch path: " << app.getPatchDir() << std::endl;

        // notify if no patches found
        if (!numPatches) {
            addMenuItem(numMenuEntries++, "No patches found!", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "", "", &MainMenu::runDoNothing);
        }
    }

    switch(mm_pos) {
        case -1 :  menuOffset = patchMenuOffset - 1 ;break; 
        case MenuMode::M_SETTINGS: menuOffset = 1; break;
        case MenuMode::M_EXTRA: menuOffset = 1; break;
        case MenuMode::M_MAIN: menuOffset = 1; break;
        case MenuMode::M_STORAGE: menuOffset = 1; break;
        default:  menuOffset = patchMenuOffset - 1 ;break; 
    }
    cursorOffset = 1;
    drawPatchList();
}

void MainMenu::runCdMenu(const char* name, const char*mode) {
    std::cout << "run cd menu " << name << ":" << mode;
    if (strcmp(mode, MM_STR[MenuMode::M_MAIN]) == 0) {
        currentMenu = MenuMode::M_MAIN;
    } else if (strcmp(mode, MM_STR[MenuMode::M_STORAGE]) == 0) {
        currentMenu = MenuMode::M_STORAGE;
    } else if (strcmp(mode, MM_STR[MenuMode::M_SETTINGS]) == 0) {
        currentMenu = MenuMode::M_SETTINGS;
    } else if (strcmp(mode, MM_STR[MenuMode::M_EXTRA]) == 0) {
        currentMenu = MenuMode::M_EXTRA;
    } else {
        //default to main menu
        std::cerr << "unknown menu, use default menu";
        currentMenu = MenuMode::M_MAIN;
    }
    buildMenu(currentMenu);
}

void MainMenu::runCdPatchDirectory(const char* name, const char* arg) {
    std::cout << "Changing Patch directory... " << arg << std::endl;
    app.setPatchDir(arg);
    buildMenu();
}

void MainMenu::runCdPatchHome(const char* name, const char*) {
    std::cout << "Resetting to patch home" << std::endl;
    app.setPatchDir(NULL);
    buildMenu();
}


void MainMenu::runCdSystemDirectory(const char* name, const char* arg) {
    std::cout << "Changing System directory... " << arg << std::endl;
    app.setSystemDir(arg);
    buildMenu(MenuMode::M_EXTRA);
}

void MainMenu::runCdSystemHome(const char* name, const char*) {
    std::cout << "Resetting to system home" << std::endl;
    app.setSystemDir(NULL);
    buildMenu(MenuMode::M_EXTRA);
}

void MainMenu::runInstaller(const char*, const char* arg) {
    char buf[128];
    std::string installfile = escapePath(std::string(arg));
    std::string filename = app.getPatchDir() + "/" + installfile;

    // run script with patch dir as working dir
    sprintf(buf, "%s/scripts/install_package.sh %s &", app.getFirmwareDir().c_str(), installfile.c_str());
    std::cout << "Installing : " << installfile << std::endl;
    setEnv(app.getPatchDir());
    // run async to mother, to allow oled updates
    system(buf);
}


bool MainMenu::loadPatch(const char* patchName) {
    runPatch(patchName, patchName);
    return true;
}

void MainMenu::setEnv(const std::string& location) {
    setenv("PATCH_DIR", app.getPatchDir().c_str(), 1);
    setenv("FW_DIR", app.getFirmwareDir().c_str(), 1);
    setenv("USER_DIR", app.getUserDir().c_str(), 1);
    setenv("WORK_DIR", location.c_str(), 1);
}


int  MainMenu::execPython(const std::string& pyscript, const std::string& wd) {
    std::string cmd = "python2 " + pyscript;
    return execShell(cmd,wd);
}


int  MainMenu::execShell(const std::string& cmd, const std::string& wd) {
    setEnv(wd);
    return system(cmd.c_str());
}


int  MainMenu::execScript(const std::string& script) {
    std::string cmd  = app.getFirmwareDir() + "/scripts/" + script;
    return execShell(cmd,app.getUserDir());
}


std::string  MainMenu::getCmdOptions(const std::string& file) {
    std::string opts;
    std::ifstream infile(file.c_str());
    std::string line;
    while (std::getline(infile, line))
    {
        if (line.length() > 0 ) {
            opts += " " + line;
        }
    }
    return opts;
}



