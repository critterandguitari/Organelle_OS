
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include <clocale>
#include <fstream>
#include <string>
#include <cstdio>

#include "MainMenu.h"

extern AppData app; 



static const char* MM_STR[MainMenu::MenuMode::M_MAX_ENTRIES] = {
    "MAIN",
    "PATCH",
    "UTIL"
};
static const char* MM_TITLE[MainMenu::MenuMode::M_MAX_ENTRIES] = {
    "------ SYSTEM -------",
    "-- Patch Functions --",
    "-- Other Functions --"
};

MainMenu::MainMenu(){
    numPatches = 0;
    numMenuEntries = 0;
    menuOffset = 9;
    cursorOffset = 1;
    favouriteMenu = false;
    actionTrigger = false;
    currentMenu = MenuMode::M_MAIN;
    menuTitle=MM_TITLE[currentMenu];
}

#define MOTHER_PD_VERSION "1.2"

bool MainMenu::isMotherPdCompatible(const char* motherpd) {
    char cmd[128];
    sprintf(cmd,"check-mother-pd.sh \"%s\" %s", motherpd, MOTHER_PD_VERSION);
    return ! execScript(cmd);
}

int MainMenu::checkFileExists (const char * filename){
    struct stat st;
    int result = stat(filename, &st);
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

void MainMenu::executeAction(void (MainMenu::* func)(const char*, const char*),const char* name,const char* arg)  {
    (this->*func)(name, arg);
}

void MainMenu::encoderPress(void){
    actionTrigger = true;
}

void MainMenu::encoderRelease(void){
    if(actionTrigger) {
        selectedEntry = menuOffset + cursorOffset;
        printf("Menu Selection: %d, %s\n", selectedEntry, menuItems[selectedEntry].name);
        executeAction(menuItems[selectedEntry].func,menuItems[selectedEntry].name,menuItems[selectedEntry].arg);
    }
    actionTrigger = false;
}

void MainMenu::runDoNothing(const char* name,const char* ){
}

void MainMenu::runReload(const char* name,const char* arg) {
    printf("Reloading... \n");
    execScript("mount.sh");
    // set patch and user dir to defaults
    app.setPatchDir(NULL);
    app.setUserDir(NULL);
    currentMenu = MenuMode::M_MAIN;
    buildMenu();
}
void MainMenu::runShutdown(const char* name,const char* arg) {
    printf("Shutting down... \n");
    execScript("shutdown.sh &");
}

void MainMenu::runInfo(const char* name,const char* arg) {
    printf("Displaying system info... \n");
    app.oled(AppData::AUX).clear();
    app.oled(AppData::AUX).drawNotification("     System Info     ");
    execScript("info.sh &");
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runEject(const char* name,const char* arg) {
    printf("Ejecting USB drive... \n");
    execScript("eject.sh &");
    app.setPatchRunning(false);
    app.setPatchScreenEncoderOverride(false);
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runMidiChannel(const char* name,const char* arg) {
    printf("Selecting MIDI ch... \n");
    execScript("midi-config.sh &");
    currentMenu = MenuMode::M_MAIN;
}
void MainMenu::runSave(const char* name,const char* arg) {
    printf("Saving... \n");
    execScript("save-patch.sh &");
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runSaveNew(const char* name,const char* arg) {
    printf("Saving new... \n");
    execScript("save-new-patch.sh &");
    currentMenu = MenuMode::M_MAIN;
}


void MainMenu::runSystemCommand(const char* name,const char* arg){
    char location[256];
    char script[256];
    sprintf(location, "%s/%s", app.getSystemDir(),arg);
    sprintf(script, "\"%s/run.sh\" &", location);
    printf("running \"%s\"",script);
    setEnv(location);
    system(script);
    currentMenu = MenuMode::M_MAIN;
}

void MainMenu::runPatch(const char* name,const char* arg){
    char buf[256];
    char buf2[256];
    char pdfile[256];
    char shellfile[256];
    char scfile[256];
    char patchlocation[256];
 
    if (strcmp(arg, "") == 0) {
        printf("Empty menu entry\n");
        return;
    }

    sprintf(patchlocation, "%s/%s", app.getPatchDir(), arg);
    sprintf(pdfile,"%s/main.pd", patchlocation);
    sprintf(scfile,"%s/main.scd", patchlocation);
    sprintf(shellfile,"%s/run.sh", patchlocation);
    printf("Checking for Patch File: %s\n", patchlocation);

    bool isPD = checkFileExists(pdfile);
    bool isSC = checkFileExists(scfile);
    bool isShell = checkFileExists(shellfile);
    
    if (isPD || isSC || isShell) {

        // first kill any other patches
        execScript("killpatch.sh");

        // remove previous symlink, make new one
        system("rm /tmp/patch");   
        sprintf(buf2, "ln -s \"%s\" /tmp/patch", patchlocation);
        system(buf2);

        // save the name 
        system("rm -fr /tmp/curpatchname");
        sprintf(buf2, "mkdir -p /tmp/curpatchname/\"%s\"", arg);
        system(buf2);

        // disable encoder override
        app.setPatchScreenEncoderOverride(false);
        
        // set environment for patch to run in
        setEnv(patchlocation);
    
        if(isPD) {
            // find mother patchdir->systemdir->firmwaredir (root)
            bool motherFound = false;
            char motherpd[128];
            sprintf(motherpd, "%s/mother.pd", patchlocation);
            if(checkFileExists(motherpd)) {
                motherFound = isMotherPdCompatible(motherpd);
                if(!motherFound) {
                    sprintf(buf2, "mv %s %s.review",motherpd,motherpd);
                    system(buf2);
                }
            }
            if(!motherFound) {
                sprintf(motherpd, "%s/mother.pd", app.getSystemDir());
                if(checkFileExists(motherpd)) {
                    motherFound = isMotherPdCompatible(motherpd);
                    if(!motherFound) {
                        sprintf(buf2, "mv %s %s.review",motherpd,motherpd);
                        system(buf2);
                    }
                }
            }
            if(!motherFound) {
                sprintf(motherpd,"%s/mother.pd", app.getFirmwareDir());
                // dont bother with checks, if its wrong not alot can be done!
            }

            // pd arguments

            char pdoptsfile[128];
            std::string pd_opts;
            sprintf(pdoptsfile, "%s/pd-opts.txt", patchlocation);
            if(checkFileExists(pdoptsfile)) {
                pd_opts = getPDOptions(pdoptsfile);
            } else {
                sprintf(pdoptsfile, "%s/pd-opts.txt", app.getSystemDir());
                if(checkFileExists(pdoptsfile)) {
                    pd_opts = getPDOptions(pdoptsfile);
                }
            }

            std::string pd_args = "-rt ";
            bool guimode = execScript("check-for-x.sh");
            if(guimode) {
                pd_args += " -audiobuf 10";
            } else {
                pd_args += " -nogui -audiobuf 4";
            }


            if(app.isAlsa()) pd_args += " -alsamidi";

            pd_args += std::string(" -path ") + app.getUserDir() + "/PdExtraLibs";

            pd_args += pd_opts;

            // prepare cmd line
            sprintf(buf, "( cd /tmp/patch ; /usr/bin/pd %s \"%s\" \"%s\" )&", 
                pd_args.c_str(), 
                motherpd, 
                pdfile);

            // start pure data with mother and patch
            printf("starting Pure Data : %s \n", buf);
            system(buf);

        } else if (isSC) {
            printf("starting jackd\n");
            execScript("start-jack.sh");

            std::string mother_sc="/root/mother.scd";
            std::string scl_args="";
            sprintf(buf, "( cd /tmp/patch ; echo "" | /usr/local/bin/sclang %s \"%s\" & echo $! > /tmp/pids/sclang.pid )",
                scl_args.c_str(),
                mother_sc.c_str()
                );
            printf("starting Supercollider lang: %s \n", buf);
            system(buf);

        } else if (isShell) {
            std::string shell_args;
            sprintf(buf, "( cd /tmp/patch ; run.sh %s & echo $! > /tmp/pids/patchsh.pid ) ", 
                shell_args.c_str() 
                );
            printf("starting shell : %s \n", buf);
            system(buf);

        }

        // update stuff
        app.setPatchLoading(true);
        app.oled(AppData::PATCH).showInfoBar = true;
        app.oled(AppData::PATCH).clear();
        app.currentScreen = AppData::PATCH;
        app.oled(AppData::PATCH).newScreen = 1;
        strcpy(app.currentPatch, arg);
        strcpy(app.currentPatchPath, app.getPatchDir());
        
        // put the patch name on top of screen
        // truncate long file names
        int len = strlen(arg);
        if (len > 20) {
           sprintf(buf, "> %s", arg);
           buf[11] = '.';
           buf[12] = '.';
           buf[13] = '.';
           strcpy(&buf[14], &arg[len-7]);
           
        }
        else {
            sprintf(buf, "> %s", arg);
        }

        app.oled(AppData::MENU).drawNotification(buf);
    } else {
        printf("No patch found: %s\n", patchlocation);
    }
}

void MainMenu::runFavourite(const char* name, const char* arg) {
    app.setPatchDir(arg);
    runPatch(name, name);
}
void MainMenu::runToggleFavourites(const char* name,const char*) {
    favouriteMenu = ! favouriteMenu;
    app.setPatchDir(NULL);
    buildMenu();
}

void MainMenu::runAddToFavourite(const char*, const char*) {
    bool exists = false;
    // check to see if exists
    char favfile[256];

    sprintf(favfile, "%s/Favourites.txt", app.getUserDir());
    printf("adding to favs %s\n",favfile);
    std::ifstream infile(favfile);
    std::string line;
    while (std::getline(infile, line) && !exists)
    {
        if(line.length()>0 ) {
            int sep = line.find(":");
            if(sep!=std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path= line.substr(0,sep);
                std::string patch = line.substr(sep+1,line.length());
                if(path.compare(app.getCurrentPatchPath())==0 && patch.compare(app.getCurrentPatch())==0 ) {
                    exists = true;
                }
            }
        }
    }
    infile.close();
    if(!exists) {
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
    sprintf(favfile, "%s/Favourites.txt", app.getUserDir());
    sprintf(tmpfile, "%s/Favourites.tmp", app.getUserDir());
    std::ofstream outfile(tmpfile);
    std::ifstream infile(favfile);
    std::string line;
    while (std::getline(infile, line) && !exists)
    {
        if(line.length()>0 ) {
            int sep = line.find(":");
            if(sep!=std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path= line.substr(0,sep);
                std::string patch = line.substr(sep+1,line.length());
                if( !(path.compare(app.getCurrentPatchPath())==0 && patch.compare(app.getCurrentPatch())==0) ) {
                    outfile << line << std::endl;
                }
            }
        }
    }
    infile.close();
    outfile.close();
    std::remove(favfile);
    std::rename(tmpfile,favfile);
    buildMenu();
}



void MainMenu::programChange(int pgm){
    if(pgm<0) { return;}
    printf("recieved pgmchange %d\n",pgm);
    bool exists = false;
    char favfile[256];
    sprintf(favfile, "%s/Favourites.txt", app.getUserDir());
    std::ifstream infile(favfile);
    std::string line;
    int idx = 0;
    exists = std::getline(infile, line).good();
    for(idx = 0;idx<(pgm-1); idx++) {
      exists = std::getline(infile, line).good();
    }

    if(exists) {
        if(line.length()>0 ) {
            int sep = line.find(":");
            if(sep!=std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string path= line.substr(0,sep);
                std::string patch = line.substr(sep+1,line.length());
                printf("Program Change: %d, %s %s\n", pgm, path.c_str(), patch.c_str());
                favouriteMenu = true;
                app.setPatchDir(path.c_str());
                buildMenu();
                runFavourite(patch.c_str(),path.c_str());                    
            }
        }
    }
    infile.close();
}

void MainMenu::drawPatchList(void){
    char line[256];
    int i;
    int len;
    for (i=0; i<5; i++) {
        // truncate long file names
        len = strlen(menuItems[i + menuOffset].name);
        if (len > 21) {
           strcpy(line, menuItems[i + menuOffset].name);
           line[9] = '.';
           line[10] = '.';
           line[11] = '.';
           strcpy(&line[12], &menuItems[i + menuOffset].name[len-9]);
           
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

    if (! (app.isPatchRunning() || app.isPatchLoading()) ) {
        app.oled(AppData::MENU).drawNotification("Select a patch...");
    }

    app.oled(AppData::MENU).newScreen = 1;
//    printf("c %d, p %d\n", cursorOffset, menuOffset);
}


void MainMenu::addMenuItem(int i, const char* name, const char* arg, void (MainMenu::* func) (const char*, const char*)) {
    strncpy(menuItems[i].name, name,22);
    menuItems[i].name[21]=0;
    strncpy(menuItems[i].arg, arg,256);
    menuItems[i].arg[255]=0;
    menuItems[i].func = func;
}

void MainMenu::buildMenu(void){

    char buf[256];

    // find patches
    struct dirent **namelist;
    int n;
    int i;
    int mindex;
  
    // set locale so sorting happens in right order
    // not sure this does anything
    std::setlocale(LC_ALL, "en_US.UTF-8");

    // clear em out
    for (i = 0; i < MAX_MENU_ENTRIES; i++){
        menuItems[i].name[0] = 0;
        menuItems[i].arg[0] = 0;
        menuItems[i].func = &MainMenu::runDoNothing;
    }

   
    // OK  got three sections here
    //  System menu offset = index of 1st item
    //  Patch menu offset = same
    //  Preset menu offset = same 
    
    systemMenuOffset = 0;
    systemUserMenuOffset = 0;
    patchMenuOffset = 0;
    presetMenuOffset = 0;
    numSystemItems = 0;
    numPatches = 0;
    numPresets = 0; 
    numMenuEntries = 0; // total things 

    // System menu
    addMenuItem(numMenuEntries++, menuTitle.c_str(), "", &MainMenu::runDoNothing);
    systemMenuOffset = numMenuEntries;

    addMenuItem(numMenuEntries++, "Shutdown","Shutdown", &MainMenu::runShutdown);
    switch(currentMenu) {
        case MenuMode::M_UTILITY: {
            addMenuItem(numMenuEntries++, "MIDI Channel", "MIDI Channel", &MainMenu::runMidiChannel);
            addMenuItem(numMenuEntries++, "Info","Info", &MainMenu::runInfo);
            addMenuItem(numMenuEntries++, "Eject","Eject", &MainMenu::runEject);
            addMenuItem(numMenuEntries++, "Reload","Reload", &MainMenu::runReload);
            addMenuItem(numMenuEntries++, "<-- System", MM_STR[MenuMode::M_MAIN], &MainMenu::runCdMenu);
            break;
        }
        case MenuMode::M_PATCH: {
            addMenuItem(numMenuEntries++, "Save","Save", &MainMenu::runSave);
            addMenuItem(numMenuEntries++, "Save New", "Save New", &MainMenu::runSaveNew);
            if(favouriteMenu) {
                addMenuItem(numMenuEntries++, "Show Patches", "Show Patches", &MainMenu::runToggleFavourites);
            } else {
                addMenuItem(numMenuEntries++, "Show Favourites", "Show Favourites", &MainMenu::runToggleFavourites);
            }
            addMenuItem(numMenuEntries++, "<-- System", MM_STR[MenuMode::M_MAIN], &MainMenu::runCdMenu);
            break;
        }
        case MenuMode::M_MAIN:
        default: {
            addMenuItem(numMenuEntries++, "Patch Functions     >", MM_STR[MenuMode::M_PATCH], &MainMenu::runCdMenu);
            addMenuItem(numMenuEntries++, "Other Functions     >", MM_STR[MenuMode::M_UTILITY], &MainMenu::runCdMenu);
        }
    }
 
    // system scripts from usb/sdcard
    systemUserMenuOffset = numMenuEntries; // the starting point of user system entries


    if(!app.isSystemHome()) { 
        addMenuItem(numMenuEntries++, "<-- Script Menu", "", &MainMenu::runCdSystemHome);
    }

    if(checkFileExists(app.getSystemDir())) {
            n = scandir(app.getSystemDir(), &namelist, NULL, alphasort);
            if (n < 0)
                perror("scandir usercmds");
            else {
                for (i = 0; i < n; i++) {
                    if (namelist[i]->d_type == DT_DIR &&
                    strcmp (namelist[i]->d_name, "..") != 0
                    && strcmp (namelist[i]->d_name, ".") != 0) {

                        char runsh[256];
                        sprintf(runsh, "%s/%s/run.sh", app.getSystemDir(), namelist[i]->d_name);
                        if (checkFileExists(runsh)) {
                            addMenuItem(numMenuEntries++, namelist[i]->d_name , namelist[i]->d_name, &MainMenu::runSystemCommand);
                            // for the uncommon situation of having many system scripts
                            if (numMenuEntries > MAX_MENU_ENTRIES - 100) {
                                numMenuEntries = MAX_MENU_ENTRIES - 100;
                            }
                        } else {
                            char dirpath[255];
                            char name[22];
                            int len = strlen(namelist[i]->d_name);
                            strncpy(name,namelist[i]->d_name,22);
                            if(len<22) memset(name+len,' ',22-len);
                            name[20] = '>';
                            name[21] = 0;
                            
                            sprintf(dirpath,"%s/%s",app.getSystemDir(),namelist[i]->d_name);
                            addMenuItem(numMenuEntries++, name , dirpath, &MainMenu::runCdSystemDirectory);
                        }
                    }
                    free(namelist[i]);
                }
                free(namelist);
            }
    }

    if(favouriteMenu) {
        addMenuItem(numMenuEntries++, "---- FAVOURITES -----", "", &MainMenu::runDoNothing);
        if(app.isPatchRunning() || app.isPatchLoading()) {
            addMenuItem(numMenuEntries++, "Add Current", "", &MainMenu::runAddToFavourite);
            addMenuItem(numMenuEntries++, "Remove Current", "", &MainMenu::runDelFromFavourite);
        }

        patchMenuOffset = numMenuEntries;

        char favfile[256];
        sprintf(favfile, "%s/Favourites.txt", app.getUserDir());
        std::ifstream infile(favfile);
        std::string line;
        while (std::getline(infile, line).good())
        {
            if(line.length()>0 ) {
                int sep = line.find(":");
                if(sep!=std::string::npos && sep > 0 && line.length() - sep > 2) {
                    std::string path= line.substr(0,sep);
                    std::string patch = line.substr(sep+1,line.length());
                    addMenuItem(numMenuEntries++, patch.c_str(), path.c_str(), &MainMenu::runFavourite);
                    numPatches++;
                    // for the uncommon situation of having many system scripts
                    if (numMenuEntries > MAX_MENU_ENTRIES - 10) {
                        numMenuEntries = MAX_MENU_ENTRIES - 10;
                    }

                } else {
                    printf("invalid line in favourites %s\n", line.c_str());
                }
            }
        }
    } else {
        addMenuItem(numMenuEntries++, "------ PATCHES ------", "", &MainMenu::runDoNothing);
        if(!app.isPatchHome()) { 
            addMenuItem(numMenuEntries++, "<-- HOME", "", &MainMenu::runCdPatchHome);
        }

        patchMenuOffset = numMenuEntries;

        // set locale so sorting happ.ns in right order
        // not sure this does anything
        std::setlocale(LC_ALL, "en_US.UTF-8");
        n = scandir(app.getPatchDir(), &namelist, NULL, alphasort);
        if (n<0)
            perror("scandir patchlist");
        else {
           for(i = 0; i < n; i++) {
                if (namelist[i]->d_type == DT_DIR && strcmp (namelist[i]->d_name, "..") != 0 && strcmp (namelist[i]->d_name, ".") != 0) {
                    char mainpd[256];
                    char mainscd[256];
                    char runsh[256];
                    sprintf(mainpd, "%s/%s/main.pd", app.getPatchDir(), namelist[i]->d_name);
                    sprintf(mainscd, "%s/%s/main.scd", app.getPatchDir(), namelist[i]->d_name);
                    sprintf(runsh, "%s/%s/run.sh", app.getPatchDir(), namelist[i]->d_name);
                    if(     checkFileExists(mainpd)
                        ||  checkFileExists(mainscd)
                        ||  checkFileExists(runsh)
                        ) {
                        addMenuItem(numMenuEntries++, namelist[i]->d_name , namelist[i]->d_name, &MainMenu::runPatch);
                    } else {
                        char dirpath[255];
                        char name[22];
                        int len = strlen(namelist[i]->d_name);
                        strncpy(name,namelist[i]->d_name,22);
                        if(len<22) memset(name+len,' ',22-len);
                        name[20] = '>';
                        name[21] = 0;
                        
                        sprintf(dirpath,"%s/%s",app.getPatchDir(),namelist[i]->d_name);
                        addMenuItem(numMenuEntries++, name , dirpath, &MainMenu::runCdPatchDirectory);
                    }
                    numPatches++;
                    // for the uncommon situation of having many system scripts
                    if (numMenuEntries > MAX_MENU_ENTRIES - 10) {
                        numMenuEntries = MAX_MENU_ENTRIES - 10;
                    }
                }
                free(namelist[i]);
            }
            free(namelist);
        }

        // end patches

        for (i=0; i<numMenuEntries; i++) {
            printf("patch[%d]: %s\n", i, menuItems[i].arg);
        }

        printf("num patches %d\n", numPatches);
        printf("patch menu offset %d\n", patchMenuOffset);

        // notify if no patches found
        if (!numPatches){
            addMenuItem(numMenuEntries++, "No patches found!", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "Insert USB drive ", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "with Patches folder.", "", &MainMenu::runDoNothing);
            addMenuItem(numMenuEntries++, "Then select Reload.", "", &MainMenu::runDoNothing);
        }
    }

    menuOffset = patchMenuOffset - 1;
    cursorOffset = 1;
    drawPatchList();
}

void MainMenu::runCdMenu(const char* name,const char*mode) {
    printf("run cd menu %s:%s \n",name,mode);
    if(strcmp(mode,MM_STR[MenuMode::M_MAIN])==0) {
        printf("main menu\n");
        currentMenu = MenuMode::M_MAIN;
    } else if(strcmp(mode,MM_STR[MenuMode::M_PATCH])==0) {
        printf("patch menu\n");
        currentMenu = MenuMode::M_PATCH;
    } else if(strcmp(mode,MM_STR[MenuMode::M_UTILITY])==0) {
        printf("utility menu\n");
        currentMenu = MenuMode::M_UTILITY;
    } else {
        //default to main menu
        printf("default menu\n");
        currentMenu = MenuMode::M_MAIN;
    }
    menuTitle=MM_TITLE[currentMenu];
    buildMenu();
}

void MainMenu::runCdPatchDirectory(const char* name,const char* arg) {
    printf("Changing Patch directory... %s\n",arg);
    app.setPatchDir(arg);
    buildMenu();
}

void MainMenu::runCdPatchHome(const char* name,const char*) {
    printf("Resetting to patch home\n");
    app.setPatchDir(NULL);
    buildMenu();
}


void MainMenu::runCdSystemDirectory(const char* name,const char* arg) {
    printf("Changing System directory... %s\n",arg);
    app.setSystemDir(arg);
    buildMenu();
}

void MainMenu::runCdSystemHome(const char* name,const char*) {
    printf("Resetting to system home\n");
    app.setSystemDir(NULL);
    buildMenu();
}



bool MainMenu::loadPatch(const char* patchName) {
    runPatch(patchName, patchName);       
    return true;
}

void MainMenu::setEnv(const char* location) {
    setenv("PATCH_DIR",app.getPatchDir(),1);
    setenv("FW_DIR",app.getFirmwareDir(),1);
    setenv("USER_DIR",app.getUserDir(),1);
    setenv("WORK_DIR",location,1);
}


int MainMenu::execScript(const char* cmd) {
    char buf[128];
    sprintf(buf,"%s/scripts/%s",app.getFirmwareDir(),cmd);
    setEnv(app.getUserDir());
    return system(buf);
}

std::string  MainMenu::getPDOptions(const std::string& file) {
    std::string pd_opts;
    std::ifstream infile(file.c_str());
    std::string line;
    while (std::getline(infile, line))
    {
        if(line.length() >0 ) {
            pd_opts += " " + line;
        }
    }
    return pd_opts;
}



