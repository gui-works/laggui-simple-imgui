// graphics interface for gui system

#include "types.h"
#include "gr_color.h"

extern void guiGraphText(int x, int y, char *str, Color c);
extern void guiGraphTextN(int x, int y, char *str, int n, Color c);

extern void guiGraphRaisedRect(int x0, int y0, int x1, int y1, Color c);
extern void guiGraphLoweredRect(int x0, int y0, int x1, int y1, Color c);
extern void guiGraphRaisedRectAlt(int x0, int y0, int x1, int y1, Color c);

extern void guiGraphRect(int x0, int y0, int x1, int y1, Color c);
extern void guiGraphLine(int x0, int y0, int x1, int y1, Color c);
extern void guiGraphTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c);
extern void guiGraphPoint(int x, int y, int size, Color c);
extern void guiGraphSolidRect(int x0, int y0, int x1, int y1, Color c);
extern void guiGraphSolidRectWH(int x0, int y0, int w, int h, Color c);
extern void guiGraphBitmap(void *bitmap, int x, int y, int centered);
extern void *guiMakeBitmapAlpha(int w, int h, uint8 *bits, Color c);

extern void guiGraphPushRegion(int x, int y, int w, int h, int ox, int oy);
extern void guiGraphPopRegion(int *sx, int *sy);
extern Bool guiGraphClipped(int x0, int y0, int x1, int y1);
extern Bool guiGraphPointVis(int x, int y);

#define C_white  RGB_GREY(255)
#define C_black  RGB_GREY(0)
