#ifndef definitions_h
#define definitions_h

#include <avr/io.h>
#include <avr/interrupt.h>

// #define DEBUG_PRINT
// #define DEBUG_FPS
// #define DEBUG_ADC
// #define DEBUG_WAVE
#define DEBUG_COMMAND

#define ADC_INTERRUPT_MODE
// #define ADAPTIVE_REFERENCE
// #define FLASH_GATE

#define IND             (13)
#define INDON           digitalWrite(IND, HIGH);
#define INDOFF          digitalWrite(IND, LOW);
#define CHANNEL0        (0)
#define CHANNEL1        (1)
#define CHANNEL2        (2)
#define CHANNEL3        (3)
#define ANALOGREAD      analogRead8bit
#define GETBASS         ANALOGREAD(CHANNEL0)
#define GETKICK         ANALOGREAD(CHANNEL0)>>2 ? ANALOGREAD(CHANNEL0) : 0
#define GETTREBLE       ANALOGREAD(CHANNEL2)
#define GETMID          ANALOGREAD(CHANNEL1)
#define GETEXTLIGHT     ANALOGREAD(CHANNEL3)
#define GETRANDOM(x,y)  (uint8_t)random(x,y)
#define INCWRAP(x,y)    ((x > y-2) ? 0:(x+1))
#define INCSWRAP(x,y,z) ((x > y-2) ? 0:(x+z))

enum positions {FLASH, PULSE, USER, RANDOM, BASS, MID, TREBLE, LIGHT, TOTAL};
//enum positions {LIGHT, TREBLE, MID, BASS, USER, PULSE, FLASH, TOTAL};

#endif
