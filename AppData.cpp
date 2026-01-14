#include "AppData.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cmath>

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
    screenSaverTimer = 0;

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

void  AppData::setSystemDir(const char* path) {
    if(path==NULL) {
        system_path=getDefaultSystemDir(user_path);
    } else {
        system_path=path;
    }
}

void AppData::setUserDir(const char* path) {
    if(path==NULL) {
        user_path=getDefaultUserDir();
    } else {
        user_path=path;
    }
    system_path = getDefaultSystemDir(user_path);
    
    // stash for other parts of system to use
    std::ofstream f("/tmp/user_dir", std::ios::trunc);
    if(f.is_open()) {
        f << user_path << std::endl;
        f.close();
    }
}

void AppData::setFirmwareDir(const char* path) {
    if(path==NULL) {
        firmware_path=getDefaultFirwareDir();
        std::cout << "Using FW DIR: " << firmware_path  << std::endl;
    } else {
        firmware_path=path;
    }
    
    // stash for other parts of system to use
    std::ofstream f("/tmp/fw_dir", std::ios::trunc);
    if(f.is_open()) {
        f << firmware_path << std::endl;
        f.close();
    }
}

void AppData::resetScreenSaver() {
    if (currentScreen == SCREENSAVER) {
        // Restore previous screen
        currentScreen = previousScreenBeforeSaver;
        oled((AppData::Screen) currentScreen).newScreen = 1;
    }
    screenSaverTimer = 0;
    screenSaverFrame = 0;
}

void AppData::updateScreenSaver() {
    screenSaverTimer++;
    if (currentScreen != SCREENSAVER && screenSaverTimer > SCREENSAVER_TIMEOUT) {
        // Save current screen and switch to screensaver
        previousScreenBeforeSaver = currentScreen;
        currentScreen = SCREENSAVER;
        oled(SCREENSAVER).newScreen = 1;
        screenSaverTimer = SCREENSAVER_TIMEOUT;
    }
}

void AppData::drawScreenSaverFrame() {
    OledScreen& screen = oled(SCREENSAVER);
    screen.clear();

    // Static variables to maintain state between calls
    static int circleX = -1;
    static int circleY = -1;
    static float currentRadius = 0.0f;  // Use float for smoother animation
    static const int maxRadius = 100;
    static bool growing = true;
    static int delayFrames = 0;
    static const int DELAY_BETWEEN_CIRCLES = 30; // ~0.5 second delay at 60fps

    // Handle delay between circles
    if (currentRadius <= 0 && growing && delayFrames < DELAY_BETWEEN_CIRCLES) {
        delayFrames++;
        screenSaverFrame++;
        screen.newScreen = 1;
        return; // Don't draw anything during delay
    }

    // Initialize new circle after delay or if first time
    if (screenSaverFrame == 0 || (currentRadius <= 0 && growing && delayFrames >= DELAY_BETWEEN_CIRCLES)) {
        // Simple pseudo-random number generation
        circleX = rand() % 128; // 0-127 for screen width
        circleY = rand() % 64;  // 0-63 for screen height
        currentRadius = 0.0f;
        growing = true;
        delayFrames = 0; // Reset delay counter
    }

    // Draw the circle if radius > 0
    if (currentRadius > 0) {
        screen.draw_circle(circleX, circleY, (int)currentRadius, 1);
    }

    // Calculate growth speed - constant rate like original
    if (growing) {
        currentRadius += 1.0f; // Constant growth rate
        if (currentRadius >= maxRadius) {
            currentRadius = maxRadius;
            growing = false; // Start shrinking
        }
    } else {
        currentRadius -= 1.0f; // Constant shrinking rate
        if (currentRadius <= 0) {
            currentRadius = 0;
            growing = true; // Will trigger delay then new circle
        }
    }

    screenSaverFrame++;
    screen.newScreen = 1;
}
