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
        SCREEN_MAX,
    };

    AppData();
    bool isPatchHome();
    bool isSystemHome();

    // writable space either /usbdrive if present, or /sdcard
    void setUserDir(const char*);
    const char* getUserDir() {return user_path.c_str(); }

    // home = userdir/patches, but changes if using sub directory
    void setPatchDir(const char*);
    const char* getPatchDir() {return patches_path.c_str(); }

    // userdir/System
    const char*  getSystemDir() { return system_path.c_str();}
    void  setSystemDir(const char*);

    // media directory
    const char* getMediaDir() { return "/tmp/media";}
    // data directory
    const char* getDataDir() { return "/tmp/data";}

    // mother/mother.pd , scripts
    const char* getFirmwareDir() {return firmware_path.c_str(); }
    void setFirmwareDir(const char*);

    const char* getCurrentPatch() { return currentPatch;}
    const char* getCurrentPatchPath() { return currentPatchPath;}

    void readMidiConfig();

    int getMidiChannel() { return midiChannel;}
    bool isAlsa() { return useAlsa;}
    std::string getAlsaConfig() { return alsaConfig;}

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


    char currentPatch[256];
    char currentPatchPath[256];

    unsigned currentScreen;          // the current screen (AUX, MENU or PATCH)
    int menuScreenTimeout;

private:
    OledScreen oleds_[SCREEN_MAX];

    void initMediaDir();
    void initDataDir();

    int patchScreenEncoderOverride;  // when 1, encoder input is ignored in menu scree, routed to patch
    int auxScreenEncoderOverride; // when 1, encoder input is routed to aux
    bool patchIsLoading;
    bool patchIsRunning;         // if an actual patch is running
    int  patchLoadingTimeout;
    std::string patches_path;
    std::string user_path;
    std::string system_path;
    std::string firmware_path;
    int midiChannel;
    bool useAlsa;
    std::string alsaConfig;
};


#endif
