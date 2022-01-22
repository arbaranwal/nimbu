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

typedef char byte;

#endif // __DEFINITIONS__