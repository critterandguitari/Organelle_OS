

#include "OledScreen.h"
#include "fonts.h"
#include <string.h>
#include "simple_svg_1.0.0.hpp"

OledScreen::OledScreen() {
  clear();
  showInfoBar = true;
  newScreen = 0;
  //printf("init oled screen, clearing oled screen\n");
}

void OledScreen::saveSVG(const char * filename) {

  using namespace svg;
  // using 16 pixels per oled pixel so we can have 1 px separation to
  // simulate look of real screen
  Dimensions dimensions(2048 + (6 * 16), 1024 + (6 * 16));
  Document doc(filename, Layout(dimensions, Layout::BottomLeft));

  // bg
  doc << Rectangle(Point(0, dimensions.height), dimensions.width, dimensions.height, Color::Black);

  // loop over oled pix buf
  int x, y;
  for (y = 0; y < 64; y++) {
    for (x = 0; x < 128; x++) {
      if (get_pixel(x, y)) {
        // offset for 3px border,  size is 15 to leave small space between pixels
        doc << Rectangle(Point(((x + 3) * 16), dimensions.height - ((y + 3) * 16)), 15, 15, Color::White);
      }
    }
  }
  doc.save();
}

void OledScreen::clear(void) {
  int i;
  for (i = 0; i < 1024; i++) pix_buf[i] = 0;
}

unsigned calcxpos(unsigned line) {
  return ((line - 1) * 11) + ((line > 0) * 9 );
}

void OledScreen::setLine(int lineNum, const char * line) {

  int i, len;

  clearLine(lineNum);

  len = strlen(line);
  println_8(line, (len > 21 ? 21 : len), 2, calcxpos(lineNum) + 1);
}

void OledScreen::clearLine(int lineNum) {
  uint8_t i, j;

  for (i = 0; i < 128; i++)
    for (j = 0; j < 11; j++)
      put_pixel(0, i, j + calcxpos(lineNum));

}

void OledScreen::invertLine(int lineNum) {

  int l1 = calcxpos(lineNum);
  int l2 = calcxpos(lineNum + 1);
  invert_area(l1,l2 - l1);
}

void OledScreen::draw_box_filled(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey ) {
  uint8_t i, j;

  sizex = sizex > 0x80 ?  0x80 : sizex;
  sizey = sizey > 0x40 ?  0x40 : sizey;

  for (i = 0; i < sizey; i++)
    for (j = 0; j < sizex; j++)
      put_pixel(1, x + i, y + j);

}

void OledScreen::draw_box(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t color) {
  uint8_t i;

  sizex = sizex > 0x80 ?  0x80 : sizex;
  sizey = sizey > 0x40 ?  0x40 : sizey;

  for (i = 0; i < sizex; i++) {
    put_pixel(color, i + x, y);
    put_pixel(color, i + x, y + sizey - 1);
  }
  for (i = 0; i < sizey; i++) {
    put_pixel(color, x, y + i);
    put_pixel(color, x + sizex - 1, y + i);
  }
}

// bresenham line algorithm from http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
void OledScreen::draw_line(int x0, int y0, int x1, int y1, unsigned int color) {

  x0 &= 0x7f;   // just constrain them here to screen size
  y0 &= 0x3f;
  x1 &= 0x7f;
  y1 &= 0x3f;

  int dy = y1 - y0;
  int dx = x1 - x0;
  int stepx, stepy;
  int fraction;

  if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
  if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
  dy <<= 1;                                                  // dy is now 2*dy
  dx <<= 1;                                                  // dx is now 2*dx

  put_pixel(color, x0, y0);
  if (dx > dy) {
    fraction = dy - (dx >> 1);                         // same as 2*dy - dx
    while (x0 != x1) {
      if (fraction >= 0) {
        y0 += stepy;
        fraction -= dx;                                // same as fraction -= 2*dx
      }
      x0 += stepx;
      fraction += dy;                                    // same as fraction -= 2*dy
      put_pixel(color, x0, y0);
    }
  } else {
    int fraction = dx - (dy >> 1);
    while (y0 != y1) {
      if (fraction >= 0) {
        x0 += stepx;
        fraction -= dy;
      }
      y0 += stepy;
      fraction += dx;
      put_pixel(color, x0, y0);
    }
  }
}

// bresenham circle algorithm
void OledScreen::draw_circle(unsigned int h, unsigned int k, unsigned int r, unsigned int color) {

  h &= 0x7f;   // just constrain them here to screen size
  k &= 0x3f;

  int x = 0;
  int y = r;
  int p = (3 - (2 * r));

  do
  {
    put_pixel(color, (h + x), (k + y));
    put_pixel(color, (h + y), (k + x));
    put_pixel(color, (h + y), (k - x));
    put_pixel(color, (h + x), (k - y));
    put_pixel(color, (h - x), (k - y));
    put_pixel(color, (h - y), (k - x));
    put_pixel(color, (h - y), (k + x));
    put_pixel(color, (h - x), (k + y));

    x++;

    if (p < 0) {
      p += ((4 * x) + 6);
    } else {
      y--;
      p += ((4 * (x - y)) + 10);
    }
  }
  while (x <= y);
}

// bresenham circle algorithm, filled
void OledScreen::draw_filled_circle(unsigned int h, unsigned int k, unsigned int r, unsigned int color) {

  h &= 0x7f;   // just constrain them here to screen size
  k &= 0x3f;

  int x = 0;
  int y = r;
  int p = (3 - (2 * r));

  do
  {
    draw_line(h+x, k+y, h+x, k-y, color); 
    draw_line(h+y, k+x, h+y, k-x, color); 
    draw_line(h-x, k+y, h-x, k-y, color); 
    draw_line(h-y, k-x, h-y, k+x, color); 

    x++;

    if (p < 0) {
      p += ((4 * x) + 6);
    } else {
      y--;
      p += ((4 * (x - y)) + 10);
    }
  }
  while (x <= y);
}
void OledScreen::fill_area(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t color) {
  uint8_t i, j;

  sizex = sizex > 0x80 ?  0x80 : sizex;
  sizey = sizey > 0x40 ?  0x40 : sizey;

  for (i = 0; i < sizex; i++)
    for (j = 0; j < sizey; j++)
      put_pixel(color, x + i, y + j);

}

void OledScreen::println(const char * line, int x, int y, int h, int color) {
  int i, deltax;
  deltax = x;

  int len = strlen(line);

  for (i = 0; i < len; i++) {
    if (h == 8) {
      deltax += put_char_small(line[i], deltax, y, color);
      deltax += 1;
    }
    else if (h == 16) {
      deltax += put_char_arial16(line[i], deltax, y, color);
      deltax += 2;
    }
    else if (h == 24) {
      deltax += put_char_arial24(line[i], deltax, y, color);
      deltax += 3;
    }
    else if (h == 32) {
      deltax += put_char_arial32(line[i], deltax, y, color);
      deltax += 4;
    }
  }
}

void OledScreen::println_8(const char * line, int len, int x, int y) {
  int i, deltax;
  deltax = x;
  for (i = 0; i < len; i++) {
    deltax += put_char_small(line[i], deltax, y, 1);
    deltax += 1;
  }
}

void OledScreen::put_pixel(unsigned int on, unsigned int x, unsigned int y) {

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

  if (on) {
    tmp8 |= (1 << (y & 0x7));
  }
  else {
    tmp8 &= ~(1 << (y & 0x7));
  }

  pix_buf[(page * 128) + column] = tmp8;
}

void OledScreen::invert_screen(void) {
  unsigned int i, x, y;

  for (i = 0; i < 1024; i++)
    pix_buf[i] = ~pix_buf[i];

}

void OledScreen::invert_area(unsigned int x, unsigned int y, unsigned int sizex, unsigned sizey) {
  sizex = sizex > 0x80 ?  0x80 : sizex;
  sizey = sizey > 0x40 ?  0x40 : sizey;

  for (int i = x; i < x + sizex; i++) {
    for (int j = y; j < y + sizey; j++) {
      put_pixel(!get_pixel(i, j) , i, j);
    }
  }
}

void OledScreen::invert_area(unsigned int y,unsigned int sizey) {
  invert_area(0, y, 128, sizey );
}

unsigned int OledScreen::get_pixel(unsigned int x, unsigned int y) {

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


unsigned int OledScreen::put_char_arial32(unsigned char character, unsigned int y, unsigned int x, unsigned int color) {
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;

  if (character < 32) character = 32;
  if (character > 127) character = 127;

  if (character == 32)
    return 8;

  character -= 33;

  charWidth = arial32Width[character + 1];
  charOffset = arial32Offset[character] * 4;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 8; j++) {
      for (k = 0; k < charWidth; k++) {
        if ((arial32[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k),  (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}

unsigned int OledScreen::put_char_arial24(unsigned char character, unsigned int y, unsigned int x, unsigned int color) {
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;

  if (character < 32) character = 32;
  if (character > 127) character = 127;

  if (character == 32)
    return 6;

  character -= 33;

  charWidth = arial24Width[character + 1];
  charOffset = arial24Offset[character] * 3;



  for (i = 0; i < 3; i++) {
    for (j = 0; j < 8; j++) {
      for (k = 0; k < charWidth; k++) {
        if ((arial24[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k), (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}

unsigned int OledScreen::put_char_arial16(unsigned char character, unsigned int y, unsigned int x, unsigned int color) {
  int i;
  int j;
  int k;
  int charWidth;
  int charOffset;

  if (character < 32) character = 32;
  if (character > 127) character = 127;

  if (character == 32)
    return 4;

  character -= 33;

  charWidth = arial16Width[character + 1];
  charOffset = arial16Offset[character] * 2;



  for (i = 0; i < 2; i++) {
    for (j = 0; j < 8; j++) {
      for (k = 0; k < charWidth; k++) {
        if ((arial16[charOffset + k + (i * charWidth)] >> j) & 0x01)
          put_pixel(color, (y + k), (x + (i * 8) + j));
      }
    }
  }
  return charWidth;
}


unsigned int OledScreen::put_char_small(unsigned char c, unsigned int y, unsigned int x, unsigned int color) {
  int i, j;
  if (c < 32) c = 32;
  if (c > 127) c = 127;

  c -= 32;
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 8; j++) {
      if ((characters[(c * 5) + i] >> j) & 0x01)
        put_pixel(color, y + i, x + j);
    }
  }
  return 5;
}



void OledScreen::drawNotification(const char * line) {

    int i, len;

    // first clear it out
    for (i = 0; i < 128; i++)
        pix_buf[i] = 0;

    len = strlen(line);
    println_8(line, len>21 ? 21: len,  2, 0);
}

void OledScreen::drawNotification(const char * line, int pwrStatus, int batteryLevel, int wifiStatus) {
    int i, len;

    // first clear it out
    for (i = 0; i < 128; i++)
        pix_buf[i] = 0;

   if (pwrStatus) drawBatteryMeter(batteryLevel);
   else drawPlug();
   drawWifiMeter(wifiStatus);
      
   len = strlen(line);
   println_8(line, len>14 ? 14: len,  2, 0);
}

void OledScreen::drawBatteryMeter(int lev) {
    
    int x = 112;
    int y = 0;
    draw_box(x, y, 13, 7, 1);
    draw_line(x + 13, y + 2, x + 13, y + 4, 1);
    put_pixel(0, x, y);
    put_pixel(0, x, y + 6);  

    if (lev > 0) draw_line(x + 2, y + 2, x + 2, y + 4, 1);
    if (lev > 1) draw_line(x + 4, y + 2, x + 4, y + 4, 1);
    if (lev > 2) draw_line(x + 6, y + 2, x + 6, y + 4, 1);
    if (lev > 3) draw_line(x + 8, y + 2, x + 8, y + 4, 1);
    if (lev > 4) draw_line(x + 10, y + 2, x + 10, y + 4, 1);
}

void OledScreen::drawPlug(void) {
	    
    int x = 112;
    int y = 0;

    draw_box_filled(113, 1, 5, 11);
/*
    put_pixel(1, x + 0, y + 3);
    put_pixel(1, x + 1, y + 3);
    put_pixel(1, x + 2, y + 3);
    put_pixel(1, x + 3, y + 3);
    put_pixel(1, x + 4, y + 3);
    put_pixel(1, x + 5, y + 3);

    put_pixel(1, x + 6, y + 2);
    put_pixel(1, x + 6, y + 3);
    put_pixel(1, x + 6, y + 4);

    put_pixel(1, x + 6, y + 1);
    put_pixel(1, x + 6, y + 5);

    put_pixel(1, x + 7, y + 0);
    put_pixel(1, x + 7, y + 6);


    put_pixel(1, x + 8, y + 0);
    put_pixel(1, x + 8, y + 6);

    put_pixel(1, x + 9, y + 0);
    put_pixel(1, x + 9, y + 6);

    draw_line(x + 10, y + 0, x + 10, y + 6, 1);
    draw_line(x + 11, y + 1, x + 13, y + 1, 1);
    draw_line(x + 11, y + 5, x + 13, y + 5, 1);
*/


}

void OledScreen::drawWifiMeter(int lev) {
    int x = 95;
    int y =0;

/*    if (lev) {
        put_pixel(1, x + 0, y + 2);
        put_pixel(1, x + 1, y + 1);
        put_pixel(1, x + 2, y + 0);
        put_pixel(1, x + 2, y + 3);
        put_pixel(1, x + 3, y + 0);
        put_pixel(1, x + 3, y + 2);
        put_pixel(1, x + 4, y + 0);
        put_pixel(1, x + 4, y + 2);
        put_pixel(1, x + 4, y + 4);
        put_pixel(1, x + 4, y + 5);
        put_pixel(1, x + 4, y + 6);
        put_pixel(1, x + 5, y + 0);
        put_pixel(1, x + 5, y + 2);
        put_pixel(1, x + 6, y + 0);
        put_pixel(1, x + 6, y + 3);
        put_pixel(1, x + 7, y + 1);
        put_pixel(1, x + 8, y + 2);
    }
*/

   if (lev) {
        put_pixel(1, x + 0, y + 0);
        put_pixel(1, x + 1, y + 0);
        put_pixel(1, x + 2, y + 0);
        put_pixel(1, x + 3, y + 0);
        put_pixel(1, x + 4, y + 0);
        put_pixel(1, x + 5, y + 0);
        put_pixel(1, x + 6, y + 0);
        put_pixel(1, x + 7, y + 0);
        put_pixel(1, x + 8, y + 0);
        put_pixel(1, x + 9, y + 0);
        put_pixel(1, x + 10, y + 0);
        
	put_pixel(1, x + 1, y + 1);
	put_pixel(1, x + 5, y + 1);
	put_pixel(1, x + 9, y + 1);

	put_pixel(1, x + 2, y + 2);
	put_pixel(1, x + 5, y + 2);
	put_pixel(1, x + 8, y + 2);

	put_pixel(1, x + 3, y + 3);
	put_pixel(1, x + 5, y + 3);
	put_pixel(1, x + 7, y + 3);

	put_pixel(1, x + 4, y + 4);
	put_pixel(1, x + 5, y + 4);
	put_pixel(1, x + 6, y + 4);

	put_pixel(1, x + 5, y + 5);
	put_pixel(1, x + 5, y + 6);

    }
}

void OledScreen::drawInfoBar(int inR, int inL, int outR, int outL, int pwrStatus, int batteryLevel, int wifiStatus) {

  int i, len;
    
  // bounds for vu
  // i guess it goes up to 11 haha
  if (inR < 0) inR = 0;
  if (inR > 11) inR = 11;
  if (inL < 0) inL = 0;
  if (inL > 11) inL = 11;

  if (outR < 0) outR = 0;
  if (outR > 11) outR = 11;
  if (outL < 0) outL = 0;
  if (outL > 11) outL = 11;

  // first clear it out
  for (i = 0; i < 128; i++) {
    pix_buf[i] = 0;
    put_pixel(0, i, 8); // really is first 9 rows
  }

  if (pwrStatus) drawBatteryMeter(batteryLevel);
  else drawPlug();
  drawWifiMeter(wifiStatus);
  
  // draw input output
  put_char_small('I', 0, 0, 1);
  put_char_small('O', 45, 0, 1);

  // VU meter
  // after I
  // little guys
  for (i = 0; i<11; i+=1) {
    draw_box_filled((i * 3) + 8, 1, 1, 1);
    draw_box_filled((i * 3) + 8, 5, 1, 1);
  }

  // big guys
  for (i = 0; i < (inR); i++) {
    draw_box_filled((i * 3) + 7, 0, 3, 2);
  }

  for (i = 0; i < (inL); i++) {
    draw_box_filled((i * 3) + 7, 4, 3, 2);
  }

  // after O
  // small guys
  for (i = 0; i<11; i+=1) {
    draw_box_filled((i * 3) + 55, 1, 1, 1);
    draw_box_filled((i * 3) + 55, 5, 1, 1);
  }
  
  // big guys
  for (i = 0; i < (outR); i++) {
    draw_box_filled((i * 3) + 54, 0, 3, 2);
  }

  for (i = 0; i < (outL); i++) {
    draw_box_filled((i * 3) + 54, 4, 3, 2);
  }
}

void OledScreen::drawInfoBar(int inR, int inL, int outR, int outL) {

  int i, len;

  // bounds for vu
  // i guess it goes up to 11 haha
  if (inR < 0) inR = 0;
  if (inR > 11) inR = 11;
  if (inL < 0) inL = 0;
  if (inL > 11) inL = 11;

  if (outR < 0) outR = 0;
  if (outR > 11) outR = 11;
  if (outL < 0) outL = 0;
  if (outL > 11) outL = 11;

  // first clear it out
  for (i = 0; i < 128; i++) {
    pix_buf[i] = 0;
    put_pixel(0, i, 8); // really is first 9 rows
  }

  // draw input output
  put_char_small('I', 0, 0, 1);
  put_char_small('O', 64, 0, 1);


  // VU meter
  // after I
  // little guys
  draw_box_filled(7,  1, 1, 2);
  draw_box_filled(12, 1, 1, 2);
  draw_box_filled(17, 1, 1, 2);
  draw_box_filled(22, 1, 1, 2);
  draw_box_filled(27, 1, 1, 2);
  draw_box_filled(32, 1, 1, 2);
  draw_box_filled(37, 1, 1, 2);
  draw_box_filled(42, 1, 1, 2);
  draw_box_filled(47, 1, 1, 2);
  draw_box_filled(52, 1, 1, 2);
  draw_box_filled(57, 1, 1, 2);

  draw_box_filled(7,  5, 1, 2);
  draw_box_filled(12, 5, 1, 2);
  draw_box_filled(17, 5, 1, 2);
  draw_box_filled(22, 5, 1, 2);
  draw_box_filled(27, 5, 1, 2);
  draw_box_filled(32, 5, 1, 2);
  draw_box_filled(37, 5, 1, 2);
  draw_box_filled(42, 5, 1, 2);
  draw_box_filled(47, 5, 1, 2);
  draw_box_filled(52, 5, 1, 2);
  draw_box_filled(57, 5, 1, 2);

  // big guys
  for (i = 0; i < (inR); i++) {
    draw_box_filled(7 + (5 * i), 0, 3, 4);
  }

  for (i = 0; i < (inL); i++) {
    draw_box_filled(7 + (5 * i), 4, 3, 4);
  }

  // after O
  // small guys
  draw_box_filled(73, 1, 1, 2);
  draw_box_filled(78, 1, 1, 2);
  draw_box_filled(83, 1, 1, 2);
  draw_box_filled(88, 1, 1, 2);
  draw_box_filled(93, 1, 1, 2);
  draw_box_filled(98, 1, 1, 2);
  draw_box_filled(103, 1, 1, 2);
  draw_box_filled(108, 1, 1, 2);
  draw_box_filled(113, 1, 1, 2);
  draw_box_filled(118, 1, 1, 2);
  draw_box_filled(123, 1, 1, 2);

  draw_box_filled(73, 5, 1, 2);
  draw_box_filled(78, 5, 1, 2);
  draw_box_filled(83, 5, 1, 2);
  draw_box_filled(88, 5, 1, 2);
  draw_box_filled(93, 5, 1, 2);
  draw_box_filled(98, 5, 1, 2);
  draw_box_filled(103, 5, 1, 2);
  draw_box_filled(108, 5, 1, 2);
  draw_box_filled(113, 5, 1, 2);
  draw_box_filled(118, 5, 1, 2);
  draw_box_filled(123, 5, 1, 2);

  // big guys
  for (i = 0; i < (outR); i++) {
    draw_box_filled(73 + (5 * i), 0, 3, 4);
  }

  for (i = 0; i < (outL); i++) {
    draw_box_filled(73 + (5 * i), 4, 3, 4);
  }
}

