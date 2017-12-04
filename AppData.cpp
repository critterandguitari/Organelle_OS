#include "AppData.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

const char* USB_PATCHES="/usbdrive/Patches";
const char* SD_PATCHES="/sdcard/Patches";
const char* DEFAULT_PATCHES="/usbdrive/Patches";

const char* USB_USERDIR="/usbdrive";
const char* SD_USERDIR="/sdcard";
const char* DEFAULT_USERDIR="/usbdrive";

const char* USB_FW="/usbdrive/Firmware";
const char* SD_FW="/sdcard/Firmware";
const char* DEFAULT_FW="/root";

const char* DEFAULT_ALSA_CONFIG="28:0 128:0 128:1 28:0";

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
    patchIsRunning =false;
    patchIsLoading =false;
    menuScreenTimeout = MENU_TIMEOUT;
    currentScreen = MENU;
    patchScreenEncoderOverride = 0;
    auxScreenEncoderOverride = 0;
    midiChannel = 1;
    useAlsa = false;
    alsaConfig = DEFAULT_ALSA_CONFIG;
    setPatchDir(NULL);
    setFirmwareDir(NULL);
    setUserDir(NULL);
    readMidiConfig();
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
    initMediaDir();
    initDataDir();
    system_path = getDefaultSystemDir(user_path);
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
    } else {
        firmware_path=path;
    }
}

void AppData::readMidiConfig() {
    std::ifstream infile(std::string(user_path+"/MIDI-Config.txt").c_str());
    std::string line;
    while (std::getline(infile, line))
    {
        if(line.length()>0 ) {
            int sep = line.find(" ");
            if(sep!=std::string::npos && sep > 0 && line.length() - sep > 2) {
                std::string param = line.substr(0,sep);
                std::string arg = line.substr(sep + 1, line.length() - sep - 2); // ignore semi colon
                if(param == "channel") {
                    midiChannel = atoi(arg.c_str());
                    printf("using midi channel %d \n", midiChannel);
                } else if (param == "usealsa") {
                    useAlsa = atoi(arg.c_str());
                    printf("useAlsa %d \n", useAlsa);
                } else if (param == "alsaconfig") {
                    alsaConfig = arg;
                    printf("alsa config  %s \n", alsaConfig.c_str());
                }
            }
        }
    }
    infile.close();

}

void AppData::initMediaDir() {
    std::string dir = std::string(getDefaultUserDir()) + "/media";
    std::string mkdircmd = std::string("mkdir -p ") + dir;
    std::string rmcmd = std::string("rm /tmp/media");
    std::string lncmd = std::string("ln -s ")+ dir +std::string(" /tmp/media");
    system(mkdircmd.c_str());
    system(rmcmd.c_str());
    system(lncmd.c_str());
}

void AppData::initDataDir() {
    std::string dir = std::string(getDefaultUserDir()) + "/data";
    std::string mkdircmd = std::string("mkdir -p ") + dir;
    std::string rmcmd = std::string("rm /tmp/data");
    std::string lncmd = std::string("ln -s ")+ dir +std::string(" /tmp/data");
    system(mkdircmd.c_str());
    system(rmcmd.c_str());
    system(lncmd.c_str());
}



