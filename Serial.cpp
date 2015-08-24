
#include "Serial.h"

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <dirent.h>
#include <termios.h>
//#include <termio.h>
#include <fcntl.h>
#include <err.h>
//#include <linux/serial.h>



static int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        //error_message ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // ignore break signal
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        // error_message ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

static void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        //error_message ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0) ;
        //error_message ("error %d setting term attributes", errno);
}

Serial::Serial()
{
    // Open serial port   
    printf ("opening serial .....\n");
    //char *portname = "/dev/ttymxc0";
    //char *portname = "/dev/tty.usbserial-A30013Jp";
    //serial_fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    //serial_fd = open("/dev/tty.usbserial-A30013Jp", O_RDWR | O_NOCTTY | O_NDELAY);
    serial_fd = open("/dev/ttymxc0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd < 0)
    {
        printf("error %d opening %s: %s", errno, "/dev/whatever", strerror (errno));
        return;
    }
        /* set no wait on any operation */
    fcntl(serial_fd, F_SETFL, FNDELAY);
    
    printf("opened serial, setting up... \n");    
    set_interface_attribs (serial_fd, B500000, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    //set_interface_attribs (serial_fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (serial_fd, 0);                // set no blocking
    // Flush the port's buffers (in and out) before we start using it
    tcflush(serial_fd, TCIOFLUSH);
    printf("done opening serial \n");    

}

  // destructor
Serial::~Serial() 
{

}

// send stuff
int Serial::writeBuffer(void *buffer, long len)
{
    return write (serial_fd, buffer, len);
}

  // receive stuff
int Serial::readBuffer(void *buffer, long bufferSize)
{
    return read(serial_fd, buffer, bufferSize);//sizeof(buffer));
}


