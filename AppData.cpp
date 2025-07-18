#include "AppData.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

const char* USB_PATCHES="/usbdrive/Patches";
const char* SD_PATCHES="/sdcard/Patches";
const char* DEFAULT_PATCHES="/usbdrive/Patches";

const char* USB_USERDIR="/usbdrive";
const char* SD_USERDIR="/sdcard";
const char* DEFAULT_USERDIR="/usbdrive";

const char* DEFAULT_FW="/root/fw_dir";

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

const std::string getDefaultSystemDir(std::string& user_path) {
    return (user_path + "/System");
}


const char* getDefaultFirwareDir() {
    if (getenv("FW_DIR")) return getenv("FW_DIR");
    else return DEFAULT_FW;
}

AppData::AppData(){
    patchIsRunning =false;
    patchIsLoading =false;
    menuScreenTimeout = MENU_TIMEOUT;
    currentScreen = MENU;
    patchScreenEncoderOverride = 0;
    auxScreenEncoderOverride = 0;
    ledFlashCounter = 0;
    ledColor = 0;
    setPatchDir(NULL);
    setFirmwareDir(NULL);
    setUserDir(NULL);
    inL = inR = outL = outR = peaks = 0;
    wifiStatus = 0;
    micLineSelection = 99; // so it gets correct initial value

    oled(SCREENSAVER).showInfoBar = false;

}

bool AppData::isPatchHome() {
    return patches_path==getDefaultPatchDir();
}

bool AppData::isSystemHome() {
    return system_path==getDefaultSystemDir(user_path);
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
    system_path = getDefaultSystemDir(user_path);
    
    // stash the user dir in a tmp file for other parts of the system to use (like web apps)
    std::string cmd = std::string("echo ") + user_path + " > /tmp/user_dir";
    system(cmd.c_str());
}

void  AppData::setSystemDir(const char* path) {
    if(path==NULL) {
        system_path=getDefaultSystemDir(user_path);
    } else {
        system_path=path;
    }
}


void AppData::setFirmwareDir(const char* path) {
    if(path==NULL) {
        firmware_path=getDefaultFirwareDir();
        std::cout << "Using FW DIR: " << firmware_path  << std::endl;
    } else {
        firmware_path=path;
    }
    // stash the fw dir in a tmp file for other parts of the system to use (like web apps)
    std::string cmd = std::string("echo ") + firmware_path + " > /tmp/fw_dir";
    system(cmd.c_str());
}

void AppData::resetScreenSaver() {
    if (currentScreen == SCREENSAVER) {
        // Restore previous screen
        currentScreen = previousScreenBeforeSaver;
        oled((AppData::Screen) currentScreen).newScreen = 1;
    }
    screenSaverTimer.reset();
    screenSaverFrame = 0;
}

void AppData::updateScreenSaver() {
    if (currentScreen != SCREENSAVER && screenSaverTimer.getElapsed() > SCREENSAVER_TIMEOUT) {
        // Save current screen and switch to screensaver
        previousScreenBeforeSaver = currentScreen;
        currentScreen = SCREENSAVER;
        screenSaverFrame = 0;
        oled(SCREENSAVER).newScreen = 1;
    }
}

void AppData::drawScreenSaverFrame() {
    OledScreen& screen = oled(SCREENSAVER);
    screen.clear();

    // Static variables to maintain state between calls
    static int circleX = -1;
    static int circleY = -1;
    static int currentRadius = 0;
    static const int maxRadius = 100;
    static const int radiusGrowthRate = 1; // pixels per frame
    static bool growing = true; // true = expanding, false = shrinking

    // Initialize with random position if first time or circle cycle is complete
    if (circleX == -1 || (currentRadius <= 0 && growing)) {
        // Simple pseudo-random number generation
        // Using frame counter as seed for variety
        circleX = (screenSaverFrame * 17 + 23) % 128; // 0-127 for screen width
        circleY = (screenSaverFrame * 13 + 37) % 64;  // 0-63 for screen height
        currentRadius = 0;
        growing = true;
    }

    // Always draw the circle - let it wrap around for cool patterns!
    screen.draw_circle(circleX, circleY, currentRadius, 1);

    // Grow or shrink the circle
    if (growing) {
        currentRadius += radiusGrowthRate;
        if (currentRadius >= maxRadius) {
            growing = false; // Start shrinking
        }
    } else {
        currentRadius -= radiusGrowthRate;
        if (currentRadius <= 0) {
            growing = true; // Will trigger new circle on next frame
        }
    }

    screenSaverFrame++;
    screen.newScreen = 1;
}
