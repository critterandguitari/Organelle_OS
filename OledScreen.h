#ifndef OLEDSCREEN_H
#define OLEDSCREEN_H


#include <stdint.h> 
#include <string>


class OledScreen
{
    public:
        OledScreen();
        uint8_t pix_buf[1024];
        bool showInfoBar;
        unsigned newScreen;              // flag indicating screen changed and needs to be sent to oled

        void setLine(int lineNumber, const char * line);
        void clearLine(int lineNum);
        void invertLine(int lineNum);
        void clear(void);
        void draw_line(int x0, int y0, int x1, int y1, unsigned int color);
        void draw_circle(unsigned int h, unsigned int k, unsigned int r, unsigned int color);
        void draw_filled_circle(unsigned int h, unsigned int k, unsigned int r, unsigned int color);
        void draw_box_filled(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey );
        void draw_box(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t color);
        void fill_area(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey,uint8_t color);
        void put_pixel(unsigned int color, unsigned int x, unsigned int y);
        unsigned int get_pixel(unsigned int x, unsigned int y);
        void invert_screen(void);
        void invert_area(unsigned int y0, unsigned int y1);
        void invert_area(unsigned int x, unsigned int y, unsigned int sizex, unsigned int sizey);
        unsigned int put_char_arial32(unsigned char character, unsigned int y, unsigned int x, unsigned int color);
        unsigned int put_char_arial24(unsigned char character, unsigned int y, unsigned int x, unsigned int color);
        unsigned int put_char_arial16(unsigned char character, unsigned int y, unsigned int x, unsigned int color);
        unsigned int put_char_small(unsigned char c, unsigned int y, unsigned int x, unsigned int color);
        void println(const char * line, int x, int y, int h, int color);
        void println_8(const char * line, int len, int x, int y);
        void saveSVG(const char * filename);
        
        void drawInfoBar(int inR, int inL, int outR, int outL, int peaks);
        void drawInfoBar(int inR, int inL, int outR, int outL, int peaks, int pwrStatus, int batteryLevel, int wifiStatus);
        void drawNotification(const char * line );
        void drawNotification(const char * line, int pwrStatus, int batteryLevel, int wifiStatus);
        void drawBatteryMeter(int lev);
        void drawWifiMeter(int lev);
        void drawPlug();
};


#endif
