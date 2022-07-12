#include "led.h"
#include "variables.h"

uint8_t led_sources[MAX_LED_SRC_NUM];
int fps_counter;

LED PRU[8];

/**
 * @brief wrapper for init of all LED PRU objects
 * 
 */
void pru_init()
{
    for(int i = 1; i<2; i++)
    {
        PRU[i].init(i);
    }
}

/**
 * @brief Construct a new LED::LED object
 * 
 */
LED::LED()
{
    _pruId = 0xFF;
}

/**
 * @brief Initialise the PRU present on the specific ID
 * 
 * @param pruId 
 * @return int 
 */
int LED::init(uint8_t pruId)
{
    _pruId = pruId;
    esp_err_t ret = i2c_check_device(PRU_I2C_ADDR);
    return ret;
    
    // compatibility exchange with PRU
    
}

/**
 * @brief Get a specific attribute in the PRU
 * 
 * @param channel 
 * @param opcode 
 * @param readBuff 
 */
void LED::pruGet(uint8_t channel, uint8_t opcode, uint8_t *readBuff)
{
    uint8_t writeBuff[PRU_PACKET_SIZE] = {0};
    writeBuff[0] = (_pruId << 4)| channel;
    writeBuff[1] = opcode;
    writeBuff[2] = '0';
    writeBuff[3] = '0';
    while(i2c_master_write_to_device(I2C_MASTER_NUM, PRU_I2C_ADDR, writeBuff, PRU_PACKET_SIZE, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) != ESP_OK);
    while(i2c_master_read_from_device(I2C_MASTER_NUM, PRU_I2C_ADDR, readBuff, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT);
}

/**
 * @brief Set a specific attribute in the PRU
 * 
 * @param channel 
 * @param opcode 
 * @param readBuff 
 */
void LED::pruSet(uint8_t channel, uint8_t opcode, uint8_t *readBuff)
{
    uint8_t writeBuff[PRU_PACKET_SIZE] = {0};
    writeBuff[0] = (_pruId << 4) | channel;
    writeBuff[1] = opcode;
    writeBuff[2] = '0';
    writeBuff[3] = '0';
    while(i2c_master_write_to_device(I2C_MASTER_NUM, PRU_I2C_ADDR, writeBuff, PRU_PACKET_SIZE, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) != ESP_OK);
    while(i2c_master_read_from_device(I2C_MASTER_NUM, PRU_I2C_ADDR, readBuff, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT);
}