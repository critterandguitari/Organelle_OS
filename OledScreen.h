#ifndef OLEDSCREEN_H
#define OLEDSCREEN_H


#include <stdint.h> 
#include <string>


class OledScreen
{
    public:

        uint8_t pix_buf[1024];

        void setLine(int lineNumber, char * line);
        void clearLine(int lineNum);
        void invertLine(int lineNum);

        
        void clear(void);
        
        void draw_box(uint8_t sizex, uint8_t sizey, uint8_t x, uint8_t y);
        void drawInfoBar(int inR, int inL, int outR, int outL);

        void drawNotification( char * line );

        void put_pixel(unsigned int color, unsigned int x, unsigned int y);

        unsigned int get_pixel(unsigned int x, unsigned int y);
        void invert_screen(void);

        void invert_area(unsigned int y0, unsigned int y1);
        unsigned int put_char_arial32(unsigned char character, unsigned int y, unsigned int x, unsigned int color);

        unsigned int put_char_arial24(unsigned char character, unsigned int y, unsigned int x, unsigned int color);

        unsigned int put_char_arial16(unsigned char character, unsigned int y, unsigned int x, unsigned int color);

        unsigned int put_char_small(unsigned char c, unsigned int y, unsigned int x, unsigned int color);

        void println_16(char * line, int len, int x, int y);

        void println_8(char * line, int len, int x, int y);

        void saveSVG(const char * filename);
};


#endif
