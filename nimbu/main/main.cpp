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

#include "bt/bt_main.h"
#include "definitions.h"
#include "apis.h"
#include "led_definitions.h"
#include "led.h"
#include "variables.h"
#define BLINK_GPIO 2

// extern variables defined in variables.h
int fps_counter;

// needed for creating entry point to C
extern "C" {
	void app_main(void);
}

void bsLEDConfig()
{
    LED1.invert(true);
    LED2.invert(true);
    LED3.invert(true);
}

void bootstrap()
{
    fps_counter=0;
    bsLEDConfig();
}

void app_main(void)
{
    bootstrap();
    // Set the LEDC peripheral configuration
    for(uint8_t i = 0; i < 255; i++)
    {
        LED1.update(i);
        LED2.update(i);
        LED3.update(i);
        delay(10);
    }
    printf("Starting Up Nimbu...\n");
    printf("esp_get_free_heap_size: %f KiB\n", (float)esp_get_free_heap_size()/1024);
    printf("esp_get_free_internal_heap_size: %f KiB\n", (float)esp_get_free_internal_heap_size()/1024);
    printf("esp_get_minimum_free_heap_size: %f KiB\n", (float)esp_get_minimum_free_heap_size()/1024);

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    i2s_config_t i2s_config = {};
#ifdef CONFIG_EXAMPLE_A2DP_SINK_OUTPUT_INTERNAL_DAC
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
#else
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);  // Only TX
#endif
    i2s_config.sample_rate = 44100;
    i2s_config.bits_per_sample = (i2s_bits_per_sample_t)16;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;         //2-channels
    i2s_config.communication_format = I2S_COMM_FORMAT_STAND_MSB;
    i2s_config.dma_desc_num = 128;
    i2s_config.dma_frame_num = 64;
    i2s_config.intr_alloc_flags = 0;                                //Default interrupt priority
    i2s_config.tx_desc_auto_clear = true;                           //Auto clear tx descriptor on underflow

    i2s_driver_install((i2s_port_t)0, &i2s_config, 0, NULL);
#ifdef CONFIG_EXAMPLE_A2DP_SINK_OUTPUT_INTERNAL_DAC
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    i2s_set_pin(0, NULL);
#else
    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_EXAMPLE_I2S_BCK_PIN,
        .ws_io_num = CONFIG_EXAMPLE_I2S_LRCK_PIN,
        .data_out_num = CONFIG_EXAMPLE_I2S_DATA_PIN,
        .data_in_num = -1                                                       //Not used
    };

    i2s_set_pin((i2s_port_t)0, &pin_config);
#endif


    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    /* create application task */
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