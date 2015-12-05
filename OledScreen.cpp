

#include "OledScreen.h"
#include "fonts.h"
#include <string.h>
#include "simple_svg_1.0.0.hpp"

void OledScreen::saveSVG(const char * filename) {

    using namespace svg;
    Dimensions dimensions(256, 128);
    Document doc(filename, Layout(dimensions, Layout::BottomLeft));

    // bg
    doc << Rectangle(Point(0, dimensions.height), dimensions.width, dimensions.height, Color::Black);

    int x,y;
    for(y=0;y<64;y++){
        for(x=0; x<128; x++){
            if (get_pixel(x, y)) {
               // printf("*");
                doc << Rectangle(Point((x * 2), 128 - (y * 2)), 2, 2, Color::White);
            }
            //else printf(" ");
        }
        //printf("\n");
    }
    //printf("\n");
    doc.save();
}

void OledScreen::clear(void){
    int i;
    for (i = 0; i < 1024; i++) pix_buf[i] = 0;
}

void OledScreen::setLine(int lineNum, char * line) {

    int i, len;

    lineNum -= 1;

    clearLine(lineNum);

    len = strlen(line);
    if (len > 21)
        println_8(line, 21,  2, (lineNum * 11) + 10);
    else
        println_8(line, len,  2, (lineNum * 11) + 10);


}

void OledScreen::clearLine(int lineNum){
    uint8_t i, j;

    for (i = 0; i<128; i++)
        for (j = 0; j<11; j++)
            put_pixel(0, i, j+9+(lineNum * 11));

}

void OledScreen::invertLine(int lineNum){

    invert_area(9 + lineNum * 11, 20 + lineNum * 11);
}

void OledScreen::draw_box(uint8_t sizex, uint8_t sizey, uint8_t x, uint8_t y){
    uint8_t i, j;

    for (i = 0; i<sizey; i++)
        for (j = 0; j<sizex; j++)
            put_pixel(1, x+i, y+j);
   
}



void OledScreen::drawNotification( char * line ) {

    int i, len;

    // first clear it out
    for (i = 0; i < 128; i++)
        pix_buf[i] = 0;

    len = strlen(line);
    if (len > 21)
        println_8(line, 21,  2, 0);
    else
        println_8(line, len,  2, 0);

}


void OledScreen::drawInfoBar(int inR, int inL, int outR, int outL) {

    int i;    

    // first clear it out
    for (i = 0; i < 128; i++)
        pix_buf[i] = 0;

    // draw input output
    put_char_small('I', 0, 0, 1);
    put_char_small('O', 48, 0, 1);

    
    // VU meter
    // after I
    // little guys
    draw_box(1, 2, 7, 1); 
    draw_box(1, 2, 12, 1); 
    draw_box(1, 2, 17, 1); 
    draw_box(1, 2, 22, 1); 
    draw_box(1, 2, 28, 1); 
    draw_box(1, 2, 33, 1); 
    draw_box(1, 2, 38, 1); 

    draw_box(1, 2, 7, 5);
    draw_box(1, 2, 12, 5);
    draw_box(1, 2, 17, 5);
    draw_box(1, 2, 22, 5);
    draw_box(1, 2, 28, 5);
    draw_box(1, 2, 33, 5);
    draw_box(1, 2, 38, 5);

    // big guys
    for (i = 0; i < (inR % 8); i++){
        draw_box(3, 4, 7 + (5 * i), 0); 
    }

    for (i = 0; i < (inL % 8); i++){
        draw_box(3, 4, 7 + (5 * i), 4);
    }

    // after O
    // small guys
    draw_box(1, 2, 55, 1); 
    draw_box(1, 2, 60, 1); 
    draw_box(1, 2, 65, 1); 
    draw_box(1, 2, 70, 1); 
    draw_box(1, 2, 75, 1); 
    draw_box(1, 2, 80, 1); 
    draw_box(1, 2, 85, 1); 

    draw_box(1, 2, 55, 5);
    draw_box(1, 2, 60, 5);
    draw_box(1, 2, 65, 5);
    draw_box(1, 2, 70, 5);
    draw_box(1, 2, 75, 5);
    draw_box(1, 2, 80, 5);
    draw_box(1, 2, 85, 5);

    // big guys
    for (i = 0; i < (outR % 8); i++){
        draw_box(3, 4, 55 + (5 * i), 0); 
    }

    for (i = 0; i < (outL % 8); i++){
        draw_box(3, 4, 55 + (5 * i), 4);
    }

    // other info
/*    put_char_small('M', 95, 0, 1);
    draw_box(1, 3, 103, 0);
    draw_box(1, 1, 104, 1);
    draw_box(1, 3, 103, 2);

    draw_box(1, 1, 104, 4);
    draw_box(1, 1, 103, 5);
    draw_box(1, 1, 105, 5);
    draw_box(1, 1, 104, 6);
    
    put_char_small('D', 110, 0, 1);
    put_char_small('F', 116, 0, 1);*/

}

void OledScreen::println_16(char * line, int len, int x, int y){
    int i, deltax;
    deltax = x;
    for (i = 0; i < len; i++) {
        deltax += put_char_arial16(line[i], deltax, y, 1);
        deltax += 2;
    }
}

void OledScreen::println_8(char * line, int len, int x, int y){
    int i, deltax;
    deltax = x;
    for (i = 0; i < len; i++) {
        deltax += put_char_small(line[i], deltax, y, 1);
        deltax += 1;
    }
}

void OledScreen::put_pixel(unsigned int on, unsigned int x, unsigned int y){    
    
    unsigned int page = 0;
    unsigned int column = 0;
    unsigned char tmp8 = 0;

    x &= 0x7f;
    y &= 0x3f;

    // subtract cause its flipped
    //page = 7 - (y / 8);
    //column = 127 - x;

    page = y / 8;
    column = x;


    tmp8 = pix_buf[(page * 128) + column];
    
    if (on){
       tmp8 |= (1 << (y & 0x7)); 
       //tmp8 |= (1 << (7 - (y & 0x7))); 
    }
    else{
       tmp8 &= ~(1 << (y & 0x7));
       //tmp8 &= ~(1 << (7 - (y & 0x7)));
   }

   pix_buf[(page * 128) + column] = tmp8;
}

void OledScreen::invert_screen(void) {
    unsigned int i,x,y;

    for(i=0;i<1024;i++)
        pix_buf[i] = ~pix_buf[i];

}

void OledScreen::invert_area(unsigned int y0, unsigned int y1) {
    int y, x;

    for (y = y0; y < y1; y++) {
        for (x= 0; x<128; x++){
            if (get_pixel(x,y))
               // put_pixel(1,x,y);
                put_pixel(0,x,y);
            else 
                //put_pixel(0,x,y);
                put_pixel(1,x,y);
        }
    }


}

unsigned int OledScreen::get_pixel(unsigned int x, unsigned int y){
   
    unsigned int page = 0;
    unsigned int column = 0;
    unsigned char tmp8 = 0;

    x &= 0x7f;
    y &= 0x3f;

    // subtract cause its flipped
    //page = 7 - (y / 8);
    //column = 127 - x;
    page = y / 8;
    column = x;


    tmp8 = pix_buf[(page * 128) + column];
    
    //return (tmp8 >> (7 - (y & 0x7))) & 1; 
    return (tmp8 >> ((y & 0x7))) & 1; 
 }


unsigned int OledScreen::put_char_arial32(unsigned char character, unsigned int y, unsigned int x, unsigned int color){
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;
  
  if (character == 32)
    return 8;
  
  character -= 33;

  charWidth = arial32Width[character + 1];
  charOffset = arial32Offset[character] * 4;

  for (i = 0; i < 4; i++){
    for (j = 0; j < 8; j++){
      for (k = 0; k < charWidth; k++){
        if ((arial32[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k),  (x + (i * 8) + j));
        else
          put_pixel(0, (y + k),  (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}

unsigned int OledScreen::put_char_arial24(unsigned char character, unsigned int y, unsigned int x, unsigned int color){
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;

  if (character == 32)
    return 6;

  character -= 33;

  charWidth = arial24Width[character + 1];
  charOffset = arial24Offset[character] * 3;


  
  for (i = 0; i < 3; i++){
    for (j = 0; j < 8; j++){
      for (k = 0; k < charWidth; k++){
        if ((arial24[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k), (x + (i * 8) + j));
        else
          put_pixel(0, (y + k), (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}

unsigned int OledScreen::put_char_arial16(unsigned char character, unsigned int y, unsigned int x, unsigned int color){
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;

  if (character == 32)
    return 4;

  character -= 33;

  charWidth = arial16Width[character + 1];
  charOffset = arial16Offset[character] * 2;


  
  for (i = 0; i < 2; i++){
    for (j = 0; j < 8; j++){
      for (k = 0; k < charWidth; k++){
        if ((arial16[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k), (x + (i * 8) + j));
        else
          put_pixel(0, (y + k), (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}


unsigned int OledScreen::put_char_small(unsigned char c, unsigned int y, unsigned int x, unsigned int color){
	int i, j;
	c -= 32;
	for (i = 0; i < 5; i++){
		for (j = 0; j < 8; j++){
		    if ((characters[(c * 5) + i] >> j) & 0x01)
			    put_pixel(color, y + i, x + j);
			else
                put_pixel(0, y + i, x + j);
		}
	}
        return 5;
}



