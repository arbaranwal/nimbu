#ifndef __PERIPHERALS__
#define __PERIPHERALS__

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include "definitions.h"

/*
 * Configure ADC input channels
 * Some of the ADC2 pins are used as strapping pins (GPIO 0, 2, 15).
 * ESP32 DevKitC   : GPIO 0 cannot be used due to external auto program circuits.
 * ESP-WROVER-KIT  : GPIO 0, 2, 4 and 15 cannot be used due to external connections.
 * ADC2 module is also used by the Wi-Fi, only one of them could get the preemption when using together.
 * So the adc2_get_raw() may get blocked until Wi-Fi stops, and vice versa.
 */

/* 
 * ADC1:
 * GPIO     : 36  --  --  39  32  33  34  35
 * CHANNEL  :  0   1   2   3   4   5   6   7
 * 
 * ADC2:
 * GPIO     :  0   2   4  12  13  14  15  25  26  27
 * CHANNEL  :  0   1   2   3   4   5   6   7   8   9
 */

/*
 * Set ADC Attenuation
 * ADC_ATTEN_DB_0      100 mV ~ 950 mV
 * ADC_ATTEN_DB_2_5    100 mV ~ 1250 mV
 * ADC_ATTEN_DB_6      150 mV ~ 1750 mV
 * ADC_ATTEN_DB_11     150 mV ~ 2450 mV
 */
#define ADC_ATTEN           ADC_ATTEN_DB_11
#define ADC_POLL_DELAY_MS   (100/MAX_ADC_SRC_NUM)

//ADC Calibration
#if CONFIG_IDF_TARGET_ESP32
#define ADC_CAL_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_CAL_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32C3
#define ADC_CAL_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_CAL_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif

#define ADC_TAG "ADC"
#ifdef DEBUG_ADC
#define ADC_ESP_LOGI                ESP_LOGI
#else
#define ADC_ESP_LOGI(ADC_TAG, ...)  void(0)
#endif

// ADC
bool adc_init(void);
void adc_task_start_up();
void adc_update(void *arg);

// I2S
void i2s_init();

#endif // __PERIPHERALS__