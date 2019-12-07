#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "gr.h"
#include "ui.h"
#include "gui_graph.h"

typedef struct
{
   int x0,y0;
   int x1,y1;
   Color c;

   unsigned char hot_alpha;
} AppRect;


#define MAX_RECTS  10
AppRect *rects[MAX_RECTS];
int num_rects, selected_rect = -1;
AppRect *active_rect;

void doRect(int n, float dt);

int getCreatedRectLoc(int sz)
{
   static int pos = 120;

   Bitmap screen = grOutputGet();

   // if new rect would be offscreen, reset to starting position
   pos += 20;
   if (pos+sz > screen.width || pos+sz > screen.height)
      pos = 100;

   return pos;
}

void makeRect(void)
{
   int size=100;
   AppRect *r = malloc(sizeof(*r));
   rects[num_rects] = r;

   r->x0 = r->y0 = getCreatedRectLoc(size);
   r->x1 = r->x0 + size;
   r->y1 = r->y0 + size;

   r->c = (selected_rect != -1) ? rects[selected_rect]->c : RGB(200,100,200);
   r->hot_alpha = 255;

   selected_rect = num_rects++;
}

void duplicateRect(void)
{
   if (num_rects < MAX_RECTS) {
      AppRect *source = rects[selected_rect];
      AppRect *z;
      selected_rect = num_rects++;
      z = rects[selected_rect] = malloc(sizeof(*z));
      *z = *source;
      z->x0 += 5; z->y0 += 5;
      z->x1 += 5; z->y1 += 5;
   }
}

void deleteRect(void)
{
   free(rects[selected_rect]);
   for (;selected_rect < num_rects-1; ++selected_rect)
      rects[selected_rect] = rects[selected_rect+1];
   --num_rects;
   selected_rect = -1;
}

void editSelection(void)
{
   AppRect *z = rects[selected_rect];
   static int r,g,b; // make these static so we can use their addresses as ids
   static Bool numeric; // display numeric values on the RGB sliders?

   if (uiButton("Duplicate Selected", duplicateRect))
      duplicateRect();

   uiButtonToggle("Show Values", "Hide Values", &numeric);

   r = R(z->c)>>4;
   g = G(z->c)>>4;
   b = B(z->c)>>4;
   uiSliderIntDisplay("red",  0,15, &r, numeric);
   uiSliderIntDisplay("green",0,15, &g, numeric);
   uiSliderIntDisplay("blue", 0,15, &b, numeric);
   r = r*16 + 8;
   g = g*16 + 8;
   b = b*16 + 8;
   z->c = RGB(r,g,b);

   if (uiButton("Delete Selected", deleteRect))
      deleteRect();
}

void appUpdate(float dt)
{
   int i;

   Bitmap screen;

   uiFrameBegin();

   // begin drawing & updating here
   screen = grOutputGet();
   grDrawRectSolid(0,0,screen.width, screen.height, RGB(255,255,255));

   for (i=0; i < screen.height; i += 10)
      grDrawHLine(i, 0, screen.width, RGB(225,255,230));

   // detect click-on-nothing as unselect
   if (uiButtonLogicRect(0,0,screen.width,screen.height,appUpdate))
      selected_rect = -1;

   // draw all the rects, and allow them to be dragged, resized, and selected
   for (i=0; i < num_rects; ++i)
      doRect(i, dt);

   // if one is clicked on, move it on top
   if (selected_rect != -1) {
      if (selected_rect != num_rects-1) {
         int i;
         AppRect *r = rects[selected_rect];
         for (i=selected_rect; i < num_rects-1; ++i)
            rects[i] = rects[i+1];
         rects[i] = r;
         selected_rect = num_rects - 1;
      }
   }

   // display the panel on the left

   // make it grow with the screen width
   uiWidth(screen.width < 400 ? 100 : screen.width/4);

   if (num_rects < MAX_RECTS)
      // @TODO: should use this to control a disabled button mode,
      // rather than hiding button, so widgets don't move around
      if (uiButton("Create New", makeRect))
         makeRect();

   layout.cy += 10;


   if (selected_rect != -1)
      editSelection();

   uiWidth(WIDTH_auto);

   // done drawing & updating
   uiFrameEnd();
}

void doRect(int n, float dt)
{
   AppRect *r = rects[n];
   Bool center_hot;
   int s = 5;  // width of handle
   // allow resizing from any edge or corner

   ui.is_hot = False; // check whether any edges are currently being hovered
   ui.is_active = False;

   // drag center region to move whole thing
   {
      int x = (r->x0 + r->x1)/2;
      int y = (r->y0 + r->y1)/2;
      int xp = x, yp = y;
      // r aliases with &r->x0, so use ID2(r)
      if (uiDragXY(&x, abs(r->x1 - r->x0), &y, abs(r->y1 - r->y0), ID2(r))) {
         // move both corners by same amount == dragging
         r->x0 += (x - xp); r->y0 += (y - yp);
         r->x1 += (x - xp); r->y1 += (y - yp);
      }
   }

   center_hot = ui.is_hot; ui.is_hot = False;
         
   // edges first, so corners are on top
   uiDragX (&r->x0,s, r->y0,r->y1,   &r->x0);
   uiDragX (&r->x1,s, r->y0,r->y1,   &r->x1);

   uiDragY (r->x0,r->x1, &r->y0,s,   &r->y0);
   uiDragY (r->x0,r->x1, &r->y1,s,   &r->y1);

   ui.cur_index = 1; // change index id so that we can reuse same pointers as new handles
   s = 9; // corners have larger handles

   uiDragXY(&r->x0,s, &r->y0,s, &r->x0);
   uiDragXY(&r->x1,s, &r->y0,s, &r->y0);

   uiDragXY(&r->x0,s, &r->y1,s, &r->y1);
   uiDragXY(&r->x1,s, &r->y1,s, &r->x1);
   ui.cur_index = 0;

   if (!center_hot && !ui.is_hot) {
      r->hot_alpha = max(r->hot_alpha - 80 * dt, 200);
   } else {
      r->hot_alpha = min(r->hot_alpha + 80 * dt, 255);
   }
   guiGraphSolidRect(r->x0,r->y0, r->x1,r->y1, RGBA(R(r->c),G(r->c),B(r->c),r->hot_alpha));

   if (ui.is_hot) { // highlight borders when they're draggable
      guiGraphRect(r->x0  ,r->y0  , r->x1  ,r->y1  , RGB_GREY(0));
      guiGraphRect(r->x0+1,r->y0+1, r->x1-1,r->y1-1, RGB_GREY(255));
      guiGraphRect(r->x0+2,r->y0+2, r->x1-2,r->y1-2, RGB_GREY(0));
   } else {
      guiGraphRect(r->x0,r->y0, r->x1,r->y1, RGB_GREY(0));
   }

   if (ui.is_active)
      selected_rect = n;
}


extern uint32 font_21[]; 

void appInit(void)
{
   Bitmap *fonts;
   fonts = malloc(sizeof(Bitmap) * 96);
   grDecompressFont(font_21,  fonts, 96);
   grSetFont (fonts, 32, 96);

   uiDefault();

   rects[0] = malloc(sizeof(*rects[0]));
   rects[0]->x0 = 100;
   rects[0]->y0 = 100;
   rects[0]->x1 = 500;
   rects[0]->y1 = 400;
   rects[0]->c = RGB(200,50,50);
   rects[0]->hot_alpha = 200;

   rects[1] = malloc(sizeof(*rects[1]));
   rects[1]->x0 = 300;
   rects[1]->y0 = 200;
   rects[1]->x1 = 600;
   rects[1]->y1 = 700;
   rects[1]->c = RGB(50,50,200);
   rects[1]->hot_alpha = 200;

   num_rects = 2;
}


