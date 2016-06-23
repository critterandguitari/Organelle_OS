
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>

#include <pthread.h>

#include "OSC/OSCMessage.h"
#include "OSC/SimpleWriter.h"
#include "UdpSocket.h"

// for communicating with Pd or other program
UdpSocket udpSock(4001);

/** OSC messages received internally (from PD or other program) **/
void whatever(OSCMessage &msg);
/* end internal OSC messages received */

// recieve thread
void *udp_recv( void *ptr );
  
int len = 0;
char udpPacketIn[256];
uint8_t i = 0;


int main(int argc, char* argv[]) {
    pthread_t thread1;
    const char *message1 = "UDP reader";
    int  iret1;

    udpSock.setDestination(4000, "localhost");
    OSCMessage msgIn;

    // setup receive thread
    iret1 = pthread_create( &thread1, NULL, udp_recv, (void*) message1);
    if(iret1){
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }
     printf("pthread_create() for thread 1 returns: %d\n",iret1);

    // full udp -> serial -> serial -> udp
    for (;;){

        // receive udp, send to serial
        printf("in main...\n");
        // sleep for 1ms
        usleep(100000);
        
    } // for;;
    
    // wait for finish,  but ever get here?
    pthread_join( thread1, NULL);
    exit(EXIT_SUCCESS);

}

void *udp_recv( void *ptr ) {
    for(;;) {
        len = udpSock.readBuffer(udpPacketIn, 256, 0);
        if (len > 0){
            printf("got %d \n", len);
          //  msgIn.empty();
            for (i = 0; i < len; i++){
          //      msgIn.fill(udpPacketIn[i]);i
                printf("%c", udpPacketIn[i]);
            }    
            printf("\n");
            //if(!msgIn.hasError()){
            //    msgIn.dispatch("/thing", whatever, 0);
            //}
            //else {
            //    printf("bad message\n");
           // }
            //msgIn.empty();
        }   
    } // for ;;
}


/** OSC messages received internally (from PD or other program) **/
// settin patch screen
void whatever(OSCMessage &msg){

}
/* end internal OSC messages received */

