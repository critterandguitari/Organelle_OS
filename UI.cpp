
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>
#include <sys/stat.h>

#include "UI.h"

UI::UI(){

    numPatches = 0;
    selectedPatch = 0;
    patchlistOffset = 9;
    cursorOffset = 1;
    patchIsRunning = 0;
    menuScreenTimeout = MENU_TIMEOUT;

    encoderEnabled = 1;     // encoder input is enabled

    newScreen = 0;
    currentScreen = MENU;

    menuScreen.clear();
    patchScreen.clear();
    auxScreen.clear();
}

int UI::checkFileExists (const char * filename){
    struct stat st;
    int result = stat(filename, &st);
    return result == 0;
}

void UI::encoderUp(void) {
    if (cursorOffset == 4) {
        if (!(patchlistOffset >= (numPatches - 1))) patchlistOffset++;
    }
    if (!(cursorOffset >= 4)) cursorOffset++;
    
    drawPatchList();
}

void UI::encoderDown(void) {
    if (cursorOffset == 0) {
        if (!(patchlistOffset < 1)) patchlistOffset--;
    }
    if (!(cursorOffset < 1)) cursorOffset--;
    
    drawPatchList();
}

void UI::encoderPress(void){
    char cmd[256];
    
    selectedPatch =  patchlistOffset + cursorOffset;
    printf("selected patch: %d, %s\n", selectedPatch, patches[selectedPatch]);
    
    if (!strcmp(patches[selectedPatch], "Reload")){
        printf("Reloading... ");
        selectedPatch = 0;
        patchlistOffset = 9;
        cursorOffset = 1;
        getPatchList();
        drawPatchList();
    }
 
    if (!strcmp(patches[selectedPatch], "Shutdown")){
        printf("Shutting down... ");
        sprintf(cmd, "shutdown -h now");
        system(cmd);
    }
    
    if (!strcmp(patches[selectedPatch], "Info")){
        printf("Displaying system info... ");

        auxScreen.clear();
        auxScreen.drawNotification("     System Info     ");

        auxScreen.setLine(1, "Software Version: 1");
        auxScreen.setLine(2, "USB: ...");
        auxScreen.setLine(3, "MIDI: ...");
        auxScreen.setLine(4, "CPU Load: ...");
        
        newScreen = 1;
        currentScreen = AUX;
    }
     
    if (!strcmp(patches[selectedPatch], "Eject")){
        printf("Ejecting USB drive... ");

        auxScreen.clear();
        auxScreen.drawNotification("          ");

        auxScreen.setLine(1, "this will eject USB ...");
        
        newScreen = 1;
        currentScreen = AUX;
    }

    if (selectedPatch >= 10) { 
        // check for X,
        // run pd with nogui if no X. also use smaller audio buf with nogui
        // the rest of the settings are in /root/.pdsettings
        if(system("/root/scripts/check-for-x.sh")){
            printf("starting in GUI mode");
            if (checkFileExists("/mnt/usbdrive/patches/mother.pd")) sprintf(cmd, "/usr/bin/pd -rt -audiobuf 10 /mnt/usbdrive/patches/mother.pd \"/mnt/usbdrive/patches/%s/main.pd\" &", patches[selectedPatch]);
            else sprintf(cmd, "/usr/bin/pd -rt -audiobuf 10 /root/mother.pd \"/mnt/usbdrive/patches/%s/main.pd\" &", patches[selectedPatch]);
        }
        else {
            printf("starting in NON GUI mode");
            if (checkFileExists("/mnt/usbdrive/patches/mother.pd")) sprintf(cmd, "/usr/bin/pd -rt -nogui -audiobuf 4 /mnt/usbdrive/patches/mother.pd \"/mnt/usbdrive/patches/%s/main.pd\" &", patches[selectedPatch]);
            else sprintf(cmd, "/usr/bin/pd -rt -nogui -audiobuf 4 /root/mother.pd \"/mnt/usbdrive/patches/%s/main.pd\" &", patches[selectedPatch]);
        }

        // first kill any other PD
        system("killall pd");
        system(cmd);
        patchIsRunning = 1;
        patchScreen.clear();
        currentScreen = PATCH;
        
        // put the patch name on the menu screen
        sprintf(cmd, "> %s", patches[selectedPatch]);
        menuScreen.drawNotification(cmd);

    }
}

void UI::encoderRelease(void){

}

void UI::drawPatchList(void){
    char line[256];
    int i;
    for (i=0; i<5; i++) {
        sprintf(line, "%s", patches[i + patchlistOffset]);
        menuScreen.setLine(i + 1, line);
    }

    menuScreen.invertLine(cursorOffset);   

    if (!patchIsRunning) {
        menuScreen.drawNotification("Select a patch cool...");
    }

    newScreen = 1;
    menuScreenTimeout = MENU_TIMEOUT;
    currentScreen = MENU;
    //printf("c %d, p %d\n", cursorOffset, patchlistOffset);
}

void UI::getPatchList(void){


    // find patches
    struct dirent **namelist;
    int n;
    int i;
  
    // clear em out
    for (i = 0; i < 127; i++){
        strcpy(patches[i], "");
    }

    // inititial patches
    numPatches = 0;
    strcpy(patches[0], "");
    numPatches++;
    strcpy(patches[1], "");
    numPatches++;
    strcpy(patches[2], "------ SYSTEM -------");
    numPatches++;

    strcpy(patches[3], "Eject");
    numPatches++;

    strcpy(patches[4], "Reload");
    numPatches++;

    strcpy(patches[5], "Info");
    numPatches++;

    strcpy(patches[6], "Shutdown");
    numPatches++;

    strcpy(patches[7], "");
    numPatches++;
    strcpy(patches[8], "");
    numPatches++;
    strcpy(patches[9], "------ PATCHES ------");
    numPatches++;


    // set locale so sorting happens in right order
    setlocale(LC_ALL, "en_US.UTF-8");

    //n = scandir("/home/debian/Desktop/patches", &namelist, NULL, alphasort);
    n = scandir("/mnt/usbdrive/patches", &namelist, NULL, alphasort);
    if (n<0)
        perror("scandir");
    else {

       while (n--) {
            if (namelist[n]->d_type == DT_DIR && strcmp (namelist[n]->d_name, "..") != 0 && strcmp (namelist[n]->d_name, ".") != 0) {
                strcpy(patches[numPatches], namelist[n]->d_name);
                numPatches++;
                numPatches &= 0x7f;  // 128 max num patches
            }
            //printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    for (i=0; i<numPatches; i++) {
        printf("patch[%d]: %s\n", i, patches[i]);
    }
}



