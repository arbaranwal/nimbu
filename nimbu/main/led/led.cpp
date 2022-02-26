#include "led.h"
#include "variables.h"

uint8_t led_sources[MAX_LED_SRC_NUM];
int fps_counter;

LED LED1(R_LEDC_OUTPUT_IO, R_LEDC_TIMER, R_LEDC_CHANNEL);
LED LED2(G_LEDC_OUTPUT_IO, G_LEDC_TIMER, G_LEDC_CHANNEL);
LED LED3(B_LEDC_OUTPUT_IO, B_LEDC_TIMER, B_LEDC_CHANNEL);

void led_init()
{
    fps_counter=0;
    // LED1.invert(true);
    LED1.update(0xff);
    LED1.setTime(6000,6000);
    // LED1.setTime(20,500);
    // LED1.routeAdcSource1(true);
    LED1.routeAdcSource2(true);
    // LED1.pulse(true);
    // LED1.limit(1);
    // LED1.flash(true);

    // LED2.invert(true);
    LED2.update(0xff);
    LED2.setTime(3000,3000);
    // LED2.pulse(true);
    LED2.routeAdcSource2(true);
    // LED2.limit(1);
    // LED2.setTime(20,1000);
    // LED2.setUserBrightness(16);
    // LED2.routeUser(true);
    
    // LED3.invert(true);
    LED3.update(0xff);
    LED3.setTime(2000,2000);
    // LED3.pulse(true);
    LED3.routeAdcSource2(true);
    // LED3.limit(1);
    // LED3.setTime(2000,2000);
    // LED3.setUserBrightness(96);
    // LED3.setRandomStep(64,2);
    // LED3.routeUser(true);
    // LED3.routeRandom(true);
}

void led_update(void *arg)
{
    // Required for vTaskDelayUntil to achieve precise FPS
    #ifdef DEBUG_FPS
    uint64_t prevTime = millis();
    #endif
    // TickType_t xLastLEDFrameUpdate;
    // portTICK_PERIOD_MS milliseconds --> 1 tick
    // time(ms) --> time/portTICK_PERIOD_MS
    // fps = 1000/time(ms)
    // const TickType_t xFrequency = 1000/(FPS*portTICK_PERIOD_MS);
    for (;;) {
        LED1.update();
        LED2.update();
        LED3.update();
        // printf("%d\n", adc_values[0]+1000);
        #ifdef DEBUG_FPS
        fps_counter++;
        if(millis() - prevTime > 1000)
        {
            ESP_LOGI("LED", "FPS: %3d", fps_counter);
            fps_counter = 0;
            prevTime = millis();
        }
        #endif
        delay(1000/FPS);
        // vTaskDelayUntil(&xLastLEDFrameUpdate, xFrequency);
        // this implementation of vTaskDelayUntil is not working for some reason
    }
}

void led_task_start_up()
{
    xTaskCreate((TaskFunction_t)led_update, "LED_upd", 4096, NULL, tskIDLE_PRIORITY, &s_led_task_handle);
}

LED::LED(uint8_t gpio_pin, ledc_timer_t ledc_timer_num, ledc_channel_t ledc_channel_num)
{
    _ledc_timer.speed_mode      = LEDC_MODE;
    _ledc_timer.timer_num       = ledc_timer_num;
    _ledc_timer.duty_resolution = LEDC_DUTY_RES;
    _ledc_timer.freq_hz         = LEDC_FREQUENCY;  // Set output frequency at 5 kHz
    _ledc_timer.clk_cfg         = LEDC_AUTO_CLK;

    _ledc_channel.speed_mode     = LEDC_MODE;
    _ledc_channel.channel        = ledc_channel_num;
    _ledc_channel.timer_sel      = ledc_timer_num;
    _ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    _ledc_channel.gpio_num       = 0;
    _ledc_channel.duty           = 0; // Set duty to 0%
    _ledc_channel.hpoint         = 0;
    _ledc_channel.gpio_num       = gpio_pin;

    _gpio_pin = gpio_pin;
    _prevUpdate = millis();
    // Apply the LEDC PWM timer configuration
    ESP_ERROR_CHECK(ledc_timer_config(&_ledc_timer));
    // Apply the LEDC PWM channel configuration
    ESP_ERROR_CHECK(ledc_channel_config(&_ledc_channel));
}

uint8_t LED::GET_SOURCE(uint8_t src)
{
    return led_sources[src];
}

void LED::update(uint8_t value)
{
    ESP_ERROR_CHECK(ledc_set_duty(_ledc_channel.speed_mode, _ledc_channel.channel, value));
    ESP_ERROR_CHECK(ledc_update_duty(_ledc_channel.speed_mode, _ledc_channel.channel));
    // printf("%d %p %lld %d %d\n", _brightness[TOTAL], &_brightness[TOTAL], millis(), _onTime, _offTime);
}

void LED::update()
{
    // simple update : refresh total-brightness
    _brightness[TOTAL] = calculateBrightness();
    if(_invert)
        LED::update((0xFF)^(_brightness[TOTAL])); // 255 - _brightness[TOTAL])
    else
        LED::update(_brightness[TOTAL]);
}

void LED::setUserBrightness(uint8_t brightness)
{
    _brightness[USER] = brightness;
}

void LED::setBrightness(uint8_t brightness)
{
    _brightness[TOTAL] = brightness;
    LED::update(_brightness[TOTAL]);
}

void LED::setColourDepth(uint8_t colourDepth)
{
    _colourDepth = colourDepth;
    // have to add more things here
    _stepValue = 256/pow(2,_colourDepth);
    //we need to prepare variables again
    if(_control & (1<<PULSE))
        pulse(true);
    if(_control & (1<<FLASH))
        flash(true);
}

uint8_t LED::getColourDepth()
{
    return _colourDepth;
}

uint8_t LED::getBrightness()
{
    // we don't want to call calculateBrightness here,
    // instead, return the value of brightness after the last update
    return _brightness[TOTAL];
}

uint8_t LED::getActiveSources()
{
    // return the number of sources that are routed to the LED
    uint8_t count = 0;
    for (int i = 0; i < 7; i++)
    {
        if(_control & (1<<i))
            count++;
    }
    return count;
}

byte LED::getControlParameters()
{
    // return the _control byte
    return _control;
}

bool LED::setControlParameters(byte control)
{
    if(control & (1<<PULSE) && control & (1<<FLASH))
        //flash and pulse should not be simultaneuosly on
        return false;
    else
    {
        (control & (1<<FLASH))          ? flash(true)           : flash(false);
        (control & (1<<PULSE))          ? pulse(true)           : pulse(false);
        (control & (1<<USER))           ? routeUser(true)       : routeUser(false);
        (control & (1<<LIGHT))          ? watchExtLight(true)   : watchExtLight(false);
        (control & (1<<RANDOM))         ? invert(true)          : invert(false);
        (control & (1<<ADC_SOURCE1))    ? routeAdcSource1(true) : routeAdcSource1(false);
        (control & (1<<ADC_SOURCE2))    ? routeAdcSource2(true) : routeAdcSource2(false);
        (control & (1<<ADC_SOURCE3))    ? routeAdcSource3(true) : routeAdcSource3(false);
    }
    return true;
}

void LED::setRandomStep(uint8_t randomStep, uint8_t randomJitter)
{
    _randomStep = randomStep;
    _randomJitter = randomJitter;
}

void LED::setTime(uint16_t onTime, uint16_t offTime)
{
    // set the time variables for flashing and pulsing
    _onTime = onTime;
    _offTime = offTime;
}

void LED::setTime(uint16_t Time)
{
    // set the time variables for flashing and pulsing
    _onTime = Time;
    _offTime = Time;
}

// invert the signals for the LED
void LED::invert(bool invertColour)
{
    _invert = invertColour;
}

// limits the maximum brightness, in a sense that
// brightness is never constrained.
// Eg: If pulse time is 256 ms and limit is 127,
// each step increase will take 2 ms.
void LED::limit(uint8_t brightness)
{
    // defaults to 0, which means no limit
    _maxBrightness = brightness;
    // update timing parameters
    if(_control & (1<<PULSE))
        pulse(true);
    if(_control & (1<<FLASH))
        flash(true);
}

void LED::limit(uint8_t minBrightness, uint8_t maxBrightness)
{
    // defaults to 0, which means no limit
    _maxBrightness = maxBrightness;
    // defaults to 0, which means no limit
    _minBrightness = minBrightness;

    // update timing parameters
    if(_control & (1<<FLASH))
        flash(true);
}

void LED::limitPulseBrightness(uint8_t minPulseBrightness, uint8_t maxPulseBrightness)
{
    // defaults to 0, which means no limit
    _maxPulseBrightness = maxPulseBrightness;
    // defaults to 0, which means no limit
    _minPulseBrightness = minPulseBrightness;

    // update timing parameters
    if(_control & (1<<PULSE))
        pulse(true);
}

void LED::flash(bool flash)
{
    if(!flash)
    {
        _control = _control & ~(1<<FLASH);
        // nothing more to do if flashing has been switched off
        return;
    }

    // switch off pulsing before flashing
    _control &= ~(1<<PULSE);
    _control |= (1<<FLASH);
    _state = false;

    _activeSources = getActiveSources();
}

void LED::pulse(bool pulse)
{
    if(!pulse)
    {
        _control = _control & ~(1<<PULSE);
        // nothing more to do if pulse has been switched off
        return;
    }

    // switch off flashing before pulsing
    _control &= ~(1<<FLASH);
    _control |= (1<<PULSE);
    _state = false;
    _brightness[PULSE] = 0;
    _incrementStepTime = (_onTime/(_maxPulseBrightness - _minPulseBrightness))*(_stepValue);
    _decrementStepTime = (_offTime/(_maxPulseBrightness - _minPulseBrightness))*(_stepValue);

    _activeSources = getActiveSources();
}

void LED::routeRandom(bool argRandom)
{
    if(!argRandom)
    {
        _control = _control & ~(1<<RANDOM);
        return;
    }

    _control |= (1<<RANDOM);

    _activeSources = getActiveSources();
}

void LED::routeUser(bool user)
{
    if(!user)
    {
        _control &= ~(1<<USER);
        // nothing more to do if user input is disabled
        return;
    }

    _control |= (1<<USER);

    _activeSources = getActiveSources();
}

void LED::routeAdcSource1(bool adcSource1)
{
    if(!adcSource1)
    {
        _control &= ~(1<<ADC_SOURCE1);
        // nothing more to do if adcSource1 is not being routed
        return;
    }

    // set default breather mode off
    adcSource1IncrCount = 0;
    adcSource1DecrCount = 0;
    adcSource1IncrCountThresh = 1;
    adcSource1DecrCountThresh = 1;

    _control |= (1<<ADC_SOURCE1);

    _activeSources = getActiveSources();
}

void LED::routeAdcSource1(bool adcSource1, uint8_t argIncrThresh, uint8_t argDecrThresh)
{
    if(!adcSource1)
    {
        _control &= ~(1<<ADC_SOURCE1);
        // nothing more to do if adcSource1 is not being routed
        return;
    }

    // reset counters and update valuesfor thresholds
    adcSource1IncrCount = 0;
    adcSource1DecrCount = 0;
    adcSource1IncrCountThresh = argIncrThresh;
    adcSource1DecrCountThresh = argDecrThresh;

    _control |= (1<<ADC_SOURCE1);

    _activeSources = getActiveSources();
}

void LED::routeAdcSource2(bool adcSource2)
{
    if(!adcSource2)
    {
        _control &= ~(1<<ADC_SOURCE2);
        // nothing more to do if adcSource2 is not being routed
        return;
    }

    // reset counters and update valuesfor thresholds
    adcSource2IncrCount = 0;
    adcSource2DecrCount = 0;
    adcSource2IncrCountThresh = 1;
    adcSource2DecrCountThresh = 1;

    _control |= (1<<ADC_SOURCE2);

    _activeSources = getActiveSources();
}

void LED::routeAdcSource2(bool adcSource2, uint8_t argIncrThresh, uint8_t argDecrThresh)
{
    if(!adcSource2)
    {
        _control &= ~(1<<ADC_SOURCE2);
        // nothing more to do if adcSource2 is not being routed
        return;
    }

    // reset counters and update valuesfor thresholds
    adcSource2IncrCount = 0;
    adcSource2DecrCount = 0;
    adcSource2IncrCountThresh = argIncrThresh;
    adcSource2DecrCountThresh = argDecrThresh;

    _control |= (1<<ADC_SOURCE2);

    _activeSources = getActiveSources();
}

void LED::routeAdcSource3(bool adcSource3)
{
    if(!adcSource3)
    {
        _control &= ~(1<<ADC_SOURCE3);
        // nothing more to do if adcSource3 is not being routed
        return;
    }

    _control |= (1<<ADC_SOURCE3);

    _activeSources = getActiveSources();
}

void LED::watchExtLight(bool extlight, uint8_t brightness)
{
    _brightness[LIGHT] = brightness;
    if(!extlight)
    {
        _control &= ~(1<<LIGHT);
        return;
    }
    _control |= (1<<LIGHT);
}

void LED::watchExtLight(bool extlight)
{
    if(!extlight)
    {
        _control &= ~(1<<LIGHT);
        return;
    }
    _control |= (1<<LIGHT);
}

uint8_t LED::calculateBrightness()
{
    // averages out all values of brightness
    // a future update will use specific weights to calculate the average

    uint64_t timeNow = millis();

    // external light
    if(_control & (1<<LIGHT))
    {
        uint8_t value = GETEXTLIGHT;
        if(value > _brightness[LIGHT])
        {
            return 0;
        }
    }

    // flash
    if(_control & (1<<FLASH))
    {
        // here, state is used to check whether the LED is activated or not
        if((_state) && (timeNow - _prevUpdate) > _onTime)
        {
            _state = false;
            _prevUpdate = timeNow;
            #ifndef FLASH_GATE
                _brightness[FLASH] = _minBrightness;
            #else
                return _minBrightness;
            #endif
        }
        if((!_state) && (timeNow - _prevUpdate) > _offTime)
        {
            _state = true;
            _prevUpdate = timeNow;
            #ifndef FLASH_GATE
                _brightness[FLASH] = _maxBrightness;
            #endif
        }

    }

    //pulse
    if(_control & (1<<PULSE))
    {
        // here, state is used to check whether the brightness is increasing (0) or decreasing (1)
        // _state is explicitly set 'false' and brightness set to 0 before starting pulse
        if(!(_state) && (timeNow - _prevUpdate) > _incrementStepTime)
        {
            if(_brightness[PULSE] == _maxPulseBrightness)
                _state = true;
            else
                _brightness[PULSE] = constrain(_brightness[PULSE] + _stepValue, _minPulseBrightness, _maxPulseBrightness);
            _prevUpdate = timeNow;
        }
        if((_state) && (timeNow - _prevUpdate) > _decrementStepTime)
        {
            if(_brightness[PULSE] == _minPulseBrightness)
                _state = false;
            else
                _brightness[PULSE] = constrain(_brightness[PULSE] - _stepValue, _minPulseBrightness, _maxPulseBrightness);
            _prevUpdate = timeNow;
        }
    }

    // adcSource1 routing
    if(_control & (1<<ADC_SOURCE1))
    {
        // value is a 10 bit number, drop some bits to accomodate for set colour depth
        if(GETADC_SOURCE1 > _brightness[ADC_SOURCE1])
        {
            adcSource1IncrCount++;
            if(adcSource1IncrCount == adcSource1IncrCountThresh)
            {
                _brightness[ADC_SOURCE1] = min(_brightness[ADC_SOURCE1] + 1, 255);
                adcSource1IncrCount = 0;
            }
        }
        else
        {
            adcSource1DecrCount++;
            if(adcSource1DecrCount == adcSource1DecrCountThresh)
            {
                _brightness[ADC_SOURCE1] = max(_brightness[ADC_SOURCE1] - 1, 0);
                adcSource1DecrCount = 0;
            }
        }
    }

    // adcSource2 routing
    if(_control & (1<<ADC_SOURCE2))
    {
        // value is a 10 bit number, drop some bits to accomodate for set colour depth
        _brightness[ADC_SOURCE2] = GETADC_SOURCE2;
    }

    // adcSource3 routing
    if(_control & (1<<ADC_SOURCE3))
    {
        // value is a 10 bit number, drop some bits to accomodate for set colour depth
        _brightness[ADC_SOURCE3] = GETADC_SOURCE3;
    }

    // random brightness gets applied on every update
    if(_control & (1<<RANDOM))
    {
        // slowly move towards the randomly defined value
        _brightness[RANDOM] = (GETRANDOM(0,_randomStep+1))*GETRANDOM(1,_randomJitter+1);
    }

    float totalBrightness = 0;
    // weighted values of all the individual brightness values make the total.
    // a future update may have provisions for absolute value or custom weights.
    // equal weighting is used for now:
    // weight = 255/_activeSources;

    #ifndef FLASH_GATE
    for (int i = 0; i < 7; i++)
    #else
    for (int i = 1; i < 7; i++) // not including flash brightness
    #endif
    {
        if(_control & (1<<i))
            totalBrightness += float(_brightness[i])/_activeSources;
            #ifdef DEBUG_PRINT
                printf("%d",_brightness[i]);
                printf("\t");
            #endif
    }
    #ifdef DEBUG_PRINT
    printf("\n");
    #endif

    // if limits are different, constrain totalBrightness
    // this might be not required, will have to check the math here
    if(_maxBrightness != 255 || _minBrightness)
        return constrain(totalBrightness, _minBrightness, _maxBrightness);
    else
        return totalBrightness;
}
