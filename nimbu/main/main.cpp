#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/ledc.h"
#include "esp_err.h"

#include "esp_bt.h"
#include "bt/bt_app_core.h"
#include "bt/bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "driver/i2s.h"
#include "driver/gpio.h"

#include "bt_main.h"
#include "definitions.h"
#include "apis.h"
#include "led_definitions.h"
#include "led.h"
#include "variables.h"
#include "fft_engine.h"
#include "peripherals.h"
#define BLINK_GPIO 2

// extern variables defined in variables.h
xTaskHandle s_led_task_handle = NULL;
xTaskHandle s_fft_task_handle = NULL;
xTaskHandle s_adc_task_handle = NULL;

// needed for creating entry point to C
extern "C" {
	void app_main(void);
}

void app_init()
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Initialise LED Controller */
    led_init();
    for(uint8_t i = 0; i < 255; i++)
    {
        LED1.update(i);
        LED2.update(i);
        LED3.update(i);
        delay(10);
    }

    /* initialise peripherals */
    adc_init();
    i2s_init();

    /* initialise Bluetooth */
    bt_init();
}

void app_main(void)
{
    ESP_LOGI("MAIN", "Starting Up Nimbu...");
    ESP_LOGI("MAIN", "esp_get_free_heap_size: %f KiB", (float)esp_get_free_heap_size()/1024);
    ESP_LOGI("MAIN", "esp_get_free_internal_heap_size: %f KiB", (float)esp_get_free_internal_heap_size()/1024);
    ESP_LOGI("MAIN", "esp_get_minimum_free_heap_size: %f KiB", (float)esp_get_minimum_free_heap_size()/1024);

    app_init();

    /* create application tasks */
    adc_task_start_up();
    led_task_start_up();
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

    #if (CONFIG_BT_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
    #endif

    /*
     * Set default parameters for Legacy Pairing
     * Use fixed pin code
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '4';
    pin_code[1] = '3';
    pin_code[2] = '2';
    pin_code[3] = '1';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);
    // gpio_set_level(BLINK_GPIO, 0);

}