#include "apis.h"

uint64_t millis()
{
    return ((uint64_t)(xTaskGetTickCount())*1000)/configTICK_RATE_HZ;
}

void delay(uint64_t msDelay)
{
    vTaskDelay(msDelay/portTICK_PERIOD_MS);
}