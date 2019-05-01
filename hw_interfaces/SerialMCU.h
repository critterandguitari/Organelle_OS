#ifndef SERIALMCU_H
#define SERIALMCU_H


#include <stdint.h> 
#include "../OledScreen.h"
#include "../OSC/OSCMessage.h"
#include "../OSC/SimpleWriter.h"
#include "../SLIPEncodedSerial.h"
#include "../Serial.h"

class SerialMCU
{
    public:
        SerialMCU();
        void init();
        void poll();
        void pollKnobs();
        void updateOLED(OledScreen &s);
        void ping();
        void shutdown();
        void setLED(unsigned c);
        void clearFlags();

        uint32_t encBut;
        uint32_t encButFlag;
        uint32_t encTurn;
        uint32_t encTurnFlag;
        uint32_t knobFlag;
        uint32_t adcs[8];
        uint32_t footswitch;
        uint32_t footswitchFlag;
        uint32_t keyStates;
        uint32_t keyStatesLast;
        uint32_t keyFlag;

    private:        
        // handlers for the inputs
        void knobsInput(OSCMessage &msg);
        void footswitchInput(OSCMessage &msg);
        void encoderInput(OSCMessage &msg);
        void encoderButtonInput(OSCMessage &msg);
        void keysInput(OSCMessage &msg);

        void updateScreenPage(uint8_t page, OledScreen &screen);
        SLIPEncodedSerial slip;
        Serial serial;
        SimpleWriter oscBuf;
};


#endif
