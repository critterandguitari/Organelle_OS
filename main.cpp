
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
#include "PatchMenu.h"

#define MENU_TIMEOUT 2000  // m sec timeout when screen switches back to patch detail

Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;
OledScreen menuScreen;
OledScreen patchScreen;
PatchMenu menu;

int needNewScreen = 0;
int patchScreenEnabled = 0;
int menuScreenTimeout = MENU_TIMEOUT;

void updateScreen();
void updateScreenPage(uint8_t page, OledScreen &screen);
void setOledLine(int lineNum, OSCMessage &msg);


void setOledLine1(OSCMessage &msg){
    setOledLine(1, msg);
}

void setOledLine2(OSCMessage &msg){
    setOledLine(2, msg);
}


void setOledLine3(OSCMessage &msg){
    setOledLine(3, msg);
}


void setOledLine4(OSCMessage &msg){
    setOledLine(4, msg);
}


void setOledLine5(OSCMessage &msg){
    setOledLine(5, msg);
}


void setOledLine(int lineNum, OSCMessage &msg){

    char str[256];
    char screenLine[256];
    int i = 0;

    screenLine[0] = 0;

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
    patchScreen.setLine(lineNum, screenLine);
    //    printf("%s\n", screenLine);
}

// vu handler
void vuMeter(OSCMessage &msg){
    static int count;

    char line[1024];
    int len, i, outR, outL, inR, inL;    

    if (msg.isInt(0)) inR = msg.getInt(0);
    if (msg.isInt(1)) inL = msg.getInt(1);
    if (msg.isInt(2)) outR = msg.getInt(2);
    if (msg.isInt(3)) outL = msg.getInt(3);

    patchScreen.drawInfoBar(inR, inL, outR, outL);

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

void sendReady(OSCMessage &msg){
    
    printf("sending ready...\n");
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void sendGetKnobs(OSCMessage &msg){
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void sendLED(OSCMessage &msg){
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}


void encoderInput(OSCMessage &msg){
    
    if (msg.isInt(0)){
        if (msg.getInt(0) == 1) menu.encoderUp();
        if (msg.getInt(0) == 0) menu.encoderDown();
    }
    menu.drawPatchList(menuScreen);
    needNewScreen = 1;
    patchScreenEnabled = 0;
    menuScreenTimeout = MENU_TIMEOUT;
}

void encoderButton(OSCMessage &msg){
    
    if (msg.isInt(0)){
        if (msg.getInt(0) == 1) menu.encoderPress();
        if (msg.getInt(0) == 0) menu.encoderRelease();
    }
    patchScreenEnabled = 1;
    patchScreen.clear();
    menuScreen.clear();
    needNewScreen = 1;  // send full screen to clear it
}

int main(int argc, char* argv[]) {
      
    uint32_t seconds = 0;
    char udpPacketIn[256];
    //uint8_t osc_packet_in[256];
    uint8_t i = 0;
    int len = 0;
    int count = 0;
    int page = 0;
    int count20fps = 0;
    
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

    menu.getPatchList();
    menu.drawPatchList(menuScreen);


    // send ready 
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();


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
                msgIn.dispatch("/oled/line/1", setOledLine1, 0);
                msgIn.dispatch("/oled/line/2", setOledLine2, 0);
                msgIn.dispatch("/oled/line/3", setOledLine3, 0);
                msgIn.dispatch("/oled/line/4", setOledLine4, 0);
                msgIn.dispatch("/oled/line/5", setOledLine5, 0);
                msgIn.dispatch("/ready", sendReady, 0);
                msgIn.dispatch("/led", sendLED, 0);
                msgIn.dispatch("/getknobs", sendGetKnobs, 0);
                // send it along
            //    msgIn.send(dump);
            //    slip.sendMessage(dump.buffer, dump.length, serial);
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
        
        // if patch detail is enabled
        if (patchScreenEnabled) {
            // every 16 ms send a new screen page
            if (count > 16){
                count = 0;
            
                updateScreenPage(page, patchScreen);
                page++;
                page %= 8;
            }
            count++;
        }

        if (menuScreenTimeout) menuScreenTimeout--;
        else patchScreenEnabled = 1;
        
        // we can do a whole screen,  but not faster than 20fps
        if (count20fps > 50){
            count20fps = 0;
            if (needNewScreen){
                needNewScreen = 0;
                updateScreenPage(1, menuScreen);
                updateScreenPage(2, menuScreen);
                updateScreenPage(3, menuScreen);
                updateScreenPage(4, menuScreen);
                updateScreenPage(5, menuScreen);
                updateScreenPage(6, menuScreen);
                updateScreenPage(7, menuScreen);
            }
        }
        count20fps++;
    } // for;;
}


