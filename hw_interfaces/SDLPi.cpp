#include "SDLPi.h"
#include <SDL2/SDL.h>

SDL_Renderer *renderer;

SDLPi::SDLPi() {
}

void SDLPi::init(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Organelle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ORGANELLE_HW_WIDTH, ORGANELLE_HW_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, 128, 64);

    // keys
    keyStatesLast = 0;
    clearFlags();
}

void SDLPi::clearFlags() {
    encButFlag = 0;
    encTurnFlag = 0;
    knobFlag = 0;
    keyFlag = 0;
    footswitchFlag = 0;
}

void SDLPi::poll(){
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
                shutdown();
                break;
            case SDL_KEYDOWN:
                printf( "Key press detected\n" );
                break;

            case SDL_KEYUP:
                printf( "Key release detected\n" );
                break;

            default:
                break;
        }
    }
}

void SDLPi::pollKnobs(){
}

void SDLPi::updateOLED(OledScreen &s){
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int x, y;
    for (y = 0; y < 64; y++) {
        for (x = 0; x < 128; x++) {
            if (s.get_pixel(x, y)) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);
}

void SDLPi::ping(){
}

void SDLPi::shutdown() {
    printf("sending shutdown...\n");
    SDL_Quit();
    exit(0);
}

void SDLPi::setLED(unsigned c) {
    printf("LED: %d\n", c);
}

void SDLPi::footswitchInput(OSCMessage &msg){
     if (msg.isInt(0)) {
        printf("fs %d \n", msg.getInt(0));
        footswitch = msg.getInt(0);
        footswitchFlag = 1;
    }
}

void SDLPi::encoderInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        printf("enc %d \n", msg.getInt(0));
        encTurn = msg.getInt(0);
        encTurnFlag = 1;
    }
}

void SDLPi::encoderButtonInput(OSCMessage &msg){
    if (msg.isInt(0)) {
        printf("enc but %d \n", msg.getInt(0));
        encBut = msg.getInt(0);
        encButFlag = 1;
        
    }
}

void SDLPi::knobsInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        printf("knobs %d %d %d %d %d %d \n", msg.getInt(0), msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
        for (int i = 0; i < 6; i++) adcs[i] = msg.getInt(i);
        knobFlag = 1;
    }
}

void SDLPi::keysInput(OSCMessage &msg){
    if (msg.isInt(0) && msg.isInt(1)) {
        printf("%d %d \n", msg.getInt(0), msg.getInt(1));   
        if (msg.getInt(1)) keyStates |= (1 << msg.getInt(0));
        else keyStates &= ~(1 << msg.getInt(0));
        keyFlag = 1;
    }
}

