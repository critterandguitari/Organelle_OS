
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

Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;

// for communicating with Pd or other program
UdpSocket udpSock(4001);

// main menu interface program
UI ui;

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
    OSCMessage msgIn;

    ui.loadPatchList();
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
        
        if (ui.currentScreen == AUX) {
             // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f){
                screenFpsTimer.reset();
                if (ui.newScreen){
                    ui.newScreen = 0;
                    updateScreenPage(0, ui.auxScreen);//menuScreen);
                    updateScreenPage(1, ui.auxScreen);
                    updateScreenPage(2, ui.auxScreen);
                    updateScreenPage(3, ui.auxScreen);
                    updateScreenPage(4, ui.auxScreen);
                    updateScreenPage(5, ui.auxScreen);
                    updateScreenPage(6, ui.auxScreen);
                    updateScreenPage(7, ui.auxScreen);
                }
            }
        }
        else if (ui.currentScreen == MENU) {
             // we can do a whole screen,  but not faster than 20fps
            if (screenFpsTimer.getElapsed() > 50.f){
                screenFpsTimer.reset();
                if (ui.newScreen){
                    ui.newScreen = 0;
                    updateScreenPage(0, ui.menuScreen);//menuScreen);
                    updateScreenPage(1, ui.menuScreen);
                    updateScreenPage(2, ui.menuScreen);
                    updateScreenPage(3, ui.menuScreen);
                    updateScreenPage(4, ui.menuScreen);
                    updateScreenPage(5, ui.menuScreen);
                    updateScreenPage(6, ui.menuScreen);
                    updateScreenPage(7, ui.menuScreen);
                }
                // if there is a patch running while on menu screen, switch back to patch screen after the timeout 
                if (ui.menuScreenTimeout > 0) ui.menuScreenTimeout -= 50;
                else ui.currentScreen = PATCH;
            }
        }
        else if (ui.currentScreen == PATCH) {
            if (ui.patchIsRunning) {
                // every 16 ms send a new screen page
                if (screenLineTimer.getElapsed() > 15.f){
                    screenLineTimer.reset();
                    updateScreenPage(page, ui.patchScreen);
                    page++;
                    page %= 8;
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
    setScreenLine(ui.patchScreen, 1, msg);
}
void setPatchScreenLine2(OSCMessage &msg){
    setScreenLine(ui.patchScreen, 2, msg);
}
void setPatchScreenLine3(OSCMessage &msg){
    setScreenLine(ui.patchScreen, 3, msg);
}
void setPatchScreenLine4(OSCMessage &msg){
    setScreenLine(ui.patchScreen, 4, msg);
}
void setPatchScreenLine5(OSCMessage &msg){
    setScreenLine(ui.patchScreen, 5, msg);
}

// setting aux screen
void setAuxScreenLine1(OSCMessage &msg) {
    setScreenLine(ui.auxScreen, 1, msg);
}
void setAuxScreenLine2(OSCMessage &msg) {
    setScreenLine(ui.auxScreen, 2, msg);
}
void setAuxScreenLine3(OSCMessage &msg) {
    setScreenLine(ui.auxScreen, 3, msg);
}
void setAuxScreenLine4(OSCMessage &msg) {
    setScreenLine(ui.auxScreen, 4, msg);
}
void setAuxScreenLine5(OSCMessage &msg) {
    setScreenLine(ui.auxScreen, 5, msg);
}
void auxScreenClear(OSCMessage &msg) {
    ui.auxScreen.clear();
}

void screenShot(OSCMessage &msg){
    if (ui.currentScreen == AUX) 
        ui.auxScreen.saveSVG("/usbdrive/AuxScreen.svg");
    
    if (ui.currentScreen == MENU) 
        ui.menuScreen.saveSVG("/usbdrive/MenuScreen.svg");
    
    if (ui.currentScreen == PATCH) 
        ui.patchScreen.saveSVG("/usbdrive/PatchScreen.svg");
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

    ui.patchScreen.drawInfoBar(inR, inL, outR, outL);

}

void setScreen(OSCMessage &msg){
    if (msg.isInt(0)) ui.currentScreen = msg.getInt(0);
    ui.newScreen = 1;
}

void reload(OSCMessage &msg){
    printf("received reload msg\n");
    ui.loadPatchList();
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
/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, the key and knob messages get passed righ to PD or other program */
void encoderInput(OSCMessage &msg){
    if (msg.isInt(0)){
        if (msg.getInt(0) == 1) ui.encoderUp();
        if (msg.getInt(0) == 0) ui.encoderDown();
    }
}

void encoderButton(OSCMessage &msg){
    if (msg.isInt(0)){
        if (msg.getInt(0) == 1) {
            ui.encoderPress();
        }
        if (msg.getInt(0) == 0) {
            ui.encoderRelease();
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


