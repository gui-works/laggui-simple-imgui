#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "gr.h"
#include "gr_plat.h"

#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

extern Rect active;

// a very dumb and inefficient triangle renderer,
// but should be good enough for small graphical elements
// or for building bitmaps which are then cached
void grDrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color)
{
   int y_min, y_max, y;

   // throw away if offscreen to left or right
   if (x0 < x1) {
      if (min(x0,x2) >= active.x1) return;
      if (max(x1,x2) <  active.x0) return;
   } else {
      if (min(x1,x2) >= active.x1) return;
      if (max(x0,x2) <  active.x0) return;
      if (x0 == x1 && x1 == x2) return;
   }

   // throw away if offscreen to top or bottom,
   // and compute min and max locations vertically
   if (y0 < y1) {
      y_min = min(y0,y2);
      y_max = max(y1,y2);
      if (y_min >= active.y1) return;
      if (y_max <  active.y0) return;
   } else {
      y_min = min(y1,y2);
      y_max = max(y0,y2);
      if (y_min >= active.y1) return;
      if (y_max <  active.y0) return;
      if (y_max == y_min) return;
      // resort them so y0 < y1
      {
         int t;
         t = y0, y0 = y1, y1 = t;
         t = x0, x0 = x1, x1 = t;
      }
   }

   if (y_min < active.y0) y_min = active.y0;
   if (y_max >= active.y1) y_max = active.y1-1;

   // now generate sets of vertices sorted in y...
   // right now, we know y0 < y1
   {
      int x4,y4,x5,y5;
      int x6,y6,x7,y7;

      // so check y0 vs. y2, and set up an edge pair
      // for them, with y4 is smaller, y5 is larger
      if (y0 < y2)
         x4 = x0, y4 = y0, x5 = x2, y5 = y2;
      else
         x4 = x2, y4 = y2, x5 = x0, y5 = y0;

      // now check y1 vs. y2, and set up an edge pair
      // for them with y6 smaller and y7 larger
      if (y1 < y2)
         x6 = x1, y6 = y1, x7 = x2, y7 = y2;
      else
         x6 = x2, y6 = y2, x7 = x1, y7 = y1;

      for (y=y_min; y <= y_max; ++y) {
         int x[3];
         int num=0;
         if (y0 <= y && y < y1) x[num++] = x0 + (x1-x0)*(y-y0)/(y1-y0);
         if (y4 <= y && y < y5) x[num++] = x4 + (x5-x4)*(y-y4)/(y5-y4);
         if (y6 <= y && y < y7) x[num++] = x6 + (x7-x6)*(y-y6)/(y7-y6);
         assert (num <= 2);
         if (num == 2) {
            grDrawHLine(y, x[0],x[1], color);
         }
      }
   }
}

void grDrawPolygon(int *pts[2], int num, Color color)
{
   int i;
   for (i=1; i < num-1; ++i)
      grDrawTriangle(pts[0][0], pts[0][1], pts[i][0], pts[i][1], pts[i+1][0], pts[i+1][1], color);
}

static Bitmap *fontchars;
static int start_char, num_chars;

void grSetFont(Bitmap *chars, int start, int num)
{
   fontchars = chars;
   start_char = start;
   num_chars = num;
}

void grGetFont(Bitmap **chars, int *start, int *num)
{
   if (chars) *chars = fontchars;
   if (start) *start = start_char;
   if ( num ) * num  = num_chars;
}

void grText(int x, int y, char *str, Color color)
{
   while (*str) {
      uint8 c = *str++;
      if (c < start_char || c >= start_char + num_chars)
         c = start_char;
      grDrawBitmapColored(x,y,&fontchars[c-start_char], color);
      x += fontchars[c - start_char].width;
      if (c == 'f' && *str == 't') --x;
   }
}

void grTextN(int x, int y, char *str, int count, Color color)
{
   while (*str && --count >= 0) {
      uint8 c = *str++;
      if (c < start_char || c >= start_char + num_chars)
         c = start_char;
      grDrawBitmapColored(x,y,&fontchars[c-start_char], color);
      x += fontchars[c - start_char].width;
      if (c == 'f' && *str == 't') --x;
   }
}

int grCharWidth(char c)
{
   if (c < start_char || c >= start_char + num_chars)
      c = start_char;
   return fontchars[c-start_char].width;
}

int  grCharHeight(char c)
{
   if (c < start_char || c >= start_char + num_chars)
      c = start_char;
   return fontchars[c-start_char].height;
}

int grTextWidth(char *str)
{
   int width=0;
   while (*str) {
      width += grCharWidth(*str++);
      if (str[-1] == 'f' && str[0] == 't') --width;
   }
   return width;
}

int grTextWidthN(char *str, int count)
{
   int width=0;
   while (*str && --count >= 0) {
      width += grCharWidth(*str++);
      if (str[-1] == 'f' && str[0] == 't') --width;
   }
   return width;
}

#define skipbit() \
   if ((buffer >>= 1), (--bits_left == 0)) { buffer=*decode++; bits_left = 32; } else

int grDecompressFont(uint32 *decode, Bitmap *chars, int num)
{
   int i,j;
   int start = (decode[0] >>  0) & 0xff;
   int count = (decode[0] >>  8) & 0xff;
   int h     = (decode[0] >> 16) & 0xff;
   int bits_left;
   int buffer;
   Color *c;

   assert(count == num);

   ++decode;
   for (i=0; i < num; ++i) {
      int w = (decode[i >> 2] >> ((i & 3) << 3)) & 0xff;
      chars[i] = grAllocBitmap(w, h, ALPHA_font);
   }
   decode += (num + 3) >> 2;
   
   buffer = *decode++;
   bits_left = 32;
   while (--num >= 0) {
      c = chars->bits;
      for (j=0; j < chars->height; ++j) {
         Bool z = buffer & 1;
         skipbit();
         if (!z) {
            for (i=0; i < chars->width; ++i)
               *c++ = RGBA(255,255,255,0);
         } else {
            for (i=0; i < chars->width; ++i) {
               z = buffer & 1;
               skipbit();
               if (!z)
                  *c++ = RGBA(255,255,255,0);
               else {
                  int n=0;
                  // read out a 3-bit number
                  n += n + (buffer & 1); skipbit();
                  n += n + (buffer & 1); skipbit();
                  n += n + (buffer & 1); skipbit();
                  n += 1;
                  *c++ = RGBA(255,255,255,(255*n)>>3);
               }
            }
         }
      }
      ++chars;      
   }
   
   return start;   
}
