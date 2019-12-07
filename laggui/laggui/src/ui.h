#include "platform.h"
#include "gr.h"


// client usage

extern void uiDefault(void);

extern Bool uiButton(char *label, void *id);
extern Bool uiButtonToggle(char *label1, char *label2, Bool *var);

extern Bool uiSlider(char *label, float mn, float mx, float *val);
extern Bool uiSliderInt(char *label, int mn, int mx, int *val);

// like above, but also displays the value
extern Bool uiSliderDisplay(char *label, float mn, float mx, float *val, Bool display);
extern Bool uiSliderIntDisplay(char *label, int mn, int mx, int *val, Bool display);

extern Bool uiDragXY(int *x, int w,  int *y, int h , void *id);
extern Bool uiDragX (int *x, int w,  int y0, int y1, void *id);
extern Bool uiDragY (int x0, int x1, int *y, int h , void *id);

extern void uiBgcolor(Color c);
extern void uiColor(Color c);
extern void uiPos(int x, int y);

#define   WIDTH_auto   0
extern void uiWidth1(int w);
extern void uiWidth(int w);

extern void uiFrameBegin(void);
extern void uiFrameEnd(void);

// client convenient tools for making variant ids from a single id

#define ID2(x)   (1+(char *)(x))
#define ID3(x)   (2+(char *)(x))
#define ID4(x)   (3+(char *)(x))

// widget implementation:

extern void uiClearActive(void);
extern void uiSetActive(void *id);
extern void uiSetHot(void *id);

extern Bool uiButtonLogic(void *id, Bool over);
extern Bool uiButtonLogicRect(int x, int y, int w, int h, void *id);

#define compareID(i,ind,par,dest)  ((i) == (dest).id && (ind)==(dest).index && (par) == (dest).parent)
#define anyActive()    (ui.active.id != NULL)
#define isActive(id)   compareID(ui.cur_id ? ui.cur_id : id,ui.cur_index,ui.cur_parent, ui.active)
#define isHot(id)      compareID(ui.cur_id ? ui.cur_id : id,ui.cur_index,ui.cur_parent, ui.hot)



typedef struct
{
   int16 x,y;
} GUI_Point;

typedef struct
{
   void *id;
   int   index;
   void *parent;
} GUI_WidgetID;

typedef struct
{
   Bool left_up, left_down;
   Bool right_up, right_down;
   Bool middle_up, middle_down;

   int mx,my;
   int delta_mx, delta_my;

   GUI_WidgetID active ;
   GUI_WidgetID hot    ;
   GUI_WidgetID hot_to_be;

   void *cur_parent;
   int   cur_index;
   void *cur_id;

   // this is set by whichever widget is hot/active; you can watch
   // for it to check
   Bool is_hot;
   Bool is_active;

   // true on the first frame a widget becomes active, useful for
   // layering widget implementations
   Bool went_active;

   // INTERNAL USE ONLY
   Bool prev_valid;
   int prev_mx, prev_my;

   int drag_x, drag_y; // drag offsets
} GUI_State;

typedef struct
{
   Color fg;
   Color bg, bg_dark, bg_lite;

   int spacing_w;
   int spacing_h;

   int force_w;
   int force_w_once;

   int button_padding_w;
   int button_padding_h;

   int slider_tab_w;
   int slider_tab_h;
   int slider_slot_h;
   int slider_slot_default_w; // only used if width can't be computed otherwise

   int slider_dot_spacing;

   int cx, cy;
} GUI_Layout;

extern GUI_State ui;
extern GUI_Layout layout;

