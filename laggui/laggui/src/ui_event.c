// Remap event-driven interface to single-call-per-frame interface


#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "gr.h"
#include "ui.h"

void eventMouse(int event, int x, int y, Bool shift, Bool ctrl)
{
   ui.mx = x;
   ui.my = y;

   switch (event) {
      case E_leftup:      ui.left_up     = True; break;
      case E_leftdown:    ui.left_down   = True; break;
      case E_rightup:     ui.right_up    = True; break;
      case E_rightdown:   ui.right_down  = True; break;
      case E_middleup:    ui.middle_up   = True; break;
      case E_middledown:  ui.middle_down = True; break;
   }
}

void eventPaint(void)
{
   Bitmap screen = grOutputGet();
   platformDrawBitmap(0,0, (RGBA_struct *) screen.bits, screen.width, screen.height, screen.stride);
}

void eventSize(int w, int h)
{
   Bitmap screen = grOutputGet();
   if (screen.bits) grFreeBitmap(&screen);
   if (w < 24) w = 24;
   if (h < 24) h = 24;
   screen = grAllocBitmap(w, h, ALPHA_opaque);
   grOutputSet(&screen);
}
