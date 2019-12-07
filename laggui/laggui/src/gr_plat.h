// must be visible to both platform and gr system
// @TODO this can go away when we switch to inline fonts
typedef struct
{
   int w,h;
   uint32 *data;
} FontChar;

extern void platformComputeFont(int pixel_height, int monospaced_flag, int start, int num, FontChar *chars);
