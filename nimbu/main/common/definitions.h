#ifndef __DEFINITIONS__
#define __DEFINITIONS__

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

#define MAX_ADC_SRC_NUM (2)
#define MAX_FFT_SRC_NUM (8)
#define MAX_LED_SRC_NUM (MAX_ADC_SRC_NUM + MAX_FFT_SRC_NUM)

// #define DEBUG_FPS
// #define DEBUG_PRINT
// #define DEBUG_ADC
// #define DEBUG_WAVE

typedef char byte;

#endif // __DEFINITIONS__