#include "peripherals.h"
#include "variables.h"

uint16_t adc_values[MAX_ADC_SRC_NUM];
adc1_channel_t adc_channels[MAX_ADC_SRC_NUM] = {ADC_SRC1, ADC_SRC2};

void adc_update(void *arg)
{
    uint8_t adc_src = 0;
    for(;;)
    {
        // request ADC value
        adc_values[adc_src] = adc1_get_raw(adc_channels[adc_src]);
        // increment channel number
        adc_src = wrapIncrement(adc_src, MAX_ADC_SRC_NUM-1);
        ADC_ESP_LOGI(ADC_TAG, "ADC Data: %d", adc_values[0]);
        // wait for next reading
        delay(ADC_POLL_DELAY_MS);
    }
}

bool adc_init(void)
{
    esp_err_t ret;
    bool cal_enable = false;
    esp_adc_cal_characteristics_t adc1_chars;

    ret = esp_adc_cal_check_efuse(ADC_CAL_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        ESP_LOGW(ADC_TAG, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        ESP_LOGW(ADC_TAG, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        cal_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, (adc_bits_width_t)ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    }
    else
    {
        ESP_LOGE(ADC_TAG, "Invalid arg");
    }

    ESP_ERROR_CHECK(adc1_config_width((adc_bits_width_t)ADC_WIDTH_BIT_DEFAULT));
    for(uint8_t i = 0; i < MAX_ADC_SRC_NUM; i++)
    {
        ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channels[i], ADC_ATTEN));
    }
    return cal_enable;
}

void adc_task_start_up()
{
    ESP_LOGI(ADC_TAG, "Poll Delay: %d ms", ADC_POLL_DELAY_MS);
    xTaskCreate((TaskFunction_t)adc_update, "ADC_upd", 3072, NULL, tskIDLE_PRIORITY, &s_led_task_handle);
}

void i2s_init()
{
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
    i2s_config.dma_desc_num = 64;
    i2s_config.dma_frame_num = 64;
    i2s_config.intr_alloc_flags = 0;        //Default interrupt priority
    i2s_config.tx_desc_auto_clear = true;   //Auto clear tx descriptor on underflow

    i2s_driver_install((i2s_port_t)0, &i2s_config, 0, NULL);
    #ifdef CONFIG_EXAMPLE_A2DP_SINK_OUTPUT_INTERNAL_DAC
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    i2s_set_pin(0, NULL);
    #else
    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_EXAMPLE_I2S_BCK_PIN,
        .ws_io_num = CONFIG_EXAMPLE_I2S_LRCK_PIN,
        .data_out_num = CONFIG_EXAMPLE_I2S_DATA_PIN,
        .data_in_num = -1   //Not used
    };
    i2s_set_pin((i2s_port_t)0, &pin_config);
    #endif
}

void i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
    };
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    ESP_LOGI(I2C_TAG, "Scanning for I2C peripherals...");
    for (uint8_t i = 1; i < 127; i++)
    {
        int ret;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK)
        {
            ESP_LOGI(I2C_TAG, "Found device at: 0x%02x", i);
        }
    }
}

int i2c_check_device(uint8_t addr)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}