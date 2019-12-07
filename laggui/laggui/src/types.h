#ifndef INC_TYPES_H
#define INC_TYPES_H

#ifdef WIN32
#pragma warning(disable: 4244; disable: 4305; error:4087)
#endif

typedef int Bool;

#define True  1        // trying a new case convention just to see
#define False 0        // how it looks, and to avoid conflicts

typedef unsigned int   uint32;
typedef          int    int32;
typedef unsigned short uint16;
typedef          short  int16;
typedef unsigned char  uint8;
typedef   signed char   int8;

typedef unsigned int   uint ;

enum
{
   E_mousemove,
   E_leftdown,
   E_middledown,
   E_rightdown,
   E_leftup,
   E_middleup,
   E_rightup,
   E_keydown,

   E_paint,
   E_hit_test,
   E_query_size,
};

#endif