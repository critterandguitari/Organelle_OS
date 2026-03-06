#ifndef TIMER_H
#define TIMER_H

#include <time.h>

/*

Timer

(c) 2016 Owen Osborn, Critter & Guitari

Uses CLOCK_MONOTONIC to be immune to system time changes.

*/

class Timer
{
 public:

  Timer ();

  // destructor
  ~Timer();

   void reset(void);

   float getElapsed(void);

   struct timespec start, stop;

};


#endif
