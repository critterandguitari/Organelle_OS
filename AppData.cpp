#include "AppData.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

const char* USB_PATCHES="/usbdrive/Patches";
const char* SD_PATCHES="/sdcard/Patches";
const char* DEFAULT_PATCHES="/usbdrive/Patches";

const char* USB_USERDIR="/usbdrive";
const char* SD_USERDIR="/sdcard";
const char* DEFAULT_USERDIR="/usbdrive";

const char* USB_FW="/usbdrive/Firmware";
const char* SD_FW="/sdcard/Firmware";
const char* DEFAULT_FW="/root";

const char* getDefaultPatchDir() {
    struct stat st;
    if(stat(USB_PATCHES, &st)==0) {
        return USB_PATCHES;
    }
    if(stat(SD_PATCHES, &st)==0) {
        return SD_PATCHES;
    }
    return DEFAULT_PATCHES;
}

const char* getDefaultUserDir() {
    if(system("grep -qs /usbdrive /proc/mounts")==0) {
        return USB_USERDIR;
    }
    if(system("grep -qs /sdcard /proc/mounts")==0) {
        return SD_USERDIR;
    }
    return DEFAULT_USERDIR;
}



const char* getDefaultFirwareDir() {
    struct stat st;
    if(stat(USB_FW, &st)==0) {
        return USB_FW;
    }
    if(stat(SD_FW, &st)==0) {
        return SD_FW;
    }
    return DEFAULT_FW;
}


AppData::AppData(){
    patchIsRunning = 0;
    menuScreenTimeout = MENU_TIMEOUT;
    newScreen = 0;
    currentScreen = MENU;
    patchScreenEncoderOverride = 0;
    auxScreenEncoderOverride = 0;
    setPatchDir(NULL);
    setFirmwareDir(NULL);
    setUserDir(NULL);
}

bool AppData::isPatchHome() {
    return patches_path==getDefaultPatchDir();
}

void AppData::setPatchDir(const char* path) {
    if(path==NULL) {
        patches_path=getDefaultPatchDir();
    } else {
        patches_path=path;
    }
}

void AppData::setUserDir(const char* path) {
    if(path==NULL) {
        //strncpy(usercmds_path,"/sdcard/System",256);
        user_path=getDefaultUserDir();
    } else {
        user_path=path;
    }
    system_path=user_path + "/System";
}


void AppData::setFirmwareDir(const char* path) {
    if(path==NULL) {
        firmware_path=getDefaultFirwareDir();
    } else {
        firmware_path=path;
    }
}



