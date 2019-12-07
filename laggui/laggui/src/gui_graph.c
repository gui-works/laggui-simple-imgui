// graphics wrapper for gui system
//
// maps onto gr32 graphics library
//
// this supports a stack of translation transformations to support
// container widgets, but this isn't implement in the IMGUI implementation
// included

#include <stdlib.h>
#include "gui_graph.h"
#include "gr.h"

#define MAX_STACK  128

static struct
{
   Bitmap previous;
   int x,y;
   int ox,oy;
} regionstack[MAX_STACK];
static int stack_index=0;

static int dx,dy;

// x,y,w,h are the region of the window within the current screen
// ox,oy are the offset to add to everything relative to this window
void guiGraphPushRegion(int x, int y, int w, int h, int ox, int oy)
{
   Bitmap region,current;

   current = grOutputGet();
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   region  = grSubBitmapWH(current, x-dx,y-dy,w,h);

   regionstack[stack_index].previous = current;
   regionstack[stack_index].x = x - ox;
   regionstack[stack_index].y = y - oy;
   regionstack[stack_index].ox = dx;
   regionstack[stack_index].oy = dy;
   ++stack_index;

   grOutputSet(&region);
   dx = ox;
   dy = oy;
}

void guiGraphPopRegion(int *sx, int *sy)
{
   int n = --stack_index;
   *sx = regionstack[n].x;
   *sy = regionstack[n].y;
   dx  = regionstack[n].ox;
   dy  = regionstack[n].oy;
   grOutputSet(&regionstack[n].previous);
}

void guiGraphText(int x, int y, char *str, Color color)
{
   grText(x-dx,y-dy,str, color);
}

void guiGraphTextN(int x, int y, char *str, int n, Color color)
{
   grTextN(x-dx,y-dy,str,n, color);
}

void guiGraphRect(int x0, int y0, int x1, int y1, Color color)
{
   grDrawRectOutline(x0-dx,y0-dy,x1-dx,y1-dy, color);
}

void guiGraphTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color)
{
   grDrawTriangle(x0-dx,y0-dy,x1-dx,y1-dy,x2-dx,y2-dy, color);
}

void guiGraphPoint(int x, int y, int size, Color color)
{
   x += dx;
   y += dy;
   if (size <= 1)
      grDrawPoint(x,y, color);
   else
      grDrawRectSolid(x-size,y-size, x+size,y+size,color);
}

void guiGraphSolidRect(int x0, int y0, int x1, int y1, Color color)
{
   grDrawRectSolid(x0-dx,y0-dy,x1-dx,y1-dy, color);
}

void guiGraphSolidRectWH(int x0, int y0, int w, int h, Color color)
{
   grDrawRectSolidWH(x0, y0, w, h, color);
}

static Color weight(Color src, float weight)
{
   int r = R(src) * weight;
   int g = G(src) * weight; 
   int b = B(src) * weight;
   int a = A(src);
   if (r > 255) r = 255;
   if (g > 255) g = 255;
   if (b > 255) b = 255;
   return RGBA(r,g,b,a);
}

void guiGraph3dRect(int x0, int y0, int x1, int y1, float w0, float w1, float w2, float w3, Color color)
{
   Color temp;
   if (x1 > x0+4 && y1 > y0 + 4)
      guiGraphSolidRect(x0+2,y0+2,x1-2,y1-2, color);

   temp = weight(color, w0);
   guiGraphLine(x0,y1-1,x0,y0, temp);
   guiGraphLine(x0+1,y0,x1-1,y0, temp);

   temp = weight(color, w1);
   guiGraphLine(x0+1,y1-2,x0+1,y0+1, temp);
   guiGraphLine(x0+2,y0+1,x1-2,y0+1, temp);

   temp = weight(color, w2);
   guiGraphLine(x0+1,y1-1,x1-1,y1-1, temp);
   guiGraphLine(x1-1,y1-2,x1-1,y0+1, temp);

   temp = weight(color, w3);
   guiGraphLine(x0,y1,x1,y1, temp);
   guiGraphLine(x1,y1-1,x1,y0, temp);
}

#define W0  0.3
#define W1  0.55
#define W2  1.16
#define W3  1.5

void guiGraphRaisedRect(int x0, int y0, int x1, int y1, Color color)
{
   guiGraph3dRect(x0,y0,x1,y1, W3,W2,W1,W0, color);
}

void guiGraphRaisedRectAlt(int x0, int y0, int x1, int y1, Color color)
{
   guiGraph3dRect(x0,y0,x1,y1, W2,W3,W1,W0, color);
}

void guiGraphLoweredRect(int x0, int y0, int x1, int y1, Color color)
{
   guiGraph3dRect(x0,y0,x1,y1, W0,W1,W2,W3, color);
}


void guiGraphLine(int x0, int y0, int x1, int y1, Color color)
{
   grDrawLine(x0-dx,y0-dy, x1-dx,y1-dy, color);
}

Bool guiGraphClipped(int x0, int y0, int x1, int y1)
{
   // gr32 automatically tests against subregions
   // given the way we've implemented subregions here
   return grIsClipped(x0-dx,y0-dy, x1-dx,y1-dy);
}

Bool guiGraphPointVis(int x, int y)
{
   return ! grIsClipped(x-dx, y-dy, x-dx, y-dy);
}

void guiGraphBitmap(void *bitmap, int x, int y, int centered)
{
   Bitmap *b = bitmap;
   if (centered)
      x -= b->width/2, y -= b->height/2;
   grDrawBitmap(x-dx, y-dy, b);
}

void *guiMakeBitmapAlpha(int w, int h, uint8 *bits, Color color)
{
   int x,y;
   Bitmap *bm = malloc(sizeof(*bm));
   *bm = grAllocBitmap(w, h, ALPHA_8bit);
   for (y=0; y < h; ++y)
      for (x=0; x < w; ++x)
         bm->bits[y*w+x] = (color & 0xffffff) + (*bits++ << 24);
   return bm;
}
