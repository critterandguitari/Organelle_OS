#include "SDLPi.h"
#include <SDL2/SDL.h>

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Joystick *joystick;

SDLPi::SDLPi() {
}

void SDLPi::init(){
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Organelle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ORGANELLE_HW_WIDTH, ORGANELLE_HW_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, 128, 64);

    if (SDL_NumJoysticks() < 1) {
        printf( "Warning: No joysticks connected!\n" );
    } else {
        joystick = SDL_JoystickOpen(0);
        if (joystick == NULL) {
            printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
        }
    }

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

// this polll uses keys for buttons and arrows for 2 menu knobs
// eventually it will use stuff attached to GPIO
void SDLPi::poll(){
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            shutdown();
        } else if (e.type == SDL_KEYDOWN) {
            switch(e.key.keysym.sym) {
                case SDLK_UP:
                    encTurnFlag = 1;
                    encTurn = 0;
                    break;
                case SDLK_DOWN:
                    encTurnFlag = 1;
                    encTurn = 1;
                    break;
                case SDLK_RETURN:
                    encButFlag = 1;
                    encBut = 1;
                    break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch(e.key.keysym.sym) {
                case SDLK_RETURN:
                    encButFlag = 1;
                    encBut = 0;
                    break;
            }
        } else if (e.type == SDL_JOYBUTTONDOWN) {
            if (e.jbutton.button == SDL_CONTROLLER_BUTTON_A) {
                encButFlag = 1;
                encBut = 1;
            }
            if (e.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                encTurnFlag = 1;
                encTurn = 0;
            }
            if (e.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                encTurnFlag = 1;
                encTurn = 1;
            }
            printf("button: %i\n", e.jbutton.button);
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
    SDL_JoystickClose(joystick);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(0);
}

void SDLPi::setLED(unsigned stat) {
    stat %= 8;

    if (stat == 0) {
        printf("LED: 0 0 0\n");
    } else if (stat == 1) {
        printf("LED: 1 0 0\n");
    } else if (stat == 2) {
        printf("LED: 1 1 0\n");
    } else if (stat == 3) {
        printf("LED: 0 1 0\n");
    } else if (stat == 4) {
        printf("LED: 0 1 1\n");
    } else if (stat == 5) {
        printf("LED: 0 0 1\n");
    } else if (stat == 6) {
        printf("LED: 1 0 1\n");
    } else if (stat == 7) {
        printf("LED: 1 1 1\n");
    }
}



