#ifndef __LED__
#define __LED__

#include <cmath>
#include <stdlib.h>
#include "definitions.h"
#include "peripherals.h"
#include "apis.h"
#include "led_definitions.h"

#define PRU_I2C_ADDR    (0x8)
#define PRU_PACKET_SIZE (0x8)

void pru_init();
void led_task_start_up();
void led_update(void *arg);

class LED
{
private:
    uint8_t _pruId, _brightness[9] = {0,0,0,0,0,0,0,0,0}, _activeSources = 0, _minBrightness = 0, _maxBrightness = 255, _minPulseBrightness = 0, _maxPulseBrightness = 255, _colourDepth = 0, _stepValue = 1;
    uint8_t _randomStep = 1, _randomJitter = 0;
    uint8_t adcSource1IncrCount = 0, adcSource1IncrCountThresh = 1, adcSource1DecrCount = 0, adcSource1DecrCountThresh = 1;
    uint8_t adcSource2IncrCount = 0, adcSource2IncrCountThresh = 1, adcSource2DecrCount = 0, adcSource2DecrCountThresh = 1;
    uint16_t _incrementStepTime = 0, _decrementStepTime = 0;
    uint16_t _onTime = 0, _offTime = 0;
    unsigned long _prevUpdate = 0;
    bool _state=0, _invert=0;
    byte _control = 0b00000000;
    uint8_t GET_SOURCE(uint8_t src);

public:
    LED();
    int init(uint8_t pruId);
    void update(uint8_t brightness);
    void update();
    void pruGet(uint8_t channel, uint8_t opcode, uint8_t *readBuff);
    void pruSet(uint8_t channel, uint8_t opcode, uint8_t *readBuff);
    void setUserBrightness(uint8_t brightness);
    void setBrightness(uint8_t brightness);
    void setColourDepth(uint8_t colourDepth);
    uint8_t getColourDepth();
    uint8_t getBrightness();
    uint8_t getActiveSources();
    byte getControlParameters();
    bool setControlParameters(byte control);
    void setRandomStep(uint8_t randomStep, uint8_t randomJitter);
    void setTime(uint16_t onTime, uint16_t offTime);
    void setTime(uint16_t Time);
    void invert(bool invertColour);
    void limit(uint8_t brightness);
    void limit(uint8_t minBrightness, uint8_t maxBrightness);
    void limitPulseBrightness(uint8_t minPulseBrightness, uint8_t maxPulseBrightness);
    void flash(bool flash);
    void pulse(bool pulse);
    void routeRandom(bool argRandom);
    void routeUser(bool user);
    void routeAdcSource1(bool bass);
    void routeAdcSource1(bool bass, uint8_t argIncrThresh, uint8_t argDecrThresh);
    void routeAdcSource2(bool mid);
    void routeAdcSource2(bool mid, uint8_t argIncrThresh, uint8_t argDecrThresh);
    void routeAdcSource3(bool treble);
    void watchExtLight(bool extlight, uint8_t brightness);
    void watchExtLight(bool extlight);
    uint8_t calculateBrightness();
};

#endif // __LED__