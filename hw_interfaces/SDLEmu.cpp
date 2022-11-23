#include "SDLEmu.h"
#include <SDL2/SDL.h>

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

SDLEmu::SDLEmu() {
}

void SDLEmu::init(){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    window = SDL_CreateWindow( "Organelle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_SHOWN );
    screenSurface = SDL_GetWindowSurface( window );

    // keys
    keyStatesLast = 0;
    clearFlags();
}

void SDLEmu::clearFlags() {
    encButFlag = 0;
    encTurnFlag = 0;
    knobFlag = 0;
    keyFlag = 0;
    footswitchFlag = 0;
}

bool quitting = false;
void SDLEmu::poll(){
    if (quitting) {
        return;
    }

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if ( e.type == SDL_QUIT ) {
            shutdown();
            quitting = true;
            return;
        }
    }
    SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0,0,0 ) );
    SDL_UpdateWindowSurface( window );

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

void SDLEmu::pollKnobs(){    
    OSCMessage msg("/getknobs");
    msg.add(1);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SDLEmu::updateOLED(OledScreen &s){
    updateScreenPage(0, s);
    updateScreenPage(1, s);
    updateScreenPage(2, s);
    updateScreenPage(3, s);
    updateScreenPage(4, s);
    updateScreenPage(5, s);
    updateScreenPage(6, s);
    updateScreenPage(7, s);
}

void SDLEmu::updateScreenPage(uint8_t page, OledScreen &screen) {

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

void SDLEmu::ping(){
    OSCMessage msg("/ready");
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SDLEmu::shutdown() {
    printf("sending shutdown...\n");
    SDL_Quit();
    OSCMessage msg("/shutdown");
    msg.add(1);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
    exit(0);
}

void SDLEmu::setLED(unsigned c) {
    OSCMessage msg("/led");
    msg.add(c);
    msg.send(oscBuf);
    slip.sendMessage(oscBuf.buffer, oscBuf.length, serial);
}

void SDLEmu::footswitchInput(OSCMessage &msg){
     if (msg.isInt(0)) {
        printf("fs %d \n", msg.getInt(0));
        footswitch = msg.getInt(0);
        footswitchFlag = 1;
    }
}

void SDLEmu::encoderInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        printf("enc %d \n", msg.getInt(0));
        encTurn = msg.getInt(0);
        encTurnFlag = 1;
    }
}

void SDLEmu::encoderButtonInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        printf("enc but %d \n", msg.getInt(0));
        encBut = msg.getInt(0);
        encButFlag = 1;
        
    }
}

void SDLEmu::knobsInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        printf("knobs %d %d %d %d %d %d \n", msg.getInt(0), msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        for (int i = 0; i < 6; i++) adcs[i] = msg.getInt(i);
        knobFlag = 1;
    }
}

void SDLEmu::keysInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1)) {
        printf("%d %d \n", msg.getInt(0), msg.getInt(1));   
        if (msg.getInt(1)) keyStates |= (1 << msg.getInt(0));
        else keyStates &= ~(1 << msg.getInt(0));
        keyFlag = 1;
    }
}

