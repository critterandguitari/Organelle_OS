#include "SerialMCU.h"

SerialMCU::SerialMCU() {
}

void SerialMCU::init(){
    // send ready to wake up MCU
    // MCU is ignoring stuff over serial port until this message comes through
    // don't empty the message because it gets sent out periodically incase MCU resets
    OSCMessage msg("/ready");
    msg.add(1);
    msg.send(oscBuf);
    // send it a few times just in case
    for (int i = 0; i < 4; i++) {
        slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
        usleep(20000); // wait 20 ms
    }

    // keys
    keyStatesLast = 0;
    clearFlags();
}

void SerialMCU::clearFlags() {
    encButFlag = 0;
    encTurnFlag = 0;
    knobFlag = 0;
    keyFlag = 0;
    footswitchFlag = 0;
}

void SerialMCU::poll(){
    OSCMessage msgIn;
    // receive serial
    if (slip.recvMessage(serial)) {

        // check if we need to do something with this message
        msgIn.empty();
        msgIn.fill(slip.decodedBuf, slip.decodedLength);

        if (msgIn.fullMatch("/key", 0)) keysInput(msgIn);
        if (msgIn.fullMatch("/fs", 0)) footswitchInput(msgIn);
        if (msgIn.fullMatch("/enc", 0)) encoderInput(msgIn);
        if (msgIn.fullMatch("/encbut", 0)) encoderButtonInput(msgIn);
        if (msgIn.fullMatch("/knobs", 0)) knobsInput(msgIn);

    }
}

void SerialMCU::pollKnobs(){    
    OSCMessage msg("/getknobs");
    msg.add(1);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SerialMCU::updateOLED(OledScreen &s){
    updateScreenPage(0, s);
    updateScreenPage(1, s);
    updateScreenPage(2, s);
    updateScreenPage(3, s);
    updateScreenPage(4, s);
    updateScreenPage(5, s);
    updateScreenPage(6, s);
    updateScreenPage(7, s);
}

void SerialMCU::updateScreenPage(uint8_t page, OledScreen &screen) {

    uint8_t oledPage[128];
    uint8_t tmp;
    uint32_t i, j, esc;

    i = page;
    esc = 0;
    // copy 128 byte page from the screen buffer
    for (j = 0; j < 128; j++) {

        // hack to avoid too many SLIP END characters (192) in packet 
        // which causes packet size to increase and causes problems down the line
        tmp = screen.pix_buf[j + (i * 128)];
        if (tmp == 192){
            esc++;
            if (esc > 64) tmp = 128; // replace 192 with 128 'next best' 
        }
        oledPage[j] = tmp;
    }
    OSCMessage oledMsg("/oled");
    oledMsg.add(i);
    oledMsg.add(oledPage, 128);
    oledMsg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
    oledMsg.empty();
}

void SerialMCU::ping(){
    OSCMessage msg("/ready");
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SerialMCU::shutdown() {
    printf("sending shutdown...\n");
    OSCMessage msg("/shutdown");
    msg.add(1);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SerialMCU::setLED(unsigned c) {
    OSCMessage msg("/led");
    msg.add(c);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SerialMCU::footswitchInput(OSCMessage &msg){
     if (msg.isInt(0)) {
       // printf("fs %d \n", msg.getInt(0));
        footswitch = msg.getInt(0);
        footswitchFlag = 1;
    }
}

void SerialMCU::encoderInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        //printf("enc %d \n", msg.getInt(0));
        encTurn = msg.getInt(0);
        encTurnFlag = 1;
    }
}

void SerialMCU::encoderButtonInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        //printf("enc but %d \n", msg.getInt(0));
        encBut = msg.getInt(0);
        encButFlag = 1;
        
    }
}

void SerialMCU::knobsInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        //printf("knobs %d %d %d %d %d %d \n", msg.getInt(0), msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        for (int i = 0; i < 6; i++) adcs[i] = msg.getInt(i);
        knobFlag = 1;
    }
}

void SerialMCU::keysInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1)) {
        //printf("%d %d \n", msg.getInt(0), msg.getInt(1));   
        if (msg.getInt(1)) keyStates |= (1 << msg.getInt(0));
        else keyStates &= ~(1 << msg.getInt(0));
        keyFlag = 1;
    }
}

