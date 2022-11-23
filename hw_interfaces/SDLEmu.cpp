#include "SDLEmu.h"
#include <SDL2/SDL.h>

#define LOGICAL_WIDTH 128
#define LOGICAL_HEIGHT 64

SDL_Renderer *renderer;

SDLEmu::SDLEmu() {
}

void SDLEmu::init(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Organelle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

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

void SDLEmu::poll(){
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if ( e.type == SDL_QUIT ) {
            shutdown();
            return;
        }
    }
}

void SDLEmu::pollKnobs(){
}

void SDLEmu::updateOLED(OledScreen &s){
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect rect = { 0, 0, 128, 64};
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);
}

void SDLEmu::ping(){
}

void SDLEmu::shutdown() {
    printf("sending shutdown...\n");
    SDL_Quit();
    exit(0);
}

void SDLEmu::setLED(unsigned c) {
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

