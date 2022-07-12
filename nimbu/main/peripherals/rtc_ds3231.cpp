#include <rtc_ds3231.h>

uint8_t DS3231RTC::errCode;

DS3231RTC::DS3231RTC(uint8_t address)
{
    address++;
}

// Probe the peripheral
int DS3231RTC::init()
{
    return i2c_check_device(DS32_ADDR);
}

// Read the current time from the RTC and return it as a time_t
// value. Returns a zero value if an I2C error occurred (e.g. RTC
// not present).
time_t DS3231RTC::get()
{
    tmElements_t tm;

    if ( read(tm) ) return 0;
    return( mktime(&tm) );
}

// Set the RTC to the given time_t value and clear the
// oscillator stop flag (OSF) in the Control/Status register.
// Returns the I2C status (zero if successful).
uint8_t DS3231RTC::set(time_t t)
{
    // tmElements_t tm;
    tmElements_t* tm_p;
    // tm_p = &tm;
    tm_p = gmtime(&t);
    return ( write(*tm_p) );
}

// Read the current time from the RTC and return it in a tmElements_t
// structure. Returns the I2C status (zero if successful).
int DS3231RTC::read(tmElements_t &tm)
{
    uint8_t reg = DS32_SECONDS;
    uint8_t values[7] = {0};
    // set first register (DS32_SECONDS) and
    // request 7 bytes (secs, min, hr, dow, date, mth, yr)
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, DS32_ADDR, &reg, 1, values, 7, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if ( err != ESP_OK ) { return err; }
    tm.tm_sec = bcd2dec(values[0] & ~_BV(DS1307_CH));
    tm.tm_min = bcd2dec(values[1]);
    tm.tm_hour = bcd2dec(values[2] & ~_BV(DS32_HR1224));   // assumes 24hr clock
    tm.tm_wday = values[3];
    tm.tm_mday = bcd2dec(values[4]);
    tm.tm_mon = bcd2dec(values[5] & ~_BV(DS32_CENTURY)); // don't use the Century bit
    tm.tm_year = y2kYearToTm(bcd2dec(values[6]));
    return 0;
}

// Set the RTC time from a tmElements_t structure and clear the
// oscillator stop flag (OSF) in the Control/Status register.
// Returns the I2C status (zero if successful).
int DS3231RTC::write(tmElements_t &tm)
{
    esp_err_t err;
    uint8_t txBuffer[8];
    txBuffer[0] = (DS32_SECONDS);
    txBuffer[1] = (dec2bcd(tm.tm_sec));
    txBuffer[2] = (dec2bcd(tm.tm_min));
    txBuffer[3] = (dec2bcd(tm.tm_hour));         // sets 24 hour format (Bit 6 == 0)
    txBuffer[4] = (tm.tm_wday);
    txBuffer[5] = (dec2bcd(tm.tm_mday));
    txBuffer[6] = (dec2bcd(tm.tm_mon));
    txBuffer[7] = (dec2bcd(tmYearToY2k(tm.tm_year)));

    err = i2c_master_write_to_device(I2C_MASTER_NUM, DS32_ADDR, txBuffer, 8, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if(err) { return err; }
    int s = readRTC(DS32_STATUS);               // read the status register
    err = writeRTC( DS32_STATUS, s & ~_BV(DS32_OSF) );    // clear the Oscillator Stop Flag
    return err;
}

// Write multiple bytes to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
int DS3231RTC::writeRTC(uint8_t addr, uint8_t* write_buffer, uint8_t write_size)
{
    esp_err_t err = ESP_OK;
    uint8_t buffer[32] = { 0 };
    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(buffer, sizeof(buffer));
    assert (handle != NULL);

    // create the handle to be passed around
    err = i2c_master_start(handle);
    if (err != ESP_OK) {
        goto end;
    }
    // send device address
    err = i2c_master_write_byte(handle, DS32_ADDR << 1 | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) {
        goto end;
    }
    // send register address
    err = i2c_master_write_byte(handle, addr << 1 | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) {
        goto end;
    }
    // send write buffer
    err = i2c_master_write(handle, write_buffer, write_size, true);
    if (err != ESP_OK) {
        goto end;
    }
    // stop transaction
    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);
end:
    i2c_cmd_link_delete_static(handle);
    return err;
}

// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
int DS3231RTC::writeRTC(uint8_t addr, uint8_t value)
{
    return ( writeRTC(addr, &value, 1) );
}

// Read multiple bytes from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Number of bytes (nBytes) must be between 1 and 32 (Wire library
// limitation).
// Returns the I2C status (zero if successful).
int DS3231RTC::readRTC(uint8_t addr, uint8_t* values, uint8_t nBytes)
{
    i2c_master_write_read_device(I2C_MASTER_NUM, DS32_ADDR, &addr, 1, values, nBytes, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return 0;
}

// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
int DS3231RTC::readRTC(uint8_t addr)
{
    uint8_t b;
    readRTC(addr, &b, 1);
    return b;
}

// Set an alarm time. Sets the alarm registers only.  To cause the
// INT pin to be asserted on alarm match, use alarmInterrupt().
// This method can set either Alarm 1 or Alarm 2, depending on the
// value of alarmType (use a value from the ALARM_TYPES_t enumeration).
// When setting Alarm 2, the seconds value must be supplied but is
// ignored, recommend using zero. (Alarm 2 has no seconds register.)
void DS3231RTC::setAlarm(ALARM_TYPES_t alarmType, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t daydate)
{
    seconds = dec2bcd(seconds);
    minutes = dec2bcd(minutes);
    hours = dec2bcd(hours);
    daydate = dec2bcd(daydate);
    if (alarmType & 0x01) seconds |= _BV(DS32_A1M1);
    if (alarmType & 0x02) minutes |= _BV(DS32_A1M2);
    if (alarmType & 0x04) hours |= _BV(DS32_A1M3);
    if (alarmType & 0x10) daydate |= _BV(DS32_DYDT);
    if (alarmType & 0x08) daydate |= _BV(DS32_A1M4);

    uint8_t addr;
    if ( !(alarmType & 0x80) ) {    // alarm 1
        addr = DS32_ALM1_SEC;
        writeRTC(addr++, seconds);
    }
    else {
        addr = DS32_ALM2_MIN;
    }
    writeRTC(addr++, minutes);
    writeRTC(addr++, hours);
    writeRTC(addr++, daydate);
}

// Set an alarm time. Sets the alarm registers only. To cause the
// INT pin to be asserted on alarm match, use alarmInterrupt().
// This method can set either Alarm 1 or Alarm 2, depending on the
// value of alarmType (use a value from the ALARM_TYPES_t enumeration).
// However, when using this method to set Alarm 1, the seconds value
// is set to zero. (Alarm 2 has no seconds register.)
void DS3231RTC::setAlarm(ALARM_TYPES_t alarmType, uint8_t minutes, uint8_t hours, uint8_t daydate)
{
    setAlarm(alarmType, 0, minutes, hours, daydate);
}

// Enable or disable an alarm "interrupt" which asserts the INT pin
// on the RTC.
void DS3231RTC::alarmInterrupt(ALARM_NBR_t alarmNumber, bool interruptEnabled)
{
    uint8_t controlReg = readRTC(DS32_CONTROL);
    uint8_t mask = _BV(DS32_A1IE) << (alarmNumber - 1);
    if (interruptEnabled) {
        controlReg |= mask;
    }
    else {
        controlReg &= ~mask;
    }
    writeRTC(DS32_CONTROL, controlReg);
}

// Returns true or false depending on whether the given alarm has been
// triggered, and resets the alarm flag bit.
bool DS3231RTC::alarm(ALARM_NBR_t alarmNumber)
{
    uint8_t statusReg = readRTC(DS32_STATUS);
    uint8_t mask = _BV(DS32_A1F) << (alarmNumber - 1);
    if (statusReg & mask) {
        statusReg &= ~mask;
        writeRTC(DS32_STATUS, statusReg);
        return true;
    }
    else {
        return false;
    }
}

// Returns true or false depending on whether the given alarm has been
// triggered, without resetting the alarm flag bit.
bool DS3231RTC::checkAlarm(ALARM_NBR_t alarmNumber)
{
    uint8_t statusReg = readRTC(DS32_STATUS);
    uint8_t mask = _BV(DS32_A1F) << (alarmNumber - 1);
    return (statusReg & mask);
}

// Clears the given alarm flag bit if it is set.
// Returns the value of the flag bit before if was cleared.
bool DS3231RTC::clearAlarm(ALARM_NBR_t alarmNumber)
{
    uint8_t statusReg = readRTC(DS32_STATUS);
    uint8_t mask = _BV(DS32_A1F) << (alarmNumber - 1);
    bool retVal = statusReg & mask;
    if (retVal) {
        statusReg &= ~mask;
        writeRTC(DS32_STATUS, statusReg);
    }
    return retVal;
}

// Enable or disable the square wave output.
// Use a value from the SQWAVE_FREQS_t enumeration for the parameter.
void DS3231RTC::squareWave(SQWAVE_FREQS_t freq)
{
    uint8_t controlReg = readRTC(DS32_CONTROL);
    if (freq >= SQWAVE_NONE) {
        controlReg |= _BV(DS32_INTCN);
    }
    else {
        controlReg = (controlReg & 0xE3) | (freq << DS32_RS1);
    }
    writeRTC(DS32_CONTROL, controlReg);
}

// Returns the value of the oscillator stop flag (OSF) bit in the
// control/status register which indicates that the oscillator is or
// was stopped, and that the timekeeping data may be invalid.
// Optionally clears the OSF bit depending on the argument passed.
bool DS3231RTC::oscStopped(bool clearOSF)
{
    uint8_t s = readRTC(DS32_STATUS);   // read the status register
    bool ret = s & _BV(DS32_OSF);       // isolate the osc stop flag to return to caller
    if (ret && clearOSF) {              // clear OSF if it's set and the caller wants to clear it
        writeRTC( DS32_STATUS, s & ~_BV(DS32_OSF) );
    }
    return ret;
}

// Returns the temperature in Celsius times four.
int16_t DS3231RTC::temperature()
{
    union int16_byte {
        int16_t i;
        uint8_t b[2];
    } rtcTemp;

    rtcTemp.b[0] = readRTC(DS32_TEMP_LSB);
    rtcTemp.b[1] = readRTC(DS32_TEMP_MSB);
    return rtcTemp.i / 64;
}

// Decimal-to-BCD conversion
uint8_t DS3231RTC::dec2bcd(uint8_t n)
{
    return n + 6 * (n / 10);
}

// BCD-to-Decimal conversion
uint8_t __attribute__ ((noinline)) DS3231RTC::bcd2dec(uint8_t n)
{
    return n - 6 * (n >> 4);
}