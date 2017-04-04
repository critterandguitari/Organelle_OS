#include "AppData.h"

AppData::AppData(){
    patchIsRunning = 0;
    menuScreenTimeout = MENU_TIMEOUT;
    newScreen = 0;
    currentScreen = MENU;
    patchScreenEncoderOverride = 0;
    auxScreenEncoderOverride = 0;
}


