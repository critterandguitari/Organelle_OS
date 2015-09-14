
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "OSC/OSCMessage.h"
#include "OSC/SimpleWriter.h"
#include "Serial.h"
#include "UdpSocket.h"
#include "SLIPEncodedSerial.h"
#include "OledScreen.h"

Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;
OledScreen screen;

static const uint8_t eot = 0300;

void updateScreen();
void updateScreenPage(uint8_t page);

// example handler
void gotone(OSCMessage &msg){
    static int count;

    char line[1024];
    int len, i, outR, outL, inR, inL;    

    if (msg.isInt(0)) inR = msg.getInt(0);
    if (msg.isInt(1)) inL = msg.getInt(1);
    if (msg.isInt(2)) outR = msg.getInt(2);
    if (msg.isInt(3)) outL = msg.getInt(3);

    screen.drawInfoBar(inR, inL, outR, outL);
    updateScreenPage(0);

/*
    if (msg.isString(0)){
       len =  msg.getString(0, line, 1024);
    }

    line[len] = 0;
    printf("line %s len: %d \n", line, len);
    screen.println_16(line, len-1, 0, 0);
    updateScreen();*/
}

void updateScreenPage(uint8_t page){
    
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

void updateScreen(){
    
    uint8_t oledPage[128];
    uint32_t i, j;

    for (i=0;i<8;i++){
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
        usleep(5000);
    }
}

int main(int argc, char* argv[]) {
      
    uint32_t seconds = 0;
    char udpPacketIn[256];
    //uint8_t osc_packet_in[256];
    uint8_t i = 0;
    int len = 0;


    UdpSocket udpSock(4001);
    udpSock.setDestination(4000, "localhost");
    OSCMessage msgIn;
    
    printf("cool\n");

    // make an osc object
    // the message wants an OSC address as first argument
    OSCMessage msg("/sys/renumber");
    msg.add(13);
       
    // this just dumps it into the simple writer dump
    msg.send(dump);

    printf("\n msg is %d bytes\n", dump.length);

    // print it out

    // UDP send
    //    udpSock.writeBuffer(dump.buffer, dump.length);

    // serial send
    //    slip.sendMessage(dump.buffer, dump.length, serial);

    //    usleep(1000000);
    printf("\nnow gonna send oled\n");
    
    // screen.put_char_arial32('O', 10, 10, 1);
    // screen.put_char_small('E', 50, 50, 1);
    
   // screen.invert_screen();
    updateScreen();

    // full udp -> serial -> serial -> udp
    for (;;){
        // receive udp, send to serial
        len = udpSock.readBuffer(udpPacketIn, 256, 0);
        if (len > 0){
            for (i = 0; i < len; i++){
                msgIn.fill(udpPacketIn[i]);
            }
            if(!msgIn.hasError()){
                msgIn.dispatch("/vumeter", gotone, 0);
                // send it along
                msgIn.send(dump);
                slip.sendMessage(dump.buffer, dump.length, serial);
            }
            else {
                printf("bad message");
            }
            msgIn.empty();
        }   

        // receive serial, send udp
        if(slip.recvMessage(serial)) {
            udpSock.writeBuffer(slip.decodedBuf, slip.decodedLength);
        }

        // sleep for 1ms
        usleep(1000);
    } // for;;
}


