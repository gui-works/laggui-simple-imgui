#ifndef INC_PLATFORM_H
#define INC_PLATFORM_H

#include "types.h"

// functions the platform calls in the app
extern void appInit(void);
extern void eventMouse(int event, int x, int y, Bool shift, Bool ctrl);
extern void eventPaint(void);
extern void eventSize(int w, int h);
extern void appUpdate(float dt);

// this structure MUST be laid out to match the packed Color in gr.h
// (and the platform must convert if the result is non-native)
typedef struct
{
#ifdef WIN32
   uint8 b,g,r,a;       // little endian
#else
   uint8 a,r,g,b;       // big endian
#endif
} RGBA_struct;

extern void platformDrawBitmap(int x, int y, RGBA_struct *bits, int w, int h, int stride);
extern void platformRedrawAll(void);
extern void platformCapture(Bool);

#endif
