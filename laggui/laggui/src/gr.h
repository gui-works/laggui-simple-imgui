#ifndef INC_GR_H
#define INC_GR_H

#include "types.h"
#include "gr_color.h"

#define IS_OPAQUE(c)   ((c) >= 0xFF000000)
#define IS_INVIS(c)    ((c) <= 0x00FFFFFF)

typedef enum
{
    ALPHA_opaque,     // ignore alpha
    ALPHA_1bit,       // treat non-0 alpha as opaque
    ALPHA_8bit,       // alpha = 0 .. 255 <--> 0% opaque .. 100% opaque
    ALPHA_font,       // color = 255,255,255, alpha is 0..255
} AlphaType;

typedef struct
{
    int x0,y0,x1,y1;
} Rect;

typedef struct
{
	Color *bits;
   int width; // in pixels
	int stride; // in pixels;
	short height; // in pixels
   short alpha;
} Bitmap;

extern Bitmap grCreateBitmap(int width, int height, AlphaType, Color *bits);
extern Bitmap grAllocBitmap(int width, int height, AlphaType alpha);
extern void   grFreeBitmap(Bitmap *b);

// set current output "device"
extern void   grOutputSet(Bitmap *dest);
extern Bitmap grOutputGet(void);

extern void grClipSet(int  x0, int  y0, int  x1, int  y1);
extern void grClipGet(int *x0, int *y0, int *x1, int *y1);
extern void grClipOff(void);

extern Bool grIsClipped(int x0, int y0, int x1, int y1);

// draw on current output
extern void grDrawPoint(int x, int y, Color color);                       
extern void grDrawLine  (int x0, int y0, int x1, int y1, Color color);
extern void grDrawRectOutline  (int x0, int y0, int x1, int y1, Color color);
extern void grDrawRectOutlineWH(int x0, int y0, int w , int h , Color color);
extern void grDrawRectSolid  (int x0, int y0, int x1, int y1, Color color);
extern void grDrawRectSolidWH(int x0, int y0, int w , int h , Color color);
extern void grDrawHLine(int y, int x0, int x1, Color color);
extern void grDrawVLine(int x, int y0, int y1, Color color);
extern void grDrawBitmap(int x, int y, Bitmap *src);
extern void grDrawBitmapTo(Bitmap *dest, int x, int y, Bitmap *src);
extern void grDrawBitmapColored(int x, int y, Bitmap *src, Color c);

// create a new bitmap that is the specified sub-region of src
extern Bitmap grSubBitmap(Bitmap src, int x0, int y0, int x1, int y1);
extern Bitmap grSubBitmapWH(Bitmap src, int x0, int y0, int w, int h);

// copy the pixels from bitmap 'src' to bitmap 'dest' at loc (x,y)
extern void grCopyBitmap(Bitmap *dest, int x, int y, Bitmap src);

// copy the data from bitmap 'src' into a new, malloc()ed-bits bitmap
extern Bitmap grAllocCopyBitmap(Bitmap src);

// copy a subrectangle of 'src' into a new, malloc()-ed bits bitmap
//   identical to  grAllocCopyBitmap(grSubBitmap(src,x0,y0,x1,y1))
extern Bitmap grAllocCopySubBitmap  (Bitmap src, int x0, int y0, int x1, int y1);
extern Bitmap grAllocCopySubBitmapWH(Bitmap src, int x0, int y0, int w , int h );

// additional functions from gr_extra.c
extern void grDrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color);
extern void grDrawPolygon(int *pts[2], int num, Color color);
extern void grMakeFont(Bitmap *chars, int start, int num, int pixel_height, int monospace);
extern void grSetFont(Bitmap *fontchars, int first_char, int num_chars);
extern void grGetFont(Bitmap **chars, int *first, int *num);
extern void grText(int x, int y, char *str, Color color);
extern void grTextN(int x, int y, char *str, int count, Color color);
extern int  grTextWidth(char *str);
extern int  grTextWidthN(char *str, int n);
extern int  grCharWidth(char c);
extern int  grCharHeight(char c);
extern int  grDecompressFont(uint32 *decode, Bitmap *chars, int num);

#endif
