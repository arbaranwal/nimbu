#ifndef __VARIABLES__
#define __VARIABLES__

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "definitions.h"
#include "led_definitions.h"
#include "led.h"

#define MAX_FFT_SRC_NUM (8)
#define MAX_LED_SRC_NUM (MAX_ADC_SRC_NUM + MAX_FFT_SRC_NUM)

extern xQueueHandle eventQueue;

// led: defined in led.cpp
extern int fps_counter;
extern LED LED1, LED2, LED3;

// led source-sink bridge:
// defined in peripherals.cpp
extern uint16_t adc_values[MAX_ADC_SRC_NUM];
// defined in led.cpp
extern uint8_t led_sources[MAX_LED_SRC_NUM];
// defined in fft_engine.cpp
extern uint8_t fft_values[MAX_FFT_SRC_NUM];

// fft-blt bridge: defined in either bt_app_core.cpp or fft_engine.cpp
extern bool freezeBLTData, readyForFFT;
extern uint8_t bltData[64];
extern uint8_t bltDataIndex;

// tasks: defined in main.cpp
extern xTaskHandle s_led_task_handle;
extern xTaskHandle s_fft_task_handle;
extern xTaskHandle s_adc_task_handle;

#endif // __VARIABLES__