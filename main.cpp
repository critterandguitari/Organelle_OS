
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


Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;

// main menu interface program
UI ui;

/** OSC messages received internally (from PD or other program) **/
void setPatchScreenOledLine1(OSCMessage &msg);
void setPatchScreenOledLine2(OSCMessage &msg);
void setPatchScreenOledLine3(OSCMessage &msg);
void setPatchScreenOledLine4(OSCMessage &msg);
void setPatchScreenOledLine5(OSCMessage &msg);
void setLED(OSCMessage &msg);
void vuMeter(OSCMessage &msg);
void setAuxScreen(OSCMessage &msg);
void reload(OSCMessage &msg);
void sendReady(OSCMessage &msg);
void sendShutdown(OSCMessage &msg);
/* end internal OSC messages received */

/* OSC messages received from MCU (we only use ecncoder input, the key and knob messages get passed righ to PD or other program */
void encoderInput(OSCMessage &msg);
void encoderButton(OSCMessage &msg);
/* end OSC messages received from MCU */

/* helpers */
void updateScreenPage(uint8_t page, OledScreen &screen);
void setPatchScreenOledLine(int lineNum, OSCMessage &msg);
void sendGetKnobs(void);
/* end helpers */

int main(int argc, char* argv[]) {
      
    uint32_t seconds = 0;
    char udpPacketIn[256];
    //uint8_t osc_packet_in[256];
    uint8_t i = 0;
    int len = 0;
    int count = 0;
    int page = 0;
    int count20fps = 0;
    int countReadyPing = 0;
    int countKnobPoll = 0;

    // for setting real time scheduling
    /*struct sched_param par;

    par.sched_priority = 10;
    printf("settin priority to: %d\n", 10);
    if (sched_setscheduler(0,SCHED_FIFO,&par) < 0){
        printf("failed to set rt scheduling\n");
    }*/

    UdpSocket udpSock(4001);
    udpSock.setDestination(4000, "localhost");
    OSCMessage msgIn;

    ui.getPatchList();
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
                msgIn.dispatch("/oled/line/1", setPatchScreenOledLine1, 0);
                msgIn.dispatch("/oled/line/2", setPatchScreenOledLine2, 0);
                msgIn.dispatch("/oled/line/3", setPatchScreenOledLine3, 0);
                msgIn.dispatch("/oled/line/4", setPatchScreenOledLine4, 0);
                msgIn.dispatch("/oled/line/5", setPatchScreenOledLine5, 0);
                msgIn.dispatch("/ready", sendReady, 0);
                msgIn.dispatch("/shutdown", sendShutdown, 0);
                msgIn.dispatch("/led", setLED, 0);
                msgIn.dispatch("/auxscreen", setAuxScreen, 0);
                msgIn.dispatch("/reload", reload, 0);
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

        // sleep for 1ms
        usleep(1000);
        

        if (ui.currentScreen == AUX) {
             // we can do a whole screen,  but not faster than 20fps
            if (count20fps > 50){
                count20fps = 0;
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
            count20fps++;
        }
        else if (ui.currentScreen == MENU) {
             // we can do a whole screen,  but not faster than 20fps
            if (count20fps > 50){
                count20fps = 0;
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
            }
            count20fps++;

            // if there is a patch running while on menu screen, switch back to patch screen after the timeout 
            if (ui.menuScreenTimeout) ui.menuScreenTimeout--;
            else ui.currentScreen = PATCH;
        }
        else if (ui.currentScreen == PATCH) {
            if (ui.patchIsRunning) {
                // every 16 ms send a new screen page
                if (count > 16){
                    count = 0;
                    updateScreenPage(page, ui.patchScreen);
                    page++;
                    page %= 8;
                }
                count++;
            }
        }
       
        // every 1 second send a ping in case MCU resets
        if (countReadyPing >1000){
            countReadyPing = 0;
            rdyMsg.send(dump);
            slip.sendMessage(dump.buffer, dump.length, serial);
        }
        countReadyPing++;

        // poll for knobs
        if (countKnobPoll > 50){
            countKnobPoll = 0;
            sendGetKnobs();
        }
        countKnobPoll++;
    } // for;;
}

/** OSC messages received internally (from PD or other program) **/

void setPatchScreenOledLine1(OSCMessage &msg){
    setPatchScreenOledLine(1, msg);
}

void setPatchScreenOledLine2(OSCMessage &msg){
    setPatchScreenOledLine(2, msg);
}

void setPatchScreenOledLine3(OSCMessage &msg){
    setPatchScreenOledLine(3, msg);
}

void setPatchScreenOledLine4(OSCMessage &msg){
    setPatchScreenOledLine(4, msg);
}

void setPatchScreenOledLine5(OSCMessage &msg){
    setPatchScreenOledLine(5, msg);
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

void setAuxScreen(OSCMessage &msg){
    ui.currentScreen = AUX;
    ui.newScreen = 1;
}

void reload(OSCMessage &msg){
    ui.getPatchList();
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
        if (msg.getInt(0) == 1) ui.encoderPress();
        if (msg.getInt(0) == 0) ui.encoderRelease();
    }
}
/* end OSC messages received from MCU */

/* helpers */
void setPatchScreenOledLine(int lineNum, OSCMessage &msg){

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
    ui.patchScreen.setLine(lineNum, screenLine);
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


