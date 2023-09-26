#include <FastLED_NeoMatrix.h>
#ifndef MY_GLOBALS_H
#define MY_GLOBALS_H

// This is a declaration of your variable, which tells the linker this value
// is found elsewhere.  Anyone who wishes to use it must include global.h,
// either directly or indirectly.
extern void renderArcanoid(void);
extern void renderSnakeGame(void);
extern FastLED_NeoMatrix *matrix;

#endif