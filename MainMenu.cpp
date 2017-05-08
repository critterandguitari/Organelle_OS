
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>
#include <sys/stat.h>

#include "MainMenu.h"

extern AppData app; 

MainMenu::MainMenu(){
    numPatches = 0;
    numMenuEntries = 0;
    menuOffset = 9;
    cursorOffset = 1;
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

void MainMenu::encoderPress(void){
    selectedEntry =  menuOffset + cursorOffset;
    printf("Menu Selection: %d, %s\n", selectedEntry, menuItems[selectedEntry]);
    
    // menu items 0-10 are part of system menu
    if (selectedEntry < patchMenuOffset) {    
       runSystemCommand();
    }
    else { 
        runPatch();       
    }
}

void MainMenu::encoderRelease(void){

}

void MainMenu::runSystemCommand(void){
    char buf[256];
    app.auxScreenEncoderOverride = 0;
    if (!strcmp(menuItems[selectedEntry], "Reload")){
        printf("Reloading... \n");
        sprintf(buf, "/root/scripts/mount.sh");
        system(buf);
        buildMenu();
        drawPatchList();
    }
 
    else if (!strcmp(menuItems[selectedEntry], "Shutdown")){
        printf("Shutting down... \n");
        sprintf(buf, "/root/scripts/shutdown.sh &");
        system(buf);
    }
    
    else if (!strcmp(menuItems[selectedEntry], "Info")){
        printf("Displaying system info... \n");
        app.auxScreen.clear();
        app.auxScreen.drawNotification("     System Info     ");
        sprintf(buf, "/root/scripts/info.sh &");
        system(buf);

    }
     
    else if (!strcmp(menuItems[selectedEntry], "Eject")){
        printf("Ejecting USB drive... \n");
        sprintf(buf, "/root/scripts/eject.sh &");
        system(buf);
    }
      
    else if (!strcmp(menuItems[selectedEntry], "MIDI Channel")){
        printf("Selecting MIDI ch... \n");
        sprintf(buf, "/root/scripts/midi-config.sh &");
        system(buf);
    }

    else if (!strcmp(menuItems[selectedEntry], "Save")){
        printf("Saving... \n");
        sprintf(buf, "/root/scripts/save-patch.sh &");
        system(buf);
    }
    else if (!strcmp(menuItems[selectedEntry], "Save New")){
        printf("Saving new... \n");
        sprintf(buf, "/root/scripts/save-new-patch.sh &");
     
     
     
     system(buf);
    }
    
    
    
    else {
        sprintf(buf, "\""SYSTEMS_PATH"/%s/run.sh\" &", menuItems[selectedEntry]);
        system(buf);
        printf("%s \n", buf);
   }
}

void MainMenu::runPatch(void){
    char buf[256];
    char buf2[256];

    if (strcmp(menuItems[selectedEntry], "") == 0) {
        printf("Empty menu entry\n");
        return;
    }
    
    sprintf(buf, PATCHES_PATH"/%s/main.pd", menuItems[selectedEntry]);
    printf("Checking for Patch File: %s\n", buf);
    if (checkFileExists(buf)) {
        // check for X,
        // run pd with nogui if no X. also use smaller audio buf with nogui
        // the rest of the settings are in /root/.pdsettings
        if(system("/root/scripts/check-for-x.sh")){
            printf("starting in GMainMenu mode\n");
            if (checkFileExists(PATCHES_PATH"/mother.pd")) sprintf(buf, "/usr/bin/pd -rt -audiobuf 10 "PATCHES_PATH"/mother.pd \""PATCHES_PATH"/%s/main.pd\" &", menuItems[selectedEntry]);
            else sprintf(buf, "/usr/bin/pd -rt -audiobuf 10 /root/mother.pd \""PATCHES_PATH"/%s/main.pd\" &", menuItems[selectedEntry]);
        }
        else {
            printf("starting in NON GMainMenu mode\n");
            if (checkFileExists(PATCHES_PATH"/mother.pd")) sprintf(buf, "/usr/bin/pd -rt -nogui -audiobuf 4 "PATCHES_PATH"/mother.pd \""PATCHES_PATH"/%s/main.pd\" &", menuItems[selectedEntry]);
            else sprintf(buf, "/usr/bin/pd -rt -nogui -audiobuf 4 /root/mother.pd \""PATCHES_PATH"/%s/main.pd\" &", menuItems[selectedEntry]);
        }

        // first kill any other PD
        system("/root/scripts/killpd.sh");

        // remove previous symlink, make new one
        system("rm /tmp/patch");   
        sprintf(buf2, "ln -s \""PATCHES_PATH"/%s\" /tmp/patch", menuItems[selectedEntry]);
        system(buf2);

        // save the name 
        system("rm -fr /tmp/curpatchname");
        sprintf(buf2, "mkdir -p /tmp/curpatchname/\"%s\"", menuItems[selectedEntry]);
        system(buf2);

        //printf("%s \n", buf2);
    
        // disable encoder override
        app.patchScreenEncoderOverride = 0;
        
        // start patch
        system(buf);

        // update stuff
        app.patchIsRunning = 1;
        app.patchScreen.clear();
        app.currentScreen = PATCH;
        app.newScreen = 1;
        strcpy(app.currentPatch, menuItems[selectedEntry]);
        
        // put the patch name on top of screen
        // truncate long file names
        int len = strlen(menuItems[selectedEntry]);
        if (len > 20) {
           sprintf(buf, "> %s", menuItems[selectedEntry]);
           buf[11] = '.';
           buf[12] = '.';
           buf[13] = '.';
           strcpy(&buf[14], &menuItems[selectedEntry][len-7]);
           
        }
        else {
            sprintf(buf, "> %s", menuItems[selectedEntry]);
        }
        app.menuScreen.drawNotification(buf);
    } else {
        printf("Patch File Not Found: %s\n", buf);
    }
}

void MainMenu::programChange(int pgm){

    if ((pgm > numPatches) || (pgm < 1)) {
        printf("Program Change out of range\n");
        return;
    }
    else {
        printf("Program Change: %d, %s\n", pgm, menuItems[selectedEntry]);
        selectedEntry = pgm + patchMenuOffset - 1;
        runPatch();
    }
}

void MainMenu::drawPatchList(void){
    char line[256];
    int i;
    int len;
    for (i=0; i<5; i++) {
        // truncate long file names
        len = strlen(menuItems[i + menuOffset]);
        if (len > 21) {
           strcpy(line, menuItems[i + menuOffset]);
           line[9] = '.';
           line[10] = '.';
           line[11] = '.';
           strcpy(&line[12], &menuItems[i + menuOffset][len-9]);
           
        }
        else {
            sprintf(line, "%s", menuItems[i + menuOffset]);
        }
        app.menuScreen.setLine(i + 1, line);
    }
 
    // dont invert patch lines if there are no patches
    if ((selectedEntry >= patchMenuOffset) && !numPatches) {
    }
    else {
        app.menuScreen.invertLine(cursorOffset);   
    }

    if (!app.patchIsRunning) {
        app.menuScreen.drawNotification("Select a patch...");
    }

    app.newScreen = 1;
//    printf("c %d, p %d\n", cursorOffset, menuOffset);
}

void MainMenu::buildMenu(void){

    char buf[256];

    // find patches
    struct dirent **namelist;
    int n;
    int i;
    int mindex;
  
    // clear em out
    for (i = 0; i < 127; i++){
        strcpy(menuItems[i], "");
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
    // padding
    strcpy(menuItems[numMenuEntries++], "");
    strcpy(menuItems[numMenuEntries++], "");
    strcpy(menuItems[numMenuEntries++], "------ SYSTEM -------");
    systemMenuOffset = numMenuEntries;
    strcpy(menuItems[numMenuEntries++], "Eject");
    strcpy(menuItems[numMenuEntries++], "Reload");
    strcpy(menuItems[numMenuEntries++], "Info");
    strcpy(menuItems[numMenuEntries++], "Shutdown");
    strcpy(menuItems[numMenuEntries++], "MIDI Channel");
    strcpy(menuItems[numMenuEntries++], "Save");
    strcpy(menuItems[numMenuEntries++], "Save New");
 //   strcpy(menuItems[numMenuEntries++], "Save Preset");
 
    // system scripts from USB
    // set locale so sorting happ.ns in right order
    // not sure this does anything
    systemUserMenuOffset = numMenuEntries; // the starting point of user system entries
    std::setlocale(LC_ALL, "en_US.UTF-8");
    n = scandir(SYSTEMS_PATH, &namelist, NULL, alphasort);
    if (n<0)
        perror("scandir");
    else {
       for(i = 0; i < n; i++) {
            if (namelist[i]->d_type == DT_DIR && strcmp (namelist[i]->d_name, "..") != 0 && strcmp (namelist[i]->d_name, ".") != 0) {
                strcpy(menuItems[numMenuEntries], namelist[i]->d_name);
                numMenuEntries++;
                // for the uncommon situation of having many system scripts
                if (numMenuEntries > MAX_MENU_ENTRIES - 100) {
                    numMenuEntries = MAX_MENU_ENTRIES - 100;
                }
            }
            free(namelist[i]);
        }
        free(namelist);
    }

    // padding
    strcpy(menuItems[numMenuEntries++], "");
    strcpy(menuItems[numMenuEntries++], "");

    strcpy(menuItems[numMenuEntries++], "------ PATCHES ------");
    patchMenuOffset = numMenuEntries;

    // set locale so sorting happ.ns in right order
    // not sure this does anything
    std::setlocale(LC_ALL, "en_US.UTF-8");
    n = scandir(PATCHES_PATH, &namelist, NULL, alphasort);
    if (n<0)
        perror("scandir");
    else {
       for(i = 0; i < n; i++) {
            if (namelist[i]->d_type == DT_DIR && strcmp (namelist[i]->d_name, "..") != 0 && strcmp (namelist[i]->d_name, ".") != 0) {
                strcpy(menuItems[numMenuEntries], namelist[i]->d_name);
                numMenuEntries++;
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
        printf("patch[%d]: %s\n", i, menuItems[i]);
    }

    printf("num patches %d\n", numPatches);
    printf("patch menu offset %d\n", patchMenuOffset);

    // notify if no patches found
    if (!numPatches){
        strcpy(menuItems[numMenuEntries++], "No patches found!");
        strcpy(menuItems[numMenuEntries++], "Insert USB drive ");
        strcpy(menuItems[numMenuEntries++], "with Patches folder.");
        strcpy(menuItems[numMenuEntries++], "Then select Reload.");
    }

    // set cursor to beg
    menuOffset = patchMenuOffset - 1;
    cursorOffset = 1;

    // kill pd 
    printf("stopping pd... \n");
    sprintf(buf, "/root/scripts/killpd.sh ");
    system(buf);
    app.patchIsRunning = 0;

}



