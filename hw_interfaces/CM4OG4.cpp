#include "CM4OG4.h"

// for the OLED
#define SSD1306_LCDHEIGHT                 64
#define SSD1306_LCDWIDTH                  128
#define SSD1306_SETCONTRAST               0x81
#define SSD1306_DISPLAYALLON_RESUME       0xA4
#define SSD1306_DISPLAYALLON              0xA5
#define SSD1306_NORMALDISPLAY             0xA6
#define SSD1306_INVERTDISPLAY             0xA7
#define SSD1306_DISPLAYOFF                0xAE
#define SSD1306_DISPLAYON                 0xAF
#define SSD1306_SETDISPLAYOFFSET          0xD3
#define SSD1306_SETCOMPINS                0xDA
#define SSD1306_SETVCOMDETECT             0xDB
#define SSD1306_SETDISPLAYCLOCKDIV        0xD5
#define SSD1306_SETPRECHARGE              0xD9
#define SSD1306_SETMULTIPLEX              0xA8
#define SSD1306_SETLOWCOLUMN              0x00
#define SSD1306_SETHIGHCOLUMN             0x10
#define SSD1306_SETSTARTLINE              0x40
#define SSD1306_SETSTARTPAGE              0xB0
#define SSD1306_MEMORYMODE                0x20
#define SSD1306_COMSCANINC                0xC0
#define SSD1306_COMSCANDEC                0xC8
#define SSD1306_SEGREMAP                  0xA0
#define SSD1306_CHARGEPUMP                0x8D
#define SSD1306_EXTERNALVCC               0x1
#define SSD1306_SWITCHCAPVCC              0x2

// GPIO pin defs
#define OLED_DC 5               // DC pin of OLED
#define OLED_RST 6              // RST pin of OLED
#define LEDG 22          
#define LEDR 23       
#define LEDB 24         
#define ENCA 17
#define ENCB 25
#define ENCS 16
#define INT0 26
#define INT1 27
#define MIC_SEL 4

#define PCA555_0 0x20
#define PCA555_1 0x21

#define AUX_LED_RED_OFF digitalWrite(LEDR,HIGH);
#define AUX_LED_RED_ON digitalWrite(LEDR,LOW);
#define AUX_LED_GREEN_OFF digitalWrite(LEDG,HIGH);
#define AUX_LED_GREEN_ON digitalWrite(LEDG,LOW);
#define AUX_LED_BLUE_OFF digitalWrite(LEDB,HIGH);
#define AUX_LED_BLUE_ON digitalWrite(LEDB,LOW);

#define BATTERY_BAR_5 4.8
#define BATTERY_BAR_4 4.7
#define BATTERY_BAR_3 4.66
#define BATTERY_BAR_2 4.43
#define BATTERY_BAR_1 4.29
#define BATTERY_BAR_0 4.15
#define LOW_BATTERY_SHUTDOWN_THRESHOLD 4.0

// OLED init bytes
static unsigned char oled_initcode[] = {
	// Initialisation sequence
	SSD1306_DISPLAYOFF,                     // 0xAE
	SSD1306_SETLOWCOLUMN,                   // low col = 0
	SSD1306_SETHIGHCOLUMN,                  // hi col = 0
	SSD1306_SETSTARTLINE,                   // line #0
	SSD1306_SETCONTRAST,                    // 0x81
	0xCF,
	0xa1,                                   // setment remap 95 to 0 (?)
	SSD1306_NORMALDISPLAY,                  // 0xA6
	SSD1306_DISPLAYALLON_RESUME,            // 0xA4
	SSD1306_SETMULTIPLEX,                   // 0xA8
	0x3F,                                   // 0x3F 1/64 duty
	SSD1306_SETDISPLAYOFFSET,               // 0xD3
	0x0,                                    // no offset
	SSD1306_SETDISPLAYCLOCKDIV,             // 0xD5
	0xF0,                                   // the suggested ratio 0x80
	SSD1306_SETPRECHARGE,                   // 0xd9
	0xF1,
	SSD1306_SETCOMPINS,                     // 0xDA
	0x12,                                   // disable COM left/right remap
	SSD1306_SETVCOMDETECT,                  // 0xDB
	0x40,                                   // 0x20 is default?
	SSD1306_MEMORYMODE,                     // 0x20
	0x00,                                   // 0x0 act like ks0108
	SSD1306_SEGREMAP | 0x1,
	SSD1306_COMSCANDEC,
	SSD1306_CHARGEPUMP,                     //0x8D
	0x14,
	// Enabled the OLED panel
	SSD1306_DISPLAYON
};

static unsigned char oled_poscode[] = {
   	SSD1306_SETLOWCOLUMN,                   // low col = 0
	SSD1306_SETHIGHCOLUMN,                  // hi col = 0
	SSD1306_SETSTARTLINE                    // line #0
};

CM4OG4::CM4OG4() {
}

void CM4OG4::init(){
    // setup GPIO, this uses actual BCM pin numbers 
    wiringPiSetupGpio();


    // OLED pins
    pinMode (OLED_DC, OUTPUT) ;
    pinMode (OLED_RST, OUTPUT) ;
    wiringPiSPISetup(0, 4*1000*1000);
    wiringPiSPISetup(1, 4*1000*1000);  // for adc
    
    // reset OLED
    digitalWrite(OLED_RST,  LOW) ;
    delay(50);
    digitalWrite(OLED_RST,  HIGH) ;
    
    // initialize OLED
    digitalWrite(OLED_DC, LOW);
    wiringPiSPIDataRW(0, oled_initcode, 28);

   	// Encoder
    lrmem = 3;
	lrsum = 0;
	num = 0;
    pinMode(ENCA, INPUT);
    pullUpDnControl(ENCA, PUD_OFF);
    pinMode(ENCB, INPUT);
	pullUpDnControl(ENCB, PUD_OFF);
    pinMode(ENCS, INPUT);
   	pullUpDnControl(ENCS, PUD_OFF);

    // mic sel
    pinMode(MIC_SEL, INPUT);
    pullUpDnControl(MIC_SEL, PUD_OFF);
    micSelSwitch = digitalRead(MIC_SEL);

    // keys
    pinMode(INT0, INPUT);
    pullUpDnControl(INT0, PUD_OFF);
    pinMode(INT1, INPUT);
    pullUpDnControl(INT1, PUD_OFF);
    int0 = int1 = 1;                    // active low interrupt pins
    fd0 = wiringPiI2CSetup(PCA555_0);
    fd1 = wiringPiI2CSetup(PCA555_1);
    io0l = io0h = io1l = io1h = 0xFF;   // active low keys
    
    clearFlags();
    // GPIO for LEDs
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
    delay(10); // flash em
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    // GPIO for power status 
    pwrStatus = 0; 

/*
    // keys
    keyStatesLast = 0;

    // get initial pin states
    shiftRegRead();
    pinValuesLast = pinValues;
    micSelSwitch = (pinValues >> 3) & 1;

    // set 
    batteryVoltage = 5;
    batteryBars = 5;
    lowBatteryShutdown = false;
 
*/
}

void CM4OG4::clearFlags() {
    encButFlag = 0;
    encTurnFlag = 0;
    knobFlag = 0;
    keyFlag = 0;
    footswitchFlag = 0;
}

void CM4OG4::poll(){
/*
    // read keys (updates pinValues)
    shiftRegRead();

    // get key values if a key pin changed (ignore the encoder pins)
    if ((pinValues & 0xFFFFFF80) != (pinValuesLast & 0xFFFFFF80)) {
//        displayPinValues();
        getKeys();
        keyFlag = 1;
    }
    pinValuesLast = pinValues;

    micSelSwitch = (pinValues >> 3) & 1;
    
  */  
    // check mic switch
    micSelSwitch = digitalRead(MIC_SEL);

    // check key int event io expander 0
    uint8_t tmp = digitalRead(INT0);
    uint8_t tmp2 = digitalRead(INT1);
    if (tmp == 0) { 
        io0l = wiringPiI2CReadReg8(fd0, 0);
        io0h = wiringPiI2CReadReg8(fd0, 1);
        keyFlag = 1;
    }

    if (tmp2 == 0) {
        io1l = wiringPiI2CReadReg8(fd1, 0);
        io1h = wiringPiI2CReadReg8(fd1, 1);
        keyFlag = 1;
    }

    if (keyFlag) {
        keyStates = 0;
        keyStates |= io1h << 24;
        keyStates |= io1l << 16;
        keyStates |= io0h << 8;
        keyStates |= io0l;
        keyStates |= 0xFE000000;
        keyStates = ~keyStates;
    }
    // check encoder
    getEncoder();
}

void CM4OG4::pollKnobs(){    

    static uint32_t battAvg = 0;
    static uint8_t num = 0;
    
    adcs[0] = adcRead(0);
    adcs[1] = adcRead(1);
    adcs[2] = adcRead(2);
    adcs[3] = adcRead(3);
    adcs[4] = adcRead(4);
    adcs[5] = adcRead(5);
    
    knobFlag = 1;
}

void CM4OG4::updateOLED(OledScreen &s){
    // spi will overwrite the buffer with input, so we need a tmp
    uint8_t tmp[1024];
    memcpy(tmp, s.pix_buf, 1024);
    
    digitalWrite(OLED_DC, LOW);
    wiringPiSPIDataRW(0, oled_poscode, 3);
    digitalWrite(OLED_DC, HIGH);
    wiringPiSPIDataRW(0, tmp, 1024);
}


void CM4OG4::ping(){

}

void CM4OG4::shutdown() {

}

void CM4OG4::setLED(unsigned stat) {
    stat %= 8;

    if (stat == 0) {
        AUX_LED_RED_OFF;
        AUX_LED_GREEN_OFF;
        AUX_LED_BLUE_OFF;
    }
    else if (stat == 1) {
        AUX_LED_RED_ON;
        AUX_LED_GREEN_OFF;
        AUX_LED_BLUE_OFF;
    }
    else if (stat == 2) {
        AUX_LED_RED_ON;
        AUX_LED_GREEN_ON;
        AUX_LED_BLUE_OFF;
    }
    else if (stat == 3) {
        AUX_LED_RED_OFF;
        AUX_LED_GREEN_ON;
        AUX_LED_BLUE_OFF;
    }
    else if (stat == 4) {
        AUX_LED_RED_OFF;
        AUX_LED_GREEN_ON;
        AUX_LED_BLUE_ON;
    }
    else if (stat == 5) {
        AUX_LED_RED_OFF;
        AUX_LED_GREEN_OFF;
        AUX_LED_BLUE_ON;
    }
    else if (stat == 6) {
        AUX_LED_RED_ON;
        AUX_LED_GREEN_OFF;
        AUX_LED_BLUE_ON;
    }
    else if (stat == 7) {
        AUX_LED_RED_ON;
        AUX_LED_GREEN_ON;
        AUX_LED_BLUE_ON;
    }
}



void CM4OG4::getKeys(void){
    /*keyStates = 0;
    
    keyStates |= (pinValues >> (0 + 7) & 1) << 24;
    keyStates |= (pinValues >> (1 + 7) & 1) << 16;
    keyStates |= (pinValues >> (2 + 7) & 1) << 17;
    keyStates |= (pinValues >> (3 + 7) & 1) << 18;
    keyStates |= (pinValues >> (4 + 7) & 1) << 19;
    keyStates |= (pinValues >> (5 + 7) & 1) << 20;
    keyStates |= (pinValues >> (6 + 7) & 1) << 21;
    keyStates |= (pinValues >> (7 + 7) & 1) << 22;
   
    keyStates |= (pinValues >> (8 + 7) & 1) << 23;
    keyStates |= (pinValues >> (9 + 7) & 1) << 8;
    keyStates |= (pinValues >> (10 + 7) & 1) << 9;
    keyStates |= (pinValues >> (11 + 7) & 1) << 10;
    keyStates |= (pinValues >> (12 + 7) & 1) << 11;
    keyStates |= (pinValues >> (13 + 7) & 1) << 12;
    keyStates |= (pinValues >> (14 + 7) & 1) << 13;
    keyStates |= (pinValues >> (15 + 7) & 1) << 14;
    
    keyStates |= (pinValues >> (16 + 7) & 1) << 15;
    keyStates |= (pinValues >> (17 + 7) & 1) << 0;
    keyStates |= (pinValues >> (18 + 7) & 1) << 1;
    keyStates |= (pinValues >> (19 + 7) & 1) << 2;
    keyStates |= (pinValues >> (20 + 7) & 1) << 3;
    keyStates |= (pinValues >> (21 + 7) & 1) << 4;
    keyStates |= (pinValues >> (22 + 7) & 1) << 5;
    keyStates |= (pinValues >> (23 + 7) & 1) << 6;
    
    keyStates |= (pinValues >> (24 + 7) & 1) << 7;
    
    keyStates |= (0xFE000000);  // zero out the bits not key bits
    keyStates = ~keyStates;*/
}

int CM4OG4::getEncoder(void){
	static uint8_t encoder_last = 0;
	uint8_t encoder = 0;

	#define PRESS 0
	#define RELEASE 1
	uint8_t button;
	static uint8_t button_last = RELEASE;
	static uint8_t press_count = 0;
	static uint8_t release_count = 0;
	button = digitalRead(ENCS);
	if (button == PRESS) {
		press_count++;
		release_count = 0;
	}
	if ((press_count > 10) && (button_last == RELEASE)){	// press
			button_last = PRESS;
			release_count = 0;
            encBut = 1;
            encButFlag = 1;
	}

	if (button == RELEASE) {
		release_count++;
		press_count = 0;
	}
	if ((release_count > 10) && (button_last == PRESS)){	// release
			button_last = RELEASE;
			press_count = 0;
            encBut = 0;
            encButFlag = 1;
	}
	
	// turning
  	static int8_t TRANS[] = {0,-1,1,14,1,0,14,-1,-1,14,0,1,14,1,-1,0};
   	int8_t l, r;

	l = digitalRead(ENCA);
	r = digitalRead(ENCB);

   	lrmem = ((lrmem & 0x03) << 2) + 2*l + r;
   	lrsum = lrsum + TRANS[lrmem];
   	// encoder not in the neutral state 
    	//if(lrsum % 4 != 0) return 0;
   	if(lrsum % 2 != 0) return 0;  // using 2 instead of 4 since our encoder is 2 detents per pulse
   	// encoder in the neutral state 
   	if (lrsum == 2) {
      		lrsum=0;
		encTurn = 1;
		encTurnFlag = 1;
      		return 1;
    	}
   	if (lrsum == -2) {
      		lrsum=0;
		encTurn = 0;
		encTurnFlag = 1;
      		return -1;
    	}
   	// lrsum > 0 if the impossible transition 
   	lrsum=0;
   	return 0;
}

uint32_t CM4OG4::adcRead(uint8_t adcnum)
{ 
    unsigned int commandout = 0;

    // read a channel from the MCP3008 ADC
    commandout = adcnum & 0x7;  // only 0-7
    commandout |= 0x18;     // start bit + single-ended bit

    uint8_t spibuf[3];

    spibuf[0] = commandout;
    spibuf[1] = 0;
    spibuf[2] = 0;

    wiringPiSPIDataRW(1, spibuf, 3);    

    return ((spibuf[1] << 8) | (spibuf[2])) >> 4;
    
	return 0;
}

void CM4OG4::displayPinValues(void)
{
/*    for(int i = 0; i < SR_DATA_WIDTH; i++)
    {
        printf(" ");

        if((pinValues >> ((SR_DATA_WIDTH-1)-i)) & 1)
            printf("1");
        else
            printf("0");

    }
    printf("\n");*/
}

void CM4OG4::checkFootSwitch (void) {
/*    static uint8_t foot_last = 0;
    uint8_t tmp;

    if (adcs[5] < 100) tmp = 0;
    if (adcs[5] > 900) tmp = 1;

    if (tmp != foot_last){
        footswitch = tmp;
        foot_last = tmp;
        footswitchFlag = 1;
    }*/
}
