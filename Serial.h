#ifndef SERIAL_H
#define SERIAL_H


#include <string>

/*

Serial IO

(c) 2015 Owen Osborn, Critter & Guitari

*/

class Serial
{
 public:
  // Default constructor takes serial port as argument
  Serial ();

  // destructor
  ~Serial();

  int serial_fd;
  
  // send stuff
  int writeBuffer(void *buffer, long len);

  // receive stuff
  int readBuffer(void *buffer, long bufferSize);

};


#endif
