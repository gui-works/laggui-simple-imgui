// color interface used by both gr.h and ui_graph.h

typedef uint32 Color;

// define two functions RGB() and RGBA()
// these return a 'Color' from an r,g,b or r,g,b,a set
// (values ranging from 0..255)

// this happens to be native WIN32 layout, but
// it's important that A be the top 8 bits of
// the uint32 for fast testing, which I think is
// normal in other platforms

#define RGB(r,g,b)       RGBA(r,g,b,255)
#define RGBA(r,g,b,a)    PACK_UINT32(a,r,g,b)

#define PACK_UINT32(a,b,c,d)   (((uint8) (a) << 24) + ((uint8) (b) << 16) + ((uint8) (c) << 8) + (uint8) (d))

#define B(c)           ((uint8)  (c)       )
#define G(c)           ((uint8) ((c) >>  8))
#define R(c)           ((uint8) ((c) >> 16))
#define A(c)           ((uint8) ((c) >> 24))

#define RGB_GREY(x)      RGB(x,x,x)