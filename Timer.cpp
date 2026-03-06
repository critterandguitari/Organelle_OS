#include "Timer.h"
#include <stdio.h>

Timer::Timer() {
    clock_gettime(CLOCK_MONOTONIC, &start);
}

Timer::~Timer() {

}

void Timer::reset(void) {
    clock_gettime(CLOCK_MONOTONIC, &start);
}


// time in ms
float Timer::getElapsed(void) {
    clock_gettime(CLOCK_MONOTONIC, &stop);
    long elapsed_sec = stop.tv_sec - start.tv_sec;
    long elapsed_nsec = stop.tv_nsec - start.tv_nsec;
    float e = (float)elapsed_sec + (float)elapsed_nsec / 1000000000.f;
    return e * 1000.f;
}
