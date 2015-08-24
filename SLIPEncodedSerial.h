
#ifndef SLIPEncodedSerial_h
#define SLIPEncodedSerial_h

#include <stdint.h> 
#include "Serial.h"
#include "UdpSocket.h"

#define MAX_MSG_SIZE 256 // the maximum un encoded size.  the max encoded size will be this * 2 for slip overhead
#define SERIAL_READ_SIZE 64  // amount read from the serial port
#define RX_BUF_SIZE 1024  // should be at least twice as big as SERIAL_READ_SIZE
#define WAITING 1
#define RECEIVING 2

class SLIPEncodedSerial
{

private:

public:
	
	SLIPEncodedSerial();
    
    uint8_t rstate;

    // encoded message
    uint8_t encodedBuf[MAX_MSG_SIZE * 2];    // the encoded can be up to 2 * longer
    uint32_t encodedBufIndex;
    uint32_t encodedLength;

    // decoded message
    uint8_t decodedBuf[MAX_MSG_SIZE];
    uint32_t decodedBufIndex;
    uint32_t decodedLength;

    uint8_t serialIn[SERIAL_READ_SIZE]; // for reading from serial port, read in 64 byte chunks
    uint8_t rxBuf[RX_BUF_SIZE];  // circular buffer for incoming serial, should be 2 times serail read size
    uint32_t rxBufHead;
    uint32_t rxBufTail;

    uint8_t rxPacket[MAX_MSG_SIZE * 2];
    uint32_t rxPacketIndex;
    

    //SLIP specific method which begins a transmitted packet
	void beginPacket();
	
	//SLIP specific method which ends a transmittedpacket
	void endPacket();
   
    void encode(const uint8_t *buf, int size);
	void encode(uint8_t b);
    
    void decode(const uint8_t *buf, int size);

    int sendMessage(const uint8_t *buf, uint32_t len, Serial &s);

    int recvMessage(Serial &s);
    
};


#endif
