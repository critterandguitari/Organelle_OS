
#include "SLIPEncodedSerial.h"

/*
 CONSTRUCTOR
 */
SLIPEncodedSerial::SLIPEncodedSerial()
{
    rstate = WAITING;
    rxBufHead = 0;
    rxBufTail = 0;
    rxPacketIndex = 0;
    encodedBufIndex = 0;
    decodedBufIndex = 0;
}

static const uint8_t eot = 0300;
static const uint8_t slipesc = 0333;
static const uint8_t slipescend = 0334;
static const uint8_t slipescesc = 0335;


int SLIPEncodedSerial::sendMessage(const uint8_t *buf, uint32_t len, Serial &s)
{
    encode(buf, len);
    s.writeBuffer(encodedBuf, encodedLength);
}

//int SLIPEncodedSerial::recvPacket(uint8_t * buf, uint32_t len)
int SLIPEncodedSerial::recvMessage(Serial &s)
{
    int i, len;
    
    // fill up recv buffer from serial port
    len = s.readBuffer(serialIn, SERIAL_READ_SIZE);
    if (len == -1) {
        //    printf("Error reading from serial port\n");
    }
    else if (len == 0) {
        //printf("No more data\n");
    }
    else {
        for (i = 0;  i < len; i++) {
            rxBuf[rxBufHead++] = serialIn[i];
            if (rxBufHead >= (RX_BUF_SIZE - 1)) rxBufHead = 0;
        }
    }
   
    // process rx buffer, this might return before the whole thing
    // is proccessed,  but we'll just get it next time
    while (rxBufTail != rxBufHead) {
       
       uint8_t tmp8 = rxBuf[rxBufTail++];
        if (rxBufTail >= (RX_BUF_SIZE - 1)) rxBufTail = 0;
    
      //  uint8_t tmp8 = serialIn[i];
        
        if (rstate == WAITING) {
            if (tmp8 == eot) rstate = WAITING; // just keep waiting for something afer EOT
            else {
                rxPacketIndex = 0;
                rxPacket[rxPacketIndex++] = tmp8;
                rstate = RECEIVING;
            }
        } // waiting
        else if (rstate == RECEIVING){
            if (tmp8 == eot) {  //TODO:  exit if message len > max
                rstate = WAITING;
                decode(rxPacket, rxPacketIndex);
                return 1;
            }
            else {
                rxPacket[rxPacketIndex++] = tmp8;
                rstate = RECEIVING;
            }
        } //receiving
    } // gettin bytes
    return 0;
}
 
//encode SLIP, put it in the encoded buffer
void SLIPEncodedSerial::encode(uint8_t b){
    if(b == eot){
        encodedBuf[encodedBufIndex++] = slipesc;
        encodedBuf[encodedBufIndex++] = slipescend;
    } else if(b==slipesc) {
        encodedBuf[encodedBufIndex++] = slipesc;
        encodedBuf[encodedBufIndex++] = slipescesc;
   } else {
        encodedBuf[encodedBufIndex++] = b;
    }
}

void SLIPEncodedSerial::encode(const uint8_t *buf, int size) 
{  
    beginPacket();
    while(size--) encode(*buf++); 
    endPacket();
}

// decode SLIP, put it in the decoded buffer
void SLIPEncodedSerial::decode(const uint8_t *buf, int size)
{
    int i;
    decodedBufIndex = 0;
    i = 0;

    while (i < size) {
        if (buf[i] == slipesc) {  // TODO error out here if slipescend or slipescesc doesn't follow slipesc
            i++;
            if (buf[i] == slipescend) decodedBuf[decodedBufIndex++] = eot;
            if (buf[i] == slipescesc) decodedBuf[decodedBufIndex++] = slipesc;
            i++;
        }
        else {
            decodedBuf[decodedBufIndex++] = buf[i];
            i++;
        }
    }
    decodedLength = decodedBufIndex;
}

//SLIP specific method which begins a transmitted packet
void SLIPEncodedSerial::beginPacket() {
    encodedBufIndex = 0;
    encodedBuf[encodedBufIndex++] = eot;
}

//signify the end of the packet with an EOT
void SLIPEncodedSerial::endPacket(){
    encodedBuf[encodedBufIndex++] = eot;
    encodedLength = encodedBufIndex;
}


