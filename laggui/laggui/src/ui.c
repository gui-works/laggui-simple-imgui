#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "platform.h"
#include "ui.h"
#include "gui_graph.h"

GUI_State ui;
GUI_Layout layout;

Bool uiButtonLogic(void *id, Bool over)
{
   Bool result = False;

   // note that this logic happens correctly for button down then
   // up in one frame -- but not up then down

   // process down
   if (!anyActive()) {
      if (over) uiSetHot(id);
      if (isHot(id) && ui.left_down)
         uiSetActive(id);
   }

   // if button is active, then react on left up
   if (isActive(id)) {
      ui.is_active = True;
      if (over) uiSetHot(id);
      if (ui.left_up) {
         if (isHot(id))
            result = True;
         uiClearActive();
      }
   }

   if (isHot(id)) ui.is_hot = True;

   return result;   
}

// same as above, but return True on the down event
Bool uiButtonLogicDown(void *id, Bool over)
{
   Bool result = False;

   // note that this logic happens correctly for button down then
   // up in one frame -- but not up then down

   // process down
   if (!anyActive()) {
      if (over) uiSetHot(id);
      if (isHot(id) && ui.left_down) {
         uiSetActive(id);
         result = True;
      }
   }

   // if button is active, then react on left up
   if (isActive(id)) {
      ui.is_active = True;
      if (over) uiSetHot(id);
      if (ui.left_up) {
         uiClearActive();
      }
   }

   if (isHot(id)) ui.is_hot = True;

   return result;   
}


static void rectOutlined(int x, int y, int w, int h, Color bg, Color fg)
{
   guiGraphSolidRectWH(x,y,w,h, bg);
   guiGraphRect(x,y,x+w,y+h, fg);
}

static Bool inRect(int x, int y, int w, int h)
{
   return (ui.mx >= x && ui.mx <= x+w && ui.my >= y && ui.my <= y+h);
}

static Bool inRectFudge(int x, int y, int w, int h, int s)
{
   return (ui.mx >= x-s && ui.mx <= x+w+s && ui.my >= y-s && ui.my <= y+h+s);
}


Bool uiButtonLogicRect(int x, int y, int w, int h, void *id)
{
   return uiButtonLogic(id, inRect(x,y,w,h));
}

GUI_Point uiLayout(int *w, int *h)
{
   GUI_Point p = { layout.cx, layout.cy };
   if (layout.force_w != WIDTH_auto) {
      *w = layout.force_w;
   }
   if (layout.force_w_once != WIDTH_auto) {
      *w = layout.force_w_once;
      layout.force_w_once = WIDTH_auto;
   }

   layout.cy += *h + layout.spacing_h;
   return p;
}

Bool uiButtonW(char *label, void *id, int w)
{
   // compute width, height, position
   int h=grCharHeight('g')  + layout.button_padding_h*2;
   GUI_Point p = uiLayout(&w,&h);
   int center_x;

   // draw it
   rectOutlined(p.x,p.y, w,h, isHot(id) ? layout.bg_lite : layout.bg, layout.bg_dark);

   center_x = p.x + w/2 - grTextWidth(label)/2;

   p.x += layout.button_padding_w;
   p.y += layout.button_padding_h;
   
   guiGraphText(center_x,p.y, label, layout.fg);
   
   // logic
   return uiButtonLogic(id, inRect(p.x,p.y, w,h));
}

Bool uiButton(char *label, void *id)
{
   return uiButtonW(label, id, grTextWidth(label) + layout.button_padding_w*2);
}

Bool uiButtonToggle(char *label1, char *label2, Bool *var)
{
   Bool res;
   int w1 = grTextWidth(label1);
   int w2 = grTextWidth(label2);
   res = uiButtonW(*var ? label2 : label1, var, max(w1,w2) + layout.button_padding_w*2);
   if (res)
      *var = !*var;
   return res;
}

Bool uiSliderRaw(char *label, float mn, float mx, float *val, void *id, Bool do_dots)
{
   int pos;
   int lw=0;

   int sw = layout.force_w;
   int sh = layout.slider_slot_h;

   int tw = layout.slider_tab_w;
   int th = layout.slider_tab_h;

   GUI_Point p;

   if (label) {
      int h = grCharHeight('g') - layout.spacing_h + 1;
      lw = grTextWidth(label)+4;
      p = uiLayout(&lw,&h);
      guiGraphText(p.x, p.y, label, layout.fg);
   }

   if (sw == WIDTH_auto) {
      if (label)
         sw = lw + 4;
      else
         sw = layout.slider_slot_default_w;
      if (sw < 50) sw = 50;
   }

   p = uiLayout(&sw, &th);

   // compute location of left edge of tab
   pos = p.x + (*val - mn) / (mx - mn) * sw - tw/2;

   rectOutlined(p.x,p.y+(th-sh)*3/4, sw,sh, isHot(id) ? layout.bg_lite : layout.bg, layout.bg_dark);

   if (do_dots) {
      int n = abs(mx - mn) + 1;
      // display dots only if there is a minimum spacing
      if (sw >= layout.slider_dot_spacing*n) {
         int i;
         for (i=0; i < n; ++i) {
            int pos = p.x + i * sw / (mx-mn);
            guiGraphPoint(pos, p.y+(th-sh)/4, 1, layout.bg_dark);
         }
      }
   }

   rectOutlined(pos,p.y, tw,th, layout.bg_lite, layout.bg_dark);

   // compute up/down state of scroller, ignore button-push effect
   uiButtonLogic(id, inRectFudge(p.x,p.y+(th-sh)/2,sw,sh+(th-sh)/4,2) || inRectFudge(pos,p.y,tw,th,1));

   if (isActive(id)) {
      // warp so center matches cursor
      float old_val = *val;
      float z = (ui.mx - p.x) * (mx - mn) / sw + mn;
      if (z < mn) z = mn; else if (z > mx) z = mx;
      *val = z;

      uiSetHot(id); // sliders are always hot while active
      return *val != old_val;
   }
   return False;
}

Bool uiSlider(char *label, float mn, float mx, float *val)
{
   return uiSliderRaw(label,mn,mx,val,val, False);
}

Bool uiSliderInt(char *label, int mn, int mx, int *val)
{
   int old_val = *val;
   float z = *val;
   if (uiSliderRaw(label, mn, mx, &z, val, True)) {
      *val = floor(z+0.5);
      return old_val != *val;
   }
   return False;
}

Bool uiSliderDisplay(char *label, float mn, float mx, float *val, Bool display)
{
   char buffer[256];
   if (display) {
      sprintf(buffer, "%s = %g", label, *val);
      label = buffer;
   }
   return uiSlider(label, mn, mx, val);
}

Bool uiSliderIntDisplay(char *label, int mn, int mx, int *val, Bool display)
{
   char buffer[256];
   if (display) {
      sprintf(buffer, "%s = %d", label, *val);
      label = buffer;
   }
   return uiSliderInt(label, mn, mx, val);
}



// a generic draggable rectangle... if you want its position clamped, do so yourself
Bool uiDragXY(int *x, int w,  int *y, int h , void *id)
{
   if (uiButtonLogicDown(id, inRect(*x-w/2,*y-h/2,w,h))) {
      ui.drag_x = *x - ui.mx;
      ui.drag_y = *y - ui.my;
   }

   if (isActive(id)) {
      if (ui.mx + ui.drag_x != *x || ui.my + ui.drag_y != *y) {
         *x = ui.mx + ui.drag_x;
         *y = ui.my + ui.drag_y;
         return True;
      }
   }
   return False;
}

Bool uiDragX (int *x, int w,  int y0, int y1, void *id)
{
   Bool res = False;
   // @TODO: copy above uiButtonDownLogic drag offseting code
   uiButtonLogic(id, inRect(*x-w/2,min(y0,y1),w,abs(y1-y0)));
   if (isActive(id)) {
      if (ui.mx != *x) { *x = ui.mx; res = True; }
   }
   return res;
}

Bool uiDragY (int x0, int x1, int *y, int h , void *id)
{
   Bool res = False;
   uiButtonLogic(id, inRect(min(x0,x1),*y-h/2,abs(x1-x0),h));
   if (isActive(id)) {
      if (ui.my != *y) { *y = ui.my; res = True; }
   }
   return res;
}



static Color average(Color c, Color d)
{
   c = (c >> 1) & 0x7f7f7f7f;
   d = (d >> 1) & 0x7f7f7f7f;
   return c + d + (c & d & 0x01010101);
}

static Color dark(Color c)
{
   return average(c, RGB_GREY(0));
}

static Color lite(Color c)
{
   return average(c,~0);
}

void uiBgcolor(Color c)
{
   layout.bg = c;
   layout.bg_dark = dark(c);
   layout.bg_lite = lite(c);
}

void uiColor(Color c)
{
   layout.fg = c;
}

void uiPos(int x, int y)
{
   layout.cx = x;
   layout.cy = y;
}

void uiWidth(int w)
{
   layout.force_w = w;
}

void uiWidth1(int w)
{
   layout.force_w_once = w;
}

void uiDefault(void)
{
   uiColor(RGB_GREY(0));
   uiBgcolor(RGB_GREY(220));

   layout.spacing_w = 20;
   layout.spacing_h =  4;

   uiWidth (WIDTH_auto);
   uiWidth1(WIDTH_auto);

   layout.button_padding_h = 4;
   layout.button_padding_w = 4;

   layout.slider_slot_h = 4;
   layout.slider_tab_w = 8;
   layout.slider_tab_h = 16;
   layout.slider_slot_default_w = 100;

   layout.slider_dot_spacing = 12;

   uiPos(0,0);
}

void uiClear(void)
{
   ui.left_down = ui.left_up = False;
   ui.right_down = ui.right_up = False;
   ui.middle_down = ui.middle_up = False;
}

void uiClearActive(void)
{
   ui.active.id = NULL;
   ui.active.parent = NULL;
   ui.active.index = 0;

   // mark all UI for this frame as processed
   uiClear();

   platformCapture(False);
}

void uiSetActive(void *id)
{
   ui.active.id = id;
   ui.active.parent = ui.cur_parent;
   ui.active.index = ui.cur_index;

   ui.went_active = True;

   platformCapture(True);
}

void uiSetHot(void *id)
{
   ui.hot_to_be.id = id;
   ui.hot_to_be.index = ui.cur_index;
   ui.hot_to_be.parent = ui.cur_parent;
}

void uiFrameBegin(void)
{
   // at start of frame, clear old state and compute
   // relative-to-previous-frame state (like mouse deltas)

   // what's hot this frame is whatever was last in hot-to-be in last frame
   // (painter's algorithm--last-most widget is on top)
   ui.hot = ui.hot_to_be;

   ui.hot_to_be.id = ui.hot_to_be.parent = NULL;
   ui.hot_to_be.index = 0;

   if (ui.prev_valid) {
      ui.delta_mx = ui.mx - ui.prev_mx;
      ui.delta_my = ui.my - ui.prev_my;
   } else {
      ui.delta_mx = 0;
      ui.delta_my = 0;
   }

   ui.went_active = False;
   ui.is_active   = False;
   ui.is_hot      = False;

   // pad edges by default spacing
   layout.cx = layout.spacing_w;
   layout.cy = layout.spacing_h;
}

void uiFrameEnd(void)
{
   ui.prev_mx = ui.mx;
   ui.prev_my = ui.my;
   ui.prev_valid = True;
   
   // if we don't clear it, we can better handle button up/down in single frame
   // but then anything which doesn't get handled will break... we need to copy
   // button state, and if it's unchanged, clear, I think
   uiClear();
}

