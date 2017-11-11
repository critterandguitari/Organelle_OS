
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>

#include "OSC/OSCMessage.h"
#include "OSC/SimpleWriter.h"
#include "Serial.h"
#include "UdpSocket.h"
#include "SLIPEncodedSerial.h"
#include "OledScreen.h"
#include "MainMenu.h"
#include "Timer.h"
#include "AppData.h"

static const unsigned int MAX_KNOBS = 6;
static int16_t knobs_[MAX_KNOBS];
int       previousScreen = -1;
int       encoderDownTime = -1;
const int SHUTDOWN_TIME = 4;

// for communicating with OSC over serial with MCU
Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;

/*
sockets for communicating OSC with other programs we need 3:
    1) this program receives on 4001
    2) sends to Pd on 4000
    3) sends to Aux program (scripts in the System menu) on 4002
(the destinations are set below)
*/
UdpSocket udpSock(4001);
UdpSocket udpSockAux(4003);

// global app states, flags, screens, etc...
AppData app;

// main menu
MainMenu menu;

// exit flag
int quit = 0;

void setEnv() {
    setenv("PATCH_DIR", app.getPatchDir(), 1);
    setenv("FW_DIR", app.getFirmwareDir(), 1);
    setenv("USER_DIR", app.getUserDir(), 1);
}


int execScript(const char* cmd) {
    char buf[128];
    sprintf(buf, "%s/scripts/%s", app.getFirmwareDir(), cmd);
    setEnv();
    return system(buf);
}




/** OSC messages received internally (from PD or other program) **/


// ui messages
void setScreen(OSCMessage &msg);
void vuMeter(OSCMessage &msg);
void setLED(OSCMessage &msg);
void screenShot(OSCMessage &msg);
void enablePatchSubMenu(OSCMessage &msg);
void enableAuxSubMenu(OSCMessage &msg);
void goHome(OSCMessage &msg);


// new style graphics messages
void gShowInfoBar(OSCMessage &msg); // turns the vu meter on / off
void gClear(OSCMessage &msg);
void gSetPixel(OSCMessage &msg);
void gFillArea(OSCMessage &msg);
void gCircle(OSCMessage &msg);
void gLine(OSCMessage &msg);
void gBox(OSCMessage &msg);
void gInvert(OSCMessage &msg);
void gInvertArea(OSCMessage &msg);
void gCharacter(OSCMessage &msg);
void gPrintln(OSCMessage &msg);
void gWaveform(OSCMessage &msg);



// older legacy messages for screen

// messages for patch screen
void setPatchScreenLine1(OSCMessage &msg);
void setPatchScreenLine2(OSCMessage &msg);
void setPatchScreenLine3(OSCMessage &msg);
void setPatchScreenLine4(OSCMessage &msg);
void setPatchScreenLine5(OSCMessage &msg);
void invertScreenLine(OSCMessage &msg);
// messages for aux screen
void setAuxScreenLine0(OSCMessage &msg);
void setAuxScreenLine1(OSCMessage &msg);
void setAuxScreenLine2(OSCMessage &msg);
void setAuxScreenLine3(OSCMessage &msg);
void setAuxScreenLine4(OSCMessage &msg);
void setAuxScreenLine5(OSCMessage &msg);
void invertAuxScreenLine(OSCMessage &msg);
void auxScreenClear(OSCMessage &msg);


// system message
void loadPatch(OSCMessage &msg);
void midiConfig(OSCMessage &msg);
void patchLoaded(OSCMessage &msg);
void reload(OSCMessage &msg);
void sendReady(OSCMessage &msg);
void sendShutdown(OSCMessage &msg);
void quitMother(OSCMessage &msg);
void programChange(OSCMessage &msg);
/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, and smooth knobs,  key messages get passed righ to PD or other program */
void encoderInput(OSCMessage &msg);
void encoderButton(OSCMessage &msg);
void knobsInput(OSCMessage &msg);
/* end OSC messages received from MCU */

/* helpers */
void updateScreenPage(uint8_t page, OledScreen &screen);
void setScreenLine(OledScreen &screen, int lineNum, OSCMessage &msg);
void sendGetKnobs(void);
void patchLoaded(bool);
/* end helpers */

int main(int argc, char* argv[]) {
    printf("build date " __DATE__ "   " __TIME__ "/n");
    uint32_t seconds = 0;
    char udpPacketIn[256];
    //uint8_t osc_packet_in[256];
    uint8_t i = 0;
    int len = 0;
    int page = 0;

    Timer screenFpsTimer, screenLineTimer, knobPollTimer, pingTimer, upTime;

    screenFpsTimer.reset();
    knobPollTimer.reset();
    screenLineTimer.reset();
    pingTimer.reset();
    upTime.reset();

    app.oled(AppData::MENU).showInfoBar = false;
    app.oled(AppData::AUX).showInfoBar = false;
    app.oled(AppData::PATCH).showInfoBar = true;

    // set locale so sorting happens in right order
    //std::setlocale(LC_ALL, "en_US.UTF-8");

    // for setting real time scheduling
    /*struct sched_param par;

    par.sched_priority = 10;
    printf("settin priority to: %d\n", 10);
    if (sched_setscheduler(0,SCHED_FIFO,&par) < 0){
        printf("failed to set rt scheduling\n");
    }*/

    udpSock.setDestination(4000, "localhost");
    udpSockAux.setDestination(4002, "localhost"); // for sending encoder to aux program
    OSCMessage msgIn;

    menu.buildMenu();

    // send ready to wake up MCU
    // MCU is ignoring stuff over serial port until this message comes through
    // don't empty the message because it gets sent out periodically incase MCU resets
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    // send it a few times just in case
    for (i = 0; i < 4; i++) {
        slip.sendMessage(dump.buffer, dump.length, serial);
        usleep(20000); // wait 20 ms
    }

    quit = 0;

    // full udp -> serial -> serial -> udp
    for (;;) {
        // receive udp, send to serial
        len = udpSock.readBuffer(udpPacketIn, 256, 0);
        if (len > 0) {
            msgIn.empty();
            for (i = 0; i < len; i++) {
                msgIn.fill(udpPacketIn[i]);
            }
            if (!msgIn.hasError()) {
                // or'ing will do lazy eval, i.e. as soon as one succeeds it will stop
                bool processed =
                    msgIn.dispatch("/oled/vumeter", vuMeter, 0)
                    || msgIn.dispatch("/oled/gShowInfoBar", gShowInfoBar, 0)
                    || msgIn.dispatch("/oled/gClear", gClear, 0)
                    || msgIn.dispatch("/oled/gSetPixel", gSetPixel, 0)
                    || msgIn.dispatch("/oled/gFillArea", gFillArea, 0)
                    || msgIn.dispatch("/oled/gCircle", gCircle, 0)
                    || msgIn.dispatch("/oled/gLine", gLine, 0)
                    || msgIn.dispatch("/oled/gBox", gBox, 0)
                    || msgIn.dispatch("/oled/gInvert", gInvert, 0)
                    || msgIn.dispatch("/oled/gCharacter", gCharacter, 0)
                    || msgIn.dispatch("/oled/gPrintln", gPrintln, 0)
                    || msgIn.dispatch("/oled/gWaveform", gWaveform, 0)
                    || msgIn.dispatch("/oled/gInvertArea", gInvertArea, 0)

                    || msgIn.dispatch("/oled/line/1", setPatchScreenLine1, 0)
                    || msgIn.dispatch("/oled/line/2", setPatchScreenLine2, 0)
                    || msgIn.dispatch("/oled/line/3", setPatchScreenLine3, 0)
                    || msgIn.dispatch("/oled/line/4", setPatchScreenLine4, 0)
                    || msgIn.dispatch("/oled/line/5", setPatchScreenLine5, 0)
                    || msgIn.dispatch("/oled/invertline", invertScreenLine, 0)
                    || msgIn.dispatch("/oled/aux/line/1", setAuxScreenLine1, 0)
                    || msgIn.dispatch("/oled/aux/line/2", setAuxScreenLine2, 0)
                    || msgIn.dispatch("/oled/aux/line/3", setAuxScreenLine3, 0)
                    || msgIn.dispatch("/oled/aux/line/4", setAuxScreenLine4, 0)
                    || msgIn.dispatch("/oled/aux/line/5", setAuxScreenLine5, 0)
                    || msgIn.dispatch("/oled/aux/invertline", invertAuxScreenLine, 0)
                    || msgIn.dispatch("/oled/aux/clear", auxScreenClear, 0)

                    || msgIn.dispatch("/ready", sendReady, 0)
                    || msgIn.dispatch("/shutdown", sendShutdown, 0)
                    || msgIn.dispatch("/led", setLED, 0)
                    || msgIn.dispatch("/oled/setscreen", setScreen, 0)
                    || msgIn.dispatch("/reload", reload, 0)
                    || msgIn.dispatch("/quitmother", quitMother, 0)
                    || msgIn.dispatch("/screenshot", screenShot, 0)
                    || msgIn.dispatch("/pgmchg", programChange, 0)
                    || msgIn.dispatch("/gohome", goHome, 0)
                    || msgIn.dispatch("/enablepatchsub", enablePatchSubMenu, 0)
                    || msgIn.dispatch("/enableauxsub", enableAuxSubMenu, 0)
                    || msgIn.dispatch("/loadPatch", loadPatch, 0)
                    || msgIn.dispatch("/midiConfig", midiConfig, 0)
                    || msgIn.dispatch("/patchLoaded", patchLoaded, 0)
                    ;
                if (!processed) {
                    char buf[128];
                    msgIn.getAddress(buf,0,128);
                    fprintf(stderr, "unrecognised osc message received %s %i\n",buf,msgIn.size());
                }
            }
            else {
                fprintf(stderr, "osc message has error \n ");
            }
            msgIn.empty();
        }

        // receive serial, send udp
        if (slip.recvMessage(serial)) {

            // check if we need to do something with this message
            msgIn.empty();
            msgIn.fill(slip.decodedBuf, slip.decodedLength);
            bool knobs = msgIn.dispatch("/knobs", knobsInput, 0);

            if(!knobs) {
                udpSock.writeBuffer(slip.decodedBuf, slip.decodedLength);
            }

            bool processed = 
                msgIn.dispatch("/enc", encoderInput, 0)
            ||  msgIn.dispatch("/encbut", encoderButton, 0);

            msgIn.empty();
        }

        // sleep for .5ms
        usleep(750);

        if (app.currentScreen == AppData::AUX) {
            // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f) {
                screenFpsTimer.reset();
                if (app.newScreen) {
                    app.newScreen = 0;
                    updateScreenPage(0, app.oled(AppData::AUX));
                    updateScreenPage(1, app.oled(AppData::AUX));
                    updateScreenPage(2, app.oled(AppData::AUX));
                    updateScreenPage(3, app.oled(AppData::AUX));
                    updateScreenPage(4, app.oled(AppData::AUX));
                    updateScreenPage(5, app.oled(AppData::AUX));
                    updateScreenPage(6, app.oled(AppData::AUX));
                    updateScreenPage(7, app.oled(AppData::AUX));
                }
            }
        }
        else if (app.currentScreen == AppData::MENU) {
            // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f) {
                screenFpsTimer.reset();
                if (app.newScreen) {
                    app.newScreen = 0;
                    updateScreenPage(0, app.oled(AppData::MENU));
                    updateScreenPage(1, app.oled(AppData::MENU));
                    updateScreenPage(2, app.oled(AppData::MENU));
                    updateScreenPage(3, app.oled(AppData::MENU));
                    updateScreenPage(4, app.oled(AppData::MENU));
                    updateScreenPage(5, app.oled(AppData::MENU));
                    updateScreenPage(6, app.oled(AppData::MENU));
                    updateScreenPage(7, app.oled(AppData::MENU));
                }

                // don't timeout to patch screen, whilst holding down encoder
                if (encoderDownTime == -1) {
                    // if there is a patch running while on menu screen, switch back to patch screen after the timeout
                    if (app.isPatchRunning() || app.isPatchLoading()) {
                        if (app.menuScreenTimeout > 0) app.menuScreenTimeout -= 50;
                        else {
                            app.currentScreen = AppData::PATCH;
                            app.newScreen = 1;
                        }
                    }
                }
            }
        }
        else if (app.currentScreen == AppData::PATCH) {
            if (screenFpsTimer.getElapsed() > 50.f) {
                screenFpsTimer.reset();
                if (app.newScreen) {
                    app.newScreen = 0;
                    updateScreenPage(0, app.oled(AppData::PATCH));
                    updateScreenPage(1, app.oled(AppData::PATCH));
                    updateScreenPage(2, app.oled(AppData::PATCH));
                    updateScreenPage(3, app.oled(AppData::PATCH));
                    updateScreenPage(4, app.oled(AppData::PATCH));
                    updateScreenPage(5, app.oled(AppData::PATCH));
                    updateScreenPage(6, app.oled(AppData::PATCH));
                    updateScreenPage(7, app.oled(AppData::PATCH));
                }
            }
        }

        // every 1 second do (slwo) periodic tasks
        if (pingTimer.getElapsed() > 1000.f) {
            // printf("pinged the MCU at %f ms.\n", upTime.getElapsed());
            // send a ping in case MCU resets
            pingTimer.reset();
            rdyMsg.send(dump);
            slip.sendMessage(dump.buffer, dump.length, serial);

            // check for shutdown shortcut
            if (encoderDownTime != -1) {
                encoderDownTime--;
                if (encoderDownTime == 1) {
                    app.oled(AppData::AUX).clear();
                    app.oled(AppData::AUX).setLine(2, "HOLD to shutdown");
                    app.oled(AppData::AUX).setLine(4, "release to abort");
                    app.newScreen = 1;
                    previousScreen = app.currentScreen;
                    app.currentScreen = AppData::AUX;
                }
                else if (encoderDownTime == 0) {
                    fprintf(stderr, "shutting down.....\n");
                    app.oled(AppData::AUX).clear();
                    app.oled(AppData::AUX).setLine(3, "Shutting down");
                    app.newScreen = 1;
                    menu.runShutdown(0, 0);
                }
            }

            // check for patch loading timeout
            if (app.hasPatchLoadingTimedOut(1000)) {
                fprintf(stderr, "timeout: Patch did not return patchLoaded , will assume its loaded\n");
                patchLoaded(true);
            }

        }

        // poll for knobs
        if (knobPollTimer.getElapsed() > 40.f) {
            knobPollTimer.reset();
            sendGetKnobs();
        }

        // check exit flag
        if (quit) {
            printf("quitting\n");
            return 0;
        }
    } // for;;
}

/** OSC messages received internally (from PD or other program) **/
// settin patch screen
void setPatchScreenLine1(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::PATCH), 1, msg);
    app.newScreen = 1;
}
void setPatchScreenLine2(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::PATCH), 2, msg);
    app.newScreen = 1;
}
void setPatchScreenLine3(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::PATCH), 3, msg);
    app.newScreen = 1;
}
void setPatchScreenLine4(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::PATCH), 4, msg);
    app.newScreen = 1;
}
void setPatchScreenLine5(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::PATCH), 5, msg);
    app.newScreen = 1;
}

inline AppData::Screen gScreen(unsigned s) {
    return (s - 1) < AppData::SCREEN_MAX ? (AppData::Screen) (s - 1) : AppData::PATCH;
}
// graphics for patch screen
void gShowInfoBar(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1)) {
        app.oled(gScreen(msg.getInt(0))).showInfoBar = (msg.getInt(1));
        app.newScreen = 1;
    }
}
void gClear(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1)) {
        if (msg.getInt(1) == 1) app.oled(gScreen(msg.getInt(0))).clear();
        app.newScreen = 1;
    }
}

void gInvert(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1)) {
        if (msg.getInt(1) == 1) app.oled(gScreen(msg.getInt(0))).invert_screen();
        app.newScreen = 1;
    }
}

void gSetPixel(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) ) {
        app.oled(gScreen(msg.getInt(0))).put_pixel(msg.getInt(1), msg.getInt(2), msg.getInt(3));
        app.newScreen = 1;
    }
}
void gFillArea(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        app.oled(gScreen(msg.getInt(0))).fill_area(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        app.newScreen = 1;
    }
}

void gInvertArea(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        app.oled(gScreen(msg.getInt(0))).invert_area(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        app.newScreen = 1;
    }
}

void gWaveform(OSCMessage &msg) {
    uint8_t tmp[132];
    int len = 0;
    int i;
    if (msg.isInt(0) && msg.isBlob(1)) {
        len = msg.getBlob(1, tmp, 132);
        // only if we got 128 values (len and tmp includes the 4 size bytes of blob)
        if (len == 132) {
            // draw 127 connected lines
            for (i = 1; i < 128; i++) {
                app.oled(gScreen(msg.getInt(0))).draw_line(i - 1, tmp[i + 3], i, tmp[i + 4], 1);
            }
            app.newScreen = 1;
        }
    }
}

void gCircle(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        app.oled(gScreen(msg.getInt(0))).draw_circle(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        app.newScreen = 1;
    }
}
void gLine(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        app.oled(gScreen(msg.getInt(0))).draw_line(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        app.newScreen = 1;
    }
}

void gPrintln(OSCMessage &msg) {

    char str[256];
    char line[256];
    int i;
    int x, y, height, color;

    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        i = 1;
        x = msg.getInt(i++);
        y = msg.getInt(i++);
        height = msg.getInt(i++);
        color = msg.getInt(i++);
        // since there are no strings in pd, the line message will be made of different types
        // cat the line together, then throw it up on the patch screen
        line[0] = 0;
        while (msg.isString(i) || msg.isFloat(i) || msg.isInt(i)) {
            if (msg.isString(i)) {
                msg.getString(i, str, 256);
                strcat(line, str);
                strcat(line, " ");
            }
            else if (msg.isFloat(i)) {
                sprintf(str, "%g ", msg.getFloat(i));
                strcat(line, str);
            }
            else if (msg.isInt(i)) {
                sprintf(str, "%d ", msg.getInt(i));
                strcat(line, str);
            }
            i++;
        }
        app.oled(gScreen(msg.getInt(0))).println(line, x, y, height, color);
        app.newScreen = 1;
    }
}

void gBox(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        app.oled(gScreen(msg.getInt(0))).draw_box(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        app.newScreen = 1;
    }
}
void gCharacter(OSCMessage &msg) {
    int size = 8;
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        size = msg.getInt(5);
        if (size == 8) app.oled(gScreen(msg.getInt(0))).put_char_small(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 16) app.oled(gScreen(msg.getInt(0))).put_char_arial16(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 24) app.oled(gScreen(msg.getInt(0))).put_char_arial24(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 32) app.oled(gScreen(msg.getInt(0))).put_char_arial32(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        app.newScreen = 1;
    }
}

// setting aux screen
void setAuxScreenLine1(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::AUX), 1, msg);
    app.newScreen = 1;
}
void setAuxScreenLine2(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::AUX), 2, msg);
    app.newScreen = 1;
}
void setAuxScreenLine3(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::AUX), 3, msg);
    app.newScreen = 1;
}
void setAuxScreenLine4(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::AUX), 4, msg);
    app.newScreen = 1;
}
void setAuxScreenLine5(OSCMessage &msg) {
    setScreenLine(app.oled(AppData::AUX), 5, msg);
    app.newScreen = 1;
}
void auxScreenClear(OSCMessage &msg) {
    app.oled(AppData::AUX).clear();
    app.newScreen = 1;
}

void screenShot(OSCMessage &msg) {
    if (app.currentScreen == AppData::AUX)
        app.oled(AppData::AUX).saveSVG("/usbdrive/AuxScreen.svg");

    if (app.currentScreen == AppData::MENU)
        app.oled(AppData::MENU).saveSVG("/usbdrive/MenuScreen.svg");

    if (app.currentScreen == AppData::PATCH)
        app.oled(AppData::PATCH).saveSVG("/usbdrive/PatchScreen.svg");
}

void programChange(OSCMessage &msg) {
    if (msg.isInt(0)) menu.programChange(msg.getInt(0));
}

void quitMother(OSCMessage &msg) {
    quit = 1;
}

void setLED(OSCMessage &msg) {
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void vuMeter(OSCMessage &msg) {
    static int count;

    char line[1024];
    int len, i, outR, outL, inR, inL;

    if (msg.isInt(0)) inR = msg.getInt(0);
    if (msg.isInt(1)) inL = msg.getInt(1);
    if (msg.isInt(2)) outR = msg.getInt(2);
    if (msg.isInt(3)) outL = msg.getInt(3);

    if (app.oled((AppData::Screen) app.currentScreen).showInfoBar) {
        app.oled((AppData::Screen) app.currentScreen).drawInfoBar(inR, inL, outR, outL);
        app.newScreen = 1;
    }
}

void setScreen(OSCMessage &msg) {
    if (msg.isInt(0)) app.currentScreen = msg.getInt(0) - 1;
    app.newScreen = 1;
}

void reload(OSCMessage &msg) {
    printf("received reload msg\n");
    menu.buildMenu();
}

void sendReady(OSCMessage &msg) {
    printf("sending ready...\n");
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void sendShutdown(OSCMessage &msg) {
    printf("sending shutdown...\n");
    OSCMessage rdyMsg("/shutdown");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void loadPatch(OSCMessage &msg) {
    char patchName[256];
    int i = 0;

    // loop over the patches and jump to the correct one
    if (msg.isString(0)) {
        msg.getString(0, patchName, 256);
        menu.loadPatch(patchName);
    }
}

void midiConfig(OSCMessage &msg) {
    app.readMidiConfig();
    OSCMessage msgOut("/midich");
    msgOut.add(app.getMidiChannel());
    msgOut.send(dump);
    udpSock.writeBuffer(dump.buffer, dump.length);
}

void patchLoaded(bool b) {
    //patch is loaded, tell patch some config details
    app.setPatchLoading(false);
    app.setPatchRunning(true);
    printf("patch loaded, send config");

    {
        // send patch midi channel to use
        OSCMessage msgOut("/midich");
        msgOut.add(app.getMidiChannel());
        msgOut.send(dump);
        udpSock.writeBuffer(dump.buffer, dump.length);
    }

    // if using alsa, connect alsa device to PD virtual device
    if (app.isAlsa()) {
        std::string cmd = "alsaconnect.sh " + app.getAlsaConfig() + " & ";
        execScript(cmd.c_str());
    }

    {
        // send current knob positions
        OSCMessage msgOut("/knobs");
        for(unsigned i = 0; i < MAX_KNOBS;i++) {
            msgOut.add(knobs_[i]);
        }
        msgOut.send(dump);
        udpSock.writeBuffer(dump.buffer, dump.length);        
    }
}

void patchLoaded(OSCMessage &msg) {
    patchLoaded(true);
}


void invertScreenLine(OSCMessage &msg) {
    if (msg.isInt(0)) {
        // + 1 for backwards compatibility
        int line = msg.getInt(0);
        //printf("inverting %d\n", line);
        app.oled(AppData::PATCH).invertLine((line % 5)+1);
        app.newScreen = 1;
    }
}

void invertAuxScreenLine(OSCMessage &msg) {
    if (msg.isInt(0)) {
        // + 1 for backwards compatibility
        int line = msg.getInt(0);
        //printf("inverting %d\n", line);
        app.oled(AppData::AUX).invertLine((line % 5)+1);
        app.newScreen = 1;
    }
}

void goHome(OSCMessage &msg ) {
    printf("returning to main menu\n");
    app.currentScreen = AppData::MENU;
    app.newScreen = 1;
    app.menuScreenTimeout = MENU_TIMEOUT;

}

void enablePatchSubMenu(OSCMessage &msg ) {
    int v = 1;
    if (msg.isInt(0)) { v = msg.getInt(0);}
    printf("enabling patch sub menu %d\n", v);
    app.setPatchScreenEncoderOverride(v);
}

void enableAuxSubMenu(OSCMessage &msg ) {
    int v = 1;
    if (msg.isInt(0)) { v = msg.getInt(0);}
    printf("enabling aux sub menu %d\n", v);
    app.setAuxScreenEncoderOverride(v);
}

/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, the key and knob messages get passed righ to PD or other program */

// this is when the encoder gets turned
// in menu screen, just navigate the menu
// in patch screen, bounce back to menu, unless override is on
// in aux screen, same
void encoderInput(OSCMessage &msg) {
    // if encoder is turned, abort shutdown timer
    encoderDownTime = -1;
    if (previousScreen >= 0) {
        app.currentScreen = previousScreen;
        previousScreen = -1;
        app.newScreen = 1;
    }

    if (app.currentScreen == AppData::MENU) {
        if (msg.isInt(0)) {
            app.menuScreenTimeout = MENU_TIMEOUT;
            if (msg.getInt(0) == 1) menu.encoderUp();
            if (msg.getInt(0) == 0) menu.encoderDown();
        }
    }
    // if in patch mode, send encoder, but only if the patch said it wants encoder access
    if (app.currentScreen == AppData::PATCH) {
        if (msg.isInt(0)) {
            if (app.isPatchScreenEncoderOverride()) {
                OSCMessage msgOut("/encoder/turn");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSock.writeBuffer(dump.buffer, dump.length);
            }
            else {
                app.currentScreen = AppData::MENU;
                app.menuScreenTimeout = MENU_TIMEOUT;
                app.newScreen = 1;
            }
        }
    }
    // same for aux screen
    if (app.currentScreen == AppData::AUX) {
        if (msg.isInt(0)) {
            if (app.isAuxScreenEncoderOverride()) {
                OSCMessage msgOut("/encoder/turn");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSockAux.writeBuffer(dump.buffer, dump.length);
            }
            else {
                app.currentScreen = AppData::MENU;
                app.menuScreenTimeout = MENU_TIMEOUT;
                app.newScreen = 1;
            }
        }
    }
}

void knobsInput(OSCMessage &msg) {
    bool changed = false;
    // knob 1-4 + volume + expr , all 0-1023
    for(unsigned i = 0; i < MAX_KNOBS;i++) {
        if(msg.isInt(i)) {
            int16_t v = msg.getInt(i);
            if(v==0 || v==1023) {
                // allow extremes
                changed |= v != knobs_[i];
                knobs_[i] = v;
            } else {
                // 75% new value, 25% old value
                int16_t nv = (v >> 1) + (v >> 2) + (knobs_[i] >> 2);
                int diff = nv - knobs_[i];
                if(diff>2 || diff <-2) {
                    changed = true;
                    knobs_[i] = nv;
                }
            }
        }
    }
    if(changed) {
        OSCMessage msgOut("/knobs");
        for(unsigned i = 0; i < MAX_KNOBS;i++) {
            msgOut.add(knobs_[i]);
        }
        msgOut.send(dump);
        udpSock.writeBuffer(dump.buffer, dump.length);        
    }
}

// this is when the encoder gets pressed
// in menu screen, execute the menu entry
// in patch screen, bounce back to menu, unless override is on
// in aux screen, same
void encoderButton(OSCMessage &msg) {
    if ( !  ( (app.currentScreen == AppData::PATCH && app.isPatchScreenEncoderOverride())
              || (app.currentScreen == AppData::AUX && app.isAuxScreenEncoderOverride()))) {

        if (msg.isInt(0)) {
            if (msg.getInt(0)) {
                if (encoderDownTime == -1) {
                    encoderDownTime = SHUTDOWN_TIME;
                }
            }
            else {
                encoderDownTime = -1;
                if (previousScreen >= 0) {
                    app.currentScreen = previousScreen;
                    previousScreen = -1;
                    app.newScreen = 1;
                }
            }

        } else {
            encoderDownTime = -1;
            if (previousScreen >= 0) {
                app.currentScreen = previousScreen;
                previousScreen = -1;
                app.newScreen = 1;
            }
        }
    }


    if (app.currentScreen == AppData::MENU) {
        if (msg.isInt(0)) {
            if (msg.getInt(0) == 1) {
                menu.encoderPress();
            }
            if (msg.getInt(0) == 0) {
                menu.encoderRelease();
                // reset menu timeout when action is performed
                // (which is when we release encoder)
                app.menuScreenTimeout = MENU_TIMEOUT;
            }
        }
    }

    // if in patch mode, send encoder, but only if the patch said it wants encoder access
    if (app.currentScreen == AppData::PATCH) {
        if (msg.isInt(0)) {
            if (app.isPatchScreenEncoderOverride()) {
                OSCMessage msgOut("/encoder/button");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSock.writeBuffer(dump.buffer, dump.length);
            }
        }
    }
    // same for the aux screen
    if (app.currentScreen == AppData::AUX) {
        if (msg.isInt(0)) {
            if (app.isAuxScreenEncoderOverride()) {
                OSCMessage msgOut("/encoder/button");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSockAux.writeBuffer(dump.buffer, dump.length);
            }
        }
    }
}
/* end OSC messages received from MCU */

/* helpers */
void setScreenLine(OledScreen &screen, int lineNum, OSCMessage &msg) {

    char str[256];
    char screenLine[256];
    int i = 0;

    screenLine[0] = 0;

    // since there are no strings in pd, the line message will be made of different types
    // cat the line together, then throw it up on the patch screen
    while (msg.isString(i) || msg.isFloat(i) || msg.isInt(i)) {
        if (msg.isString(i)) {
            msg.getString(i, str, 256);
            strcat(screenLine, str);
            strcat(screenLine, " ");
        }
        if (msg.isFloat(i)) {
            sprintf(str, "%g ", msg.getFloat(i));
            strcat(screenLine, str);
        }
        if (msg.isInt(i)) {
            sprintf(str, "%d ", msg.getInt(i));
            strcat(screenLine, str);
        }
        i++;
    }
    screen.setLine(lineNum, screenLine);
    //    printf("%s\n", screenLine);
}

void sendGetKnobs(void) {
    OSCMessage msg("/getknobs");
    msg.add(1);
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void updateScreenPage(uint8_t page, OledScreen &screen) {

    uint8_t oledPage[128];
    uint32_t i, j;

    i = page;

    // copy 128 byte page from the screen buffer
    for (j = 0; j < 128; j++) {
        oledPage[j] = screen.pix_buf[j + (i * 128)];
    }
    OSCMessage oledMsg("/oled");
    oledMsg.add(i);
    oledMsg.add(oledPage, 128);
    oledMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    oledMsg.empty();
}
/* end helpers */


