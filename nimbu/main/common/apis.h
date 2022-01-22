#ifndef __APIS__
#define __APIS__

#include <cstdint>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

uint64_t millis();
void delay(uint64_t msDelay);

#endif // __APIS__