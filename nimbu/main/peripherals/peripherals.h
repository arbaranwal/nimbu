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
#include "driver/i2c.h"
#include "esp_adc_cal.h"
#include "definitions.h"

//////////////////
/// ADC CONFIG ///
//////////////////

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

#define ADC_SRC1    ADC1_CHANNEL_6
#define ADC_SRC2    ADC1_CHANNEL_7

bool adc_init(void);
void adc_task_start_up();
void adc_update(void *arg);


//////////////////
/// I2S CONFIG ///
//////////////////

#define I2S_TAG "I2S"
#define I2S_SAMPLE_RATE             (44100)
#define I2S_BITS_PER_SAMPLE         (16)
#define I2S_DMA_DESC_NUM            (64)
#define I2S_DMA_FRAME_NUM           (64)

void i2s_init();


//////////////////
/// I2C CONFIG ///
//////////////////

#define I2C_TAG "I2C"
#define I2C_MASTER_SCL_IO           (22)      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           (21)      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              (0)       /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          (400000)  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   (0)       /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   (0)       /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       (1000)
void i2c_master_init();
int i2c_check_device(uint8_t addr);
int probePRUs(uint8_t addr);

#endif // __PERIPHERALS__