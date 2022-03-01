#ifndef __DEFINITIONS__
#define __DEFINITIONS__

#include <time.h>

#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef constrain
#define constrain(a, minval, maxval)    min((max(a,minval)),maxval)
#endif

#ifndef wrapIncrement
#define wrapIncrement(a, maxval)    ((a)==(maxval)?(0):(a+1))
#endif

typedef char byte;
typedef tm tmElements_t;

// USER CONFIG

// #define DEBUG__FPS
// #define DEBUG__PRINT
// #define DEBUG__ADC
// #define DEBUG__BT

#define LED_VU_RESOLUTION (2)

#endif // __DEFINITIONS__