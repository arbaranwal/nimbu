#ifndef __LED_DEFINITIONS__
#define __LED_DEFINITIONS__

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY          (78125) // 40 MHz / 512 levels = 78125 Hz. Only 256 levels will be used

#define R_LEDC_TIMER            LEDC_TIMER_0
#define R_LEDC_MODE             LEDC_LOW_SPEED_MODE
#define R_LEDC_OUTPUT_IO        (17)
#define R_LEDC_CHANNEL          LEDC_CHANNEL_0
#define R_LEDC_DUTY_RES         LEDC_TIMER_8_BIT
#define R_LEDC_FREQUENCY        (LEDC_FREQUENCY)

#define G_LEDC_TIMER            LEDC_TIMER_0
#define G_LEDC_MODE             LEDC_LOW_SPEED_MODE
#define G_LEDC_OUTPUT_IO        (16)
#define G_LEDC_CHANNEL          LEDC_CHANNEL_1
#define G_LEDC_DUTY_RES         LEDC_TIMER_8_BIT
#define G_LEDC_FREQUENCY        (LEDC_FREQUENCY)

#define B_LEDC_TIMER            LEDC_TIMER_0
#define B_LEDC_MODE             LEDC_LOW_SPEED_MODE
#define B_LEDC_OUTPUT_IO        (4)
#define B_LEDC_CHANNEL          LEDC_CHANNEL_2
#define B_LEDC_DUTY_RES         LEDC_TIMER_8_BIT
#define B_LEDC_FREQUENCY        (LEDC_FREQUENCY)

#define FPS 250
#define LED_TAG "LEDC"

// #define ADAPTIVE_REFERENCE
// #define FLASH_GATE
#ifndef LED_VU_RESOLUTION
#define LED_VU_RESOLUTION       (2)
#endif

#define GETADC_SOURCE1      (GET_SOURCE(SRC_1))
#define GETKICK             (GET_SOURCE(SRC_1)>>2 ? GET_SOURCE(SRC_1) : 0)
#define GETADC_SOURCE3      (GET_SOURCE(SRC_3))
#define GETADC_SOURCE2      (GET_SOURCE(SRC_2) - max(GETADC_SOURCE1, GETADC_SOURCE3))
#define GETEXTLIGHT         (GET_SOURCE(0))
#define GETRANDOM(x,y)      (uint8_t)((random()%(y-x+1))+x)

enum positions {TOTAL, FLASH, PULSE, USER, RANDOM, ADC_SOURCE1, ADC_SOURCE2, ADC_SOURCE3, LIGHT};
enum sources {SRC_1, SRC_2, SRC_3, MAX_ADC_SRC_NUM};

#endif // __LED_DEFINITIONS__