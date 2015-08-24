

#include "OledScreen.h"
#include "fonts.h"

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
    page = 7 - (y / 8);
    column = 127 - x;

    tmp8 = pix_buf[(page * 128) + column];
    
    if (on){
       //tmp8 |= (1 << (y & 0x7)); 
       tmp8 |= (1 << (7 - (y & 0x7))); 
    }
    else{
       //tmp8 &= ~(1 << (y & 0x7));
       tmp8 &= ~(1 << (7 - (y & 0x7)));
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
    page = 7 - (y / 8);
    column = 127 - x;

    tmp8 = pix_buf[(page * 128) + column];
    
    return (tmp8 >> (7 - (y & 0x7))) & 1; 
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



