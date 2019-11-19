#ifndef APPDATA_H
#define APPDATA_H

#include <stdint.h>
#include <string>

#include "OledScreen.h"

// the UI provides 3 screens

#define MENU_TIMEOUT 2000  // m sec timeout when screen switches back to patch detail

class AppData
{
public:
    enum Screen {
        AUX,
        MENU,
        PATCH,
        ALERT,
        SCREEN_MAX,
    };

    AppData();
    bool isPatchHome();
    bool isSystemHome();

    // writable space either /usbdrive if present, or /sdcard
    void setUserDir(const char*);
    const std::string&  getUserDir() {return user_path; }

    // home = userdir/patches, but changes if using sub directory
    void setPatchDir(const char*);
    const std::string&  getPatchDir() {return patches_path; }

    // userdir/System
    const std::string&   getSystemDir() { return system_path;}
    void  setSystemDir(const char*);

    // mother/mother.pd , scripts
    const std::string& getFirmwareDir() {return firmware_path; }
    void setFirmwareDir(const char*);

    const char* getCurrentPatch() { return currentPatch;}
    const char* getCurrentPatchPath() { return currentPatchPath;}

    bool isPatchRunning() { return patchIsRunning;}
    void setPatchRunning( bool b) {
        patchIsRunning = b;
        patchLoadingTimeout = 0;
    }
    bool isPatchLoading() { return patchIsLoading;}
    void setPatchLoading(bool b) {
        patchIsLoading = b;
        if (b) {
            // 2 seconds default timeout
            patchLoadingTimeout = 2000;
        } else {
            patchLoadingTimeout = 0;
        }
    }

    inline bool hasPatchLoadingTimedOut(int msec) {
        if (patchIsLoading) {
            patchLoadingTimeout -= msec;
            return patchLoadingTimeout < 0;
        }
        return false;
    }

    bool isPatchScreenEncoderOverride() { return patchScreenEncoderOverride;}
    void setPatchScreenEncoderOverride(bool v) { patchScreenEncoderOverride = v;}
    bool isAuxScreenEncoderOverride() {return auxScreenEncoderOverride;}
    void setAuxScreenEncoderOverride(bool v)  {auxScreenEncoderOverride = v;}

    OledScreen& oled(Screen s) { return oleds_[s];}

    unsigned micLineSelection;
    unsigned ledColor;              // current led color
    unsigned ledFlashCounter;       // for flashing the led
    
    char currentPatch[256];
    char currentPatchPath[256];

    unsigned currentScreen;          // the current screen (AUX, MENU or PATCH)
    int menuScreenTimeout;

    int inL;                        // vu meters
    int inR;
    int outL;
    int outR;
    int peaks;

    int wifiStatus;              // network connection status

private:
    OledScreen oleds_[SCREEN_MAX];

    int patchScreenEncoderOverride;  // when 1, encoder input is ignored in menu scree, routed to patch
    int auxScreenEncoderOverride; // when 1, encoder input is routed to aux
    bool patchIsLoading;
    bool patchIsRunning;         // if an actual patch is running
    int  patchLoadingTimeout;
    std::string patches_path;
    std::string user_path;
    std::string system_path;
    std::string firmware_path;
};


#endif
