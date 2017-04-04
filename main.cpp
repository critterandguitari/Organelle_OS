
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
#include "UI.h"
#include "Timer.h"
#include "AppData.h"

Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;

// for communicating with Pd or other program
UdpSocket udpSock(4001);
UdpSocket udpSockAux(4003); // sends to aux program reciever

// global app data, state, flags, screens
AppData app;

// main menu
UI ui(&app);

// exit flag
int quit = 0;

/** OSC messages received internally (from PD or other program) **/
void setPatchScreenLine1(OSCMessage &msg);
void setPatchScreenLine2(OSCMessage &msg);
void setPatchScreenLine3(OSCMessage &msg);
void setPatchScreenLine4(OSCMessage &msg);
void setPatchScreenLine5(OSCMessage &msg);

void setAuxScreenLine0(OSCMessage &msg);
void setAuxScreenLine1(OSCMessage &msg);
void setAuxScreenLine2(OSCMessage &msg);
void setAuxScreenLine3(OSCMessage &msg);
void setAuxScreenLine4(OSCMessage &msg);
void setAuxScreenLine5(OSCMessage &msg);
void auxScreenClear(OSCMessage &msg);

void setLED(OSCMessage &msg);
void vuMeter(OSCMessage &msg);
void setScreen(OSCMessage &msg);
void reload(OSCMessage &msg);
void sendReady(OSCMessage &msg);
void sendShutdown(OSCMessage &msg);
void quitMother(OSCMessage &msg);
void screenShot(OSCMessage &msg);
void programChange(OSCMessage &msg);
void goHome(OSCMessage &msg);
void enablePatchSubMenu(OSCMessage &msg);
void enableAuxSubMenu(OSCMessage &msg);
void invertScreenLine(OSCMessage &msg);
/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, the key and knob messages get passed righ to PD or other program */
void encoderInput(OSCMessage &msg);
void encoderButton(OSCMessage &msg);
/* end OSC messages received from MCU */

/* helpers */
void updateScreenPage(uint8_t page, OledScreen &screen);
void setScreenLine(OledScreen &screen, int lineNum, OSCMessage &msg);
void sendGetKnobs(void);
/* end helpers */

int main(int argc, char* argv[]) {
      
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

    ui.buildMenu();
    ui.drawPatchList();

    // send ready to wake up MCU
    // MCU is ignoring stuff over serial port until this message comes through
    // don't empty the message because it gets sent out periodically incase MCU resets
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    // send it a few times just in case
    for(i = 0; i<4; i++) {
       slip.sendMessage(dump.buffer, dump.length, serial);
       usleep(20000); // wait 20 ms
    }
    
    //playFirst();
    quit = 0;

    // full udp -> serial -> serial -> udp
    for (;;){
        // receive udp, send to serial
        len = udpSock.readBuffer(udpPacketIn, 256, 0);
        if (len > 0){
            msgIn.empty();
            for (i = 0; i < len; i++){
                msgIn.fill(udpPacketIn[i]);
            }    
            if(!msgIn.hasError()){
                msgIn.dispatch("/oled/vumeter", vuMeter, 0);
                msgIn.dispatch("/oled/line/1", setPatchScreenLine1, 0);
                msgIn.dispatch("/oled/line/2", setPatchScreenLine2, 0);
                msgIn.dispatch("/oled/line/3", setPatchScreenLine3, 0);
                msgIn.dispatch("/oled/line/4", setPatchScreenLine4, 0);
                msgIn.dispatch("/oled/line/5", setPatchScreenLine5, 0);
                msgIn.dispatch("/oled/aux/line/1", setAuxScreenLine1, 0);
                msgIn.dispatch("/oled/aux/line/2", setAuxScreenLine2, 0);
                msgIn.dispatch("/oled/aux/line/3", setAuxScreenLine3, 0);
                msgIn.dispatch("/oled/aux/line/4", setAuxScreenLine4, 0);
                msgIn.dispatch("/oled/aux/line/5", setAuxScreenLine5, 0);
                msgIn.dispatch("/oled/aux/clear", auxScreenClear, 0);
                
                msgIn.dispatch("/ready", sendReady, 0);
                msgIn.dispatch("/shutdown", sendShutdown, 0);
                msgIn.dispatch("/led", setLED, 0);
                msgIn.dispatch("/oled/setscreen", setScreen, 0);
                msgIn.dispatch("/reload", reload, 0);
                msgIn.dispatch("/quitmother", quitMother, 0);
                msgIn.dispatch("/screenshot", screenShot, 0);
                msgIn.dispatch("/pgmchg", programChange, 0);
                msgIn.dispatch("/gohome", goHome, 0);
                msgIn.dispatch("/enablepatchsub", enablePatchSubMenu, 0);
                msgIn.dispatch("/enableauxsub", enableAuxSubMenu, 0);
                msgIn.dispatch("/oled/invertline", invertScreenLine, 0);
            }
            else {
                printf("bad message\n");
            }
            msgIn.empty();
        }   

        // receive serial, send udp
        if(slip.recvMessage(serial)) {
            udpSock.writeBuffer(slip.decodedBuf, slip.decodedLength);
            
            // check if we need to do something with this message
            msgIn.empty();
            msgIn.fill(slip.decodedBuf, slip.decodedLength);
            msgIn.dispatch("/enc", encoderInput, 0);
            msgIn.dispatch("/encbut", encoderButton, 0);
            msgIn.empty();
        }

        // sleep for .5ms
        usleep(750);
        
        if (app.currentScreen == AUX) {
             // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f){
                screenFpsTimer.reset();
                if (app.newScreen){
                    app.newScreen = 0;
                    updateScreenPage(0, app.auxScreen);//menuScreen);
                    updateScreenPage(1, app.auxScreen);
                    updateScreenPage(2, app.auxScreen);
                    updateScreenPage(3, app.auxScreen);
                    updateScreenPage(4, app.auxScreen);
                    updateScreenPage(5, app.auxScreen);
                    updateScreenPage(6, app.auxScreen);
                    updateScreenPage(7, app.auxScreen);
                }
            }
        }
        else if (app.currentScreen == MENU) {
             // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f){
                screenFpsTimer.reset();
                if (app.newScreen){
                    app.newScreen = 0;
                    updateScreenPage(0, app.menuScreen);//menuScreen);
                    updateScreenPage(1, app.menuScreen);
                    updateScreenPage(2, app.menuScreen);
                    updateScreenPage(3, app.menuScreen);
                    updateScreenPage(4, app.menuScreen);
                    updateScreenPage(5, app.menuScreen);
                    updateScreenPage(6, app.menuScreen);
                    updateScreenPage(7, app.menuScreen);
                }
                // if there is a patch running while on menu screen, switch back to patch screen after the timeout 
                if (app.patchIsRunning){
                    if (app.menuScreenTimeout > 0) app.menuScreenTimeout -= 50;
                    else {
                        app.currentScreen = PATCH;
                        app.newScreen = 1;
                    }
                }
            }
        }
        else if (app.currentScreen == PATCH) {
            if (screenFpsTimer.getElapsed() > 50.f){
                screenFpsTimer.reset();
                if (app.newScreen){
                    app.newScreen = 0;
                    updateScreenPage(0, app.patchScreen);//menuScreen);
                    updateScreenPage(1, app.patchScreen);
                    updateScreenPage(2, app.patchScreen);
                    updateScreenPage(3, app.patchScreen);
                    updateScreenPage(4, app.patchScreen);
                    updateScreenPage(5, app.patchScreen);
                    updateScreenPage(6, app.patchScreen);
                    updateScreenPage(7, app.patchScreen);
                }
            }
        }
       
        // every 1 second send a ping in case MCU resets
        if (pingTimer.getElapsed() > 1000.f){
          //  printf("pinged the MCU at %f ms.\n", upTime.getElapsed());
            pingTimer.reset();
            rdyMsg.send(dump);
            slip.sendMessage(dump.buffer, dump.length, serial);
        }

        // poll for knobs
        if (knobPollTimer.getElapsed() > 40.f){
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
void setPatchScreenLine1(OSCMessage &msg){
    setScreenLine(app.patchScreen, 1, msg);
    app.newScreen = 1;
}
void setPatchScreenLine2(OSCMessage &msg){
    setScreenLine(app.patchScreen, 2, msg);
    app.newScreen = 1;
}
void setPatchScreenLine3(OSCMessage &msg){
    setScreenLine(app.patchScreen, 3, msg);
    app.newScreen = 1;
}
void setPatchScreenLine4(OSCMessage &msg){
    setScreenLine(app.patchScreen, 4, msg);
    app.newScreen = 1;
}
void setPatchScreenLine5(OSCMessage &msg){
    setScreenLine(app.patchScreen, 5, msg);
    app.newScreen = 1;
}

// setting aux screen
void setAuxScreenLine1(OSCMessage &msg) {
    setScreenLine(app.auxScreen, 1, msg);
}
void setAuxScreenLine2(OSCMessage &msg) {
    setScreenLine(app.auxScreen, 2, msg);
}
void setAuxScreenLine3(OSCMessage &msg) {
    setScreenLine(app.auxScreen, 3, msg);
}
void setAuxScreenLine4(OSCMessage &msg) {
    setScreenLine(app.auxScreen, 4, msg);
}
void setAuxScreenLine5(OSCMessage &msg) {
    setScreenLine(app.auxScreen, 5, msg);
}
void auxScreenClear(OSCMessage &msg) {
    app.auxScreen.clear();
}

void screenShot(OSCMessage &msg){
    if (app.currentScreen == AUX) 
        app.auxScreen.saveSVG("/usbdrive/AuxScreen.svg");
    
    if (app.currentScreen == MENU) 
        app.menuScreen.saveSVG("/usbdrive/MenuScreen.svg");
    
    if (app.currentScreen == PATCH) 
        app.patchScreen.saveSVG("/usbdrive/PatchScreen.svg");
}

void programChange(OSCMessage &msg){
    if (msg.isInt(0)) ui.programChange(msg.getInt(0));
}

void quitMother(OSCMessage &msg){
    quit = 1;
}

void setLED(OSCMessage &msg){
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void vuMeter(OSCMessage &msg){
    static int count;

    char line[1024];
    int len, i, outR, outL, inR, inL;    

    if (msg.isInt(0)) inR = msg.getInt(0);
    if (msg.isInt(1)) inL = msg.getInt(1);
    if (msg.isInt(2)) outR = msg.getInt(2);
    if (msg.isInt(3)) outL = msg.getInt(3);

    app.patchScreen.drawInfoBar(inR, inL, outR, outL);
    app.newScreen = 1;
}

void setScreen(OSCMessage &msg){
    if (msg.isInt(0)) app.currentScreen = msg.getInt(0);
    app.newScreen = 1;
}

void reload(OSCMessage &msg){
    printf("received reload msg\n");
    ui.buildMenu();
}

void sendReady(OSCMessage &msg){   
    printf("sending ready...\n");
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void sendShutdown(OSCMessage &msg){
    printf("sending shutdown...\n");
    OSCMessage rdyMsg("/shutdown");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void invertScreenLine(OSCMessage &msg){
    
    if (msg.isInt(0)){
        int line = msg.getInt(0);
        //printf("inverting %d\n", line);
        app.patchScreen.invertLine(line % 5);
        app.newScreen = 1;
    }
}

void goHome(OSCMessage &msg ) {
    printf("returning to main menu\n");
    app.currentScreen = MENU;
    app.newScreen = 1;
    app.menuScreenTimeout = MENU_TIMEOUT;

}

void enablePatchSubMenu(OSCMessage &msg ) {
    printf("enabling patch sub menu\n");
    app.patchScreenEncoderOverride = 1;
}

void enableAuxSubMenu(OSCMessage &msg ) {
    printf("enabling aux sub menu\n");
    app.auxScreenEncoderOverride = 1;
}

/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, the key and knob messages get passed righ to PD or other program */

// this is when the encoder gets turned 
// in menu screen, just navigate the menu
// in patch screen, bounce back to menu, unless override is on 
// in aux screen, same
void encoderInput(OSCMessage &msg){
    if (app.currentScreen == MENU){
        if (msg.isInt(0)){
            app.menuScreenTimeout = MENU_TIMEOUT;
            if (msg.getInt(0) == 1) ui.encoderUp();
            if (msg.getInt(0) == 0) ui.encoderDown();
        }
    }
    // if in patch mode, send encoder, but only if the patch said it wants encoder access
    if (app.currentScreen == PATCH){
        if (msg.isInt(0)){
            if (app.patchScreenEncoderOverride){
                OSCMessage msgOut("/encoder/turn");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSock.writeBuffer(dump.buffer, dump.length);
            }
            else {
                app.currentScreen = MENU;
                app.menuScreenTimeout = MENU_TIMEOUT;
                app.newScreen = 1;
            }
        }
    }
    // same for aux screen
    if (app.currentScreen == AUX){
        if (msg.isInt(0)){
            if (app.auxScreenEncoderOverride){
                OSCMessage msgOut("/encoder/turn");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSockAux.writeBuffer(dump.buffer, dump.length);
            }
            else {
                app.currentScreen = MENU;
                app.menuScreenTimeout = MENU_TIMEOUT;
                app.newScreen = 1;
            }
        }
    }
}

// this is when the encoder gets pressed 
// in menu screen, execute the menu entry
// in patch screen, bounce back to menu, unless override is on 
// in aux screen, same
void encoderButton(OSCMessage &msg){
    if (app.currentScreen == MENU){
        if (msg.isInt(0)){
            if (msg.getInt(0) == 1) {
                ui.encoderPress();
            }
            if (msg.getInt(0) == 0) {
                ui.encoderRelease();
            }
        }   
    }

    // if in patch mode, send encoder, but only if the patch said it wants encoder access
    if (app.currentScreen == PATCH){
        if (msg.isInt(0)){
            if (app.patchScreenEncoderOverride){
                OSCMessage msgOut("/encoder/button");
                msgOut.add(msg.getInt(0));
                msgOut.send(dump);
                udpSock.writeBuffer(dump.buffer, dump.length);
            }
        }
    }
    // same for the aux screen 
    if (app.currentScreen == AUX){
         if (msg.isInt(0)){
            if (app.auxScreenEncoderOverride){
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
void setScreenLine(OledScreen &screen, int lineNum, OSCMessage &msg){

    char str[256];
    char screenLine[256];
    int i = 0;

    screenLine[0] = 0;
    
    // since there are no strings in pd, the line message will be made of different types
    // cat the line together, then throw it up on the patch screen
    while (msg.isString(i) || msg.isFloat(i) || msg.isInt(i)){
        if (msg.isString(i)){
            msg.getString(i, str, 256);
            strcat(screenLine, str);
            strcat(screenLine, " ");
        }
        if (msg.isFloat(i)){
            sprintf(str, "%g ", msg.getFloat(i));
            strcat(screenLine, str);
        }
        if (msg.isInt(i)){
            sprintf(str, "%d ", msg.getInt(i));
            strcat(screenLine, str);
        }
        i++;
    }
    screen.setLine(lineNum, screenLine);
    //    printf("%s\n", screenLine);
}

void sendGetKnobs(void){
    OSCMessage msg("/getknobs");
    msg.add(1);
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void updateScreenPage(uint8_t page, OledScreen &screen){
    
    uint8_t oledPage[128];
    uint32_t i, j;

    i = page;

        // copy 128 byte page from the screen buffer
        for (j=0; j<128; j++){
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


