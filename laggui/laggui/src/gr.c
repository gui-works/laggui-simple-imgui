// simple 500-line 32-bit graphics library
// placed in the public domain Nov 9, 2003 by the author, Sean Barrett

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "gr.h"

#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

// alpha-blend routine; DO NOT USE if src alpha is 255,
// because it will be slightly transparent instead of
// fully opaque. caller must special-case fully opaque
// (which is good for speed anyway)
static Color blend(Color dest, Color src)
{
   // accelerated blend computes r and b in parallel
   uint8 a = A(src);
   uint rb_src  = src  & 0xff00ff;
   uint rb_dest = dest & 0xff00ff;
   uint rb      = rb_dest + ((rb_src - rb_dest) * a >> 8);
   uint g_dest  = (dest & 0x00ff00);
   uint g       = g_dest + (((src & 0xff00) - (dest & 0xff00)) * a >> 8);
   // note we do not compute a real dest alpha
   return (rb & 0xff00ff) + (g & 0x00ff00) + 0xff000000; 
}

static Color blendMultiplyFont(Color dest, Color font, Color src)
{
   // accelerated blend computes r and b in parallel
   uint8 a = A(font);
   uint rb_src  = src  & 0xff00ff;
   uint rb_dest = dest & 0xff00ff;
   uint rb      = rb_dest + ((rb_src - rb_dest) * a >> 8);
   uint g_dest  = (dest & 0x00ff00);
   uint g       = g_dest + (((src & 0xff00) - (dest & 0xff00)) * a >> 8);
   // note we do not compute a real dest alpha
   return (rb & 0xff00ff) + (g & 0x00ff00) + 0xff000000; 
}

static Bitmap output;
Rect active;

static Bool rectContains(Rect *a, Rect *b) // true if a contains b
{
   return a->x0 <= b->x0 && a->x1 >= b->x1 && a->y0 <= b->y0 && a->y1 >= b->y1;
}

// this accelerates black text on whie, and nothing else. goofy!
static Color blacktext[256];

void grInit(void)
{
   int i;
   for (i=0; i < 256; ++i)
      blacktext[i] = 0x00010101 * (255 - i) + 0xff000000;
}

// set current output "device"
void grOutputSet(Bitmap *dest)
{
   static Bool first = True;
   if (first) {
      first = False;
      grInit();
   }

   output = *dest;   // copy dest so we ignore future changes to it
   active.x0 = 0;
   active.y0 = 0;
   active.x1 = dest->width;
   active.y1 = dest->height;
}

Bitmap grOutputGet(void)
{
   return output;
}

// return true if this box is entirely clipped away
Bool grIsClipped(int x0, int y0, int x1, int y1)
{
   return x0 >= active.x1 || x1 <= active.x0 || y0 >= active.y1 || y1 <= active.y0;
}

void grDrawPoint(int x, int y, Color color)
{
   if (x >= active.x0 && x < active.x1 && y >= active.y0 && y < active.y1) {
      if (IS_OPAQUE(color)) {
         output.bits[y*output.stride + x] = color;
      } else if (!IS_INVIS(color)) {
         int off = y*output.stride + x;
         output.bits[off] = blend(output.bits[off], color);
      }
   }
}

void grDrawHLine (int y, int x0, int x1, Color color)
{
   if (y < active.y0 || y >= active.y1) return;
   if (x0 < x1) {
      if (x0 >= active.x1 || x1 < active.x0)
         return;
   } else {
      int t;
      if (x1 >= active.x1 || x0 < active.x0)
         return;
      t = x0, x0 = x1, x1 = t;
   }
   if (IS_INVIS(color)) return;

   x0 = max(x0, active.x0);
   x1 = min(x1, active.x1-1);

   {
      int n = x1 - x0 + 1;
      Color *out;

      out = &output.bits[x0 + y * output.stride];

      if (IS_OPAQUE(color))
      {
         do
             *out++ = color;
         while (--n > 0);
      } else {
         do {
             *out = blend(*out, color);
             ++out;
         } while (--n > 0);
      }
   }
}

void grDrawVLine (int x, int y0, int y1, Color color)
{
   if (x < active.x0 || x >= active.x1) return;
   if (y0 < y1) {
      if (y0 >= active.y1 || y1 < active.y0)
         return;
   } else {
      int t;
      if (y1 >= active.y1 || y0 < active.y0)
         return;
      t = y0, y0 = y1, y1 = t;
   }
   if (IS_INVIS(color)) return;

   y0 = max(y0, active.y0);
   y1 = min(y1, active.y1-1);

   {
      int n = y1 - y0 + 1;
      Color *out;
      int stride = output.stride;

      out = &output.bits[x + y0 * output.stride];

      if (IS_OPAQUE(color))
      {
         do {
             *out = color;
             out += stride;
         } while (--n > 0);
      } else {
         do {
             *out = blend(*out, color);
             out += stride;
         } while (--n > 0);
      }
   }
}

void grDrawLine  (int x0, int y0, int x1, int y1, Color color)
{
   if (x0 == x1)
      grDrawVLine(x0,y0,y1, color);
   else if (y0 == y1)
      grDrawHLine(y0,x0,x1, color);
   else {
      // not the fastest way to draw a line, but who wants to write 8 cases?
      int i;
      int dx = x1 - x0;
      int dy = y1 - y0;
      int x = x0, y = y0;
      int max_len;
      float inv_len;
      Bool is_opaque;

      Rect temp;
      temp.x0 = min(x0,x1);
      temp.y0 = min(y0,y1);
      temp.x1 = max(x0,x1)+1;
      temp.y1 = max(y0,y1)+1;
   
      if (temp.x0 >= active.x1 || temp.x1 <= active.x0) return;
      if (temp.y0 >= active.y1 || temp.y1 <= active.y0) return;
      if (IS_INVIS(color)) return;

      is_opaque = IS_OPAQUE(color);

      max_len = max(abs(dx),abs(dy));
      inv_len = 65536.0 / max_len;

      dx = dx * inv_len;
      dy = dy * inv_len;
      x = (x << 16) + 32768;
      y = (y << 16) + 32768;

      // does the line need clipping?
      if (!rectContains(&active, &temp)) {
         // this is a very slow and dumb way to clip!
         for (i=0; i <= max_len; x += dx, y += dy, ++i) {
            if (   (x >> 16) >= active.x0 && (x >> 16) < active.x1
                && (y >> 16) >= active.y0 && (y >> 16) < active.y1) {
               int offset = output.stride * (y >> 16) + (x >> 16);
               if (is_opaque)
                  output.bits[offset] = color;
               else
                  output.bits[offset] = blend(output.bits[offset], color);
            }
         }
      } else if (is_opaque) {
         do {
            int offset = output.stride * (y >> 16) + (x >> 16);
            output.bits[offset] = color;
            x += dx, y += dy, --max_len;
         } while (max_len > 0);
      } else {
         do {
            int offset = output.stride * (y >> 16) + (x >> 16);
            output.bits[offset] = blend(output.bits[offset], color);
            x += dx, y += dy, --max_len;
         } while (max_len > 0);
      }
   }
}

void grDrawRectOutlineWH(int x0, int y0, int w , int h , Color color)
{
   if (h == 1)      grDrawHLine(y0, x0, x0+w-1, color);
   else if (w == 1) grDrawVLine(x0, y0, y0+h-1, color);
   else if (h > 1 && w > 1) {
      int x1 = x0+w-1, y1 = y0+h-1;
      grDrawHLine(y0,x0,x1-1, color);
      grDrawVLine(x1,y0,y1-1, color);
      grDrawHLine(y1,x0+1,x1, color);
      grDrawVLine(x0,y0+1,y1, color);
   }
}

void grDrawRectOutline  (int x0, int y0, int x1, int y1, Color color)
{
   if (x1 < x0) { int t=x0; x0=x1; x1=t; }
   if (y1 < y0) { int t=y0; y0=y1; y1=t; }
   grDrawRectOutlineWH(x0,y0, x1-x0+1, y1-y0+1, color);
}

void grDrawRectSolidWH(int x0, int y0, int w , int h , Color color)
{
   if (w > 0) {   
      int j, x1 = x0 + w -1;
      for (j=0; j < h; ++j)
         grDrawHLine(y0+j, x0, x1, color);
   }
}
  
void grDrawRectSolid  (int x0, int y0, int x1, int y1, Color color)
{
   if (x1 < x0) { int t=x0; x0=x1; x1=t; }
   if (y1 < y0) { int t=y0; y0=y1; y1=t; }
   grDrawRectSolidWH(x0,y0, x1-x0+1, y1-y0+1, color);
}

static Color blendMultiply(Color dest, Color src1, Color src2)
{
   short sr = R(src1)*(short)R(src2) >> 8;
   short sg = G(src1)*(short)G(src2) >> 8;
   short sb = B(src1)*(short)B(src2) >> 8;
   short sa = A(src1)*(short)A(src2) >> 8;

   short r = sr +  (sr >> 7);      // 0..255
   short g = sg +  (sg >> 7);
   short b = sb +  (sb >> 7);
   short a = sa + ((sa >> 6) & 2); // 0..256

   short dr = r - R(dest);
   short dg = g - G(dest);
   short db = b - B(dest);

   uint8 or = R(dest) + (uint8) ((dr * a) >> 8);
   uint8 og = G(dest) + (uint8) ((dg * a) >> 8);
   uint8 ob = B(dest) + (uint8) ((db * a) >> 8);

   // no destination alpha
   return RGB(or,og,ob);
}

static void drawBitmap(Bitmap *dest, Rect *bounds, int x, int y, Bitmap src, Color *cp)
{
   Rect src_box = { x,y, x+src.width, y+src.height };
   // check if the bitmap is not entirely onscreen
   if (!rectContains(bounds, &src_box))
   {
      int x0=0,y0=0,x1=src.width,y1=src.height;

      if (bounds->x0 >= src_box.x1) return;
      if (bounds->x1 <= src_box.x0) return;
      if (bounds->y0 >= src_box.y1) return;
      if (bounds->y1 <= src_box.y0) return;

      if (x            < bounds->x0) x0 = bounds->x0 - x;
      if (x+src.width  > bounds->x1) x1 = bounds->x1 - x;
      if (y            < bounds->y0) y0 = bounds->y0 - y;
      if (y+src.height > bounds->y1) y1 = bounds->y1 - y;

      src = grSubBitmap(src, x0,y0,x1,y1);
      assert(src.width > 0);
      assert(src.height > 0);

      x += x0;
      y += y0;
   }

   // now the bitmap is clipped to be strictly onscreen
   {
      Color *out = &dest->bits[dest->stride * y + x];
      Color *in   =   src.bits;
      int j,i;
      if (cp != NULL) {
         if (src.alpha == ALPHA_font && IS_OPAQUE(*cp)) {
            Color c = *cp;
            for (j=0; j < src.height; ++j) {
               if (c == 0xff000000) {
                  for (i=0; i < src.width; ++i) {
                     if (!IS_INVIS(in[i]))
                        if (out[i] == 0xffffffff)
                           out[i] = blacktext[A(in[i])];
                        else 
                           out[i] = blendMultiplyFont(out[i], in[i], c);
                  }
               } else {
                  for (i=0; i < src.width; ++i) {
                     if (!IS_INVIS(in[i]))
                        out[i] = blendMultiplyFont(out[i], in[i], c);
                  }
               }
               out += dest->stride;
               in  +=  src .stride;
            }
         } else {
            Color c = *cp;
            for (j=0; j < src.height; ++j) {
               for (i=0; i < src.width; ++i) {
                  if (!IS_INVIS(in[i]))
                     out[i] = blendMultiply(out[i], in[i], c);
               }
               out += dest->stride;
               in  +=  src .stride;
            }
         }
      } else if (src.alpha == ALPHA_opaque) {
         for (j=0; j < src.height; ++j) {
            memcpy(out, in, sizeof(Color) * src.width);
            out += dest->stride;
            in  +=  src .stride;
         }
      } else if (src.alpha == ALPHA_1bit) {
         for (j=0; j < src.height; ++j) {
            for (i=0; i < src.width; ++i) {
               if (!IS_INVIS(in[i]))
                  out[i] = in[i];
            }
            out += dest->stride;
            in  +=  src .stride;
         }
      } else {
         for (j=0; j < src.height; ++j) {
            for (i=0; i < src.width; ++i) {
               if (IS_OPAQUE(in[i]))
                  out[i] = in[i];
               else if (!IS_INVIS(in[i]))
                  out[i] = blend(out[i], in[i]);
            }
            out += dest->stride;
            in  +=  src .stride;
         }
      }
   }
}

void grDrawBitmap(int x, int y, Bitmap *src)
{
   drawBitmap(&output, &active, x, y, *src, NULL);
}

void grDrawBitmapColored(int x, int y, Bitmap *src, Color c)
{
   drawBitmap(&output, &active, x, y, *src, c != 0xFFFFFFFF ? &c : NULL);
}

void grDrawBitmapTo(Bitmap *dest, int x, int y, Bitmap *src)
{
   Rect bounds = { 0,0,dest->width, dest->height };
   drawBitmap(dest, &bounds, x, y, *src, NULL);
}

Bitmap grCreateBitmap(int width, int height, AlphaType alpha, Color *bits)
{
   Bitmap b;
   b.bits = bits;
   b.width = b.stride = width;
   b.height = height;
   b.alpha = alpha;
   return b;
}

static Color *allocBits(int size)
{
   Color *c;
   if (size == 0) return NULL;
   c = malloc(size * sizeof(Color));
   memset(c, 255, size * sizeof(Color));  // force alpha to be opaque
   return c;
}

Bitmap grAllocBitmap(int width, int height, AlphaType alpha)
{
   return grCreateBitmap(width, height, alpha, allocBits(width*height));
}

void grFreeBitmap(Bitmap *b)
{
   if (b->bits == NULL) return;
   free(b->bits);
   b->bits = NULL;
}

Bitmap grSubBitmap(Bitmap src, int x0, int y0, int x1, int y1)
{
   Bitmap out;
   assert(x0 <= x1);
   assert(y0 <= y1);
   if (x0 < 0) x0 = 0;
   if (y0 < 0) y0 = 0;
   if (x1 > src.width) x1 = src.width;
   if (y1 > src.height) y1 = src.height;

   out.bits = &src.bits[x0 + y0*src.stride];
   out.width = x1-x0;
   out.height = y1-y0;
   out.stride = src.stride;
   out.alpha = src.alpha;

   return out;
}

Bitmap grSubBitmapWH(Bitmap src, int x0, int y0, int w, int h)
{
   return grSubBitmap(src, x0, y0, x0+w, y0+h);
}

void grCopyBitmap(Bitmap *dest, int x, int y, Bitmap src)
{
   // make a new bitmap which is the old one but marked opaque
   // and then just draw that with the normal drawing engine
   Bitmap from = src;
   from.alpha = ALPHA_opaque;
   grDrawBitmapTo(dest, x, y, &from);
}

Bitmap grAllocCopyBitmap(Bitmap src)
{
   Bitmap dest = src;
   dest.bits = allocBits(dest.width * dest.height);
   dest.stride = dest.width;
   grCopyBitmap(&dest, 0,0, src);
   return dest;
}

Bitmap grAllocCopySubBitmap  (Bitmap src, int x0, int y0, int x1, int y1)
{
    return grAllocCopyBitmap(grSubBitmap(src,x0,y0,x1,y1));
}

Bitmap grAllocCopySubBitmapWH(Bitmap src, int x0,int y0, int w,int h)
{
   return grAllocCopySubBitmap(src, x0, y0, x0+w, y0+h);
}
