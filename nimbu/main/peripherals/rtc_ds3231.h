#ifndef __RTC_DS3231__
#define __RTC_DS3231__

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "definitions.h"
#include "peripherals.h"

//convenience macros to convert to and from tm years 
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)   

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_)  ((( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1) // 1 = Sunday
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_)  (_time_ % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before settting alarms
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) ( previousMidnight(_time_)  + SECS_PER_DAY )   // time at the end of the given day 
#define elapsedSecsThisWeek(_time_)  (elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY) )   // note that week starts on day 1
#define previousSunday(_time_)  (_time_ - elapsedSecsThisWeek(_time_))      // time at the start of the week for the given time
#define nextSunday(_time_) ( previousSunday(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time

/* Useful Macros for converting elapsed time to a time_t */
#define minutesToTime_t ((M)) ( (M) * SECS_PER_MIN)  
#define hoursToTime_t   ((H)) ( (H) * SECS_PER_HOUR)  
#define daysToTime_t    ((D)) ( (D) * SECS_PER_DAY) // fixed on Jul 22 2011
#define weeksToTime_t   ((W)) ( (W) * SECS_PER_WEEK) 

#ifdef _BV
#error "conflicting definition of _BV present"
#else
#define _BV(bit) (1 << (bit))
#endif

class DS3231RTC
{
    public:
        // Alarm masks
        enum ALARM_TYPES_t {
            ALM1_EVERY_SECOND = 0x0F,       // trigger alarm every second
            ALM1_MATCH_SECONDS = 0x0E,      // match seconds
            ALM1_MATCH_MINUTES = 0x0C,      // match minutes *and* seconds
            ALM1_MATCH_HOURS = 0x08,        // match hours *and* minutes, seconds
            ALM1_MATCH_DATE = 0x00,         // match date *and* hours, minutes, seconds
            ALM1_MATCH_DAY = 0x10,          // match day *and* hours, minutes, seconds
            ALM2_EVERY_MINUTE = 0x8E,       // trigger alarm every minute
            ALM2_MATCH_MINUTES = 0x8C,      // match minutes
            ALM2_MATCH_HOURS = 0x88,        // match hours *and* minutes
            ALM2_MATCH_DATE = 0x80,         // match date *and* hours, minutes
            ALM2_MATCH_DAY = 0x90           // match day *and* hours, minutes
        };

        // Square-wave output frequency (TS2, RS1 bits)
        enum SQWAVE_FREQS_t {
            SQWAVE_1_HZ,
            SQWAVE_1024_HZ,
            SQWAVE_4096_HZ,
            SQWAVE_8192_HZ,
            SQWAVE_NONE
        };

        // Alarm numbers for alarm functions
        enum ALARM_NBR_t {
            ALARM_1 = 1,
            ALARM_2 = 2
        };

        static constexpr uint8_t
            DS32_ADDR        {0x68},     // I2C device address
            DS32_SECONDS     {0x00},     // register addresses
            DS32_MINUTES     {0x01},
            DS32_HOURS       {0x02},
            DS32_DAY         {0x03},
            DS32_DATE        {0x04},
            DS32_MONTH       {0x05},
            DS32_YEAR        {0x06},
            DS32_ALM1_SEC    {0x07},
            DS32_ALM1_MIN    {0x08},
            DS32_ALM1_HR     {0x09},
            DS32_ALM1_DYDT   {0x0A},
            DS32_ALM2_MIN    {0x0B},
            DS32_ALM2_HR     {0x0C},
            DS32_ALM2_DYDT   {0x0D},
            DS32_CONTROL     {0x0E},
            DS32_STATUS      {0x0F},
            DS32_AGING       {0x10},
            DS32_TEMP_MSB    {0x11},
            DS32_TEMP_LSB    {0x12},
            DS32_SRAM_START  {0x14},     // first SRAM address
            DS32_SRAM_SIZE   {236},      // number of bytes of SRAM
            DS32_A1M1        {7},        // alarm mask bits
            DS32_A1M2        {7},
            DS32_A1M3        {7},
            DS32_A1M4        {7},
            DS32_A2M2        {7},
            DS32_A2M3        {7},
            DS32_A2M4        {7},
            DS32_EOSC        {7},        // control register bits
            DS32_BBSQW       {6},
            DS32_CONV        {5},
            DS32_RS2         {4},
            DS32_RS1         {3},
            DS32_INTCN       {2},
            DS32_A2IE        {1},
            DS32_A1IE        {0},
            DS32_OSF         {7},        // status register bits
            DS32_BB32KHZ     {6},
            DS32_CRATE1      {5},
            DS32_CRATE0      {4},
            DS32_EN32KHZ     {3},
            DS32_BSY         {2},
            DS32_A2F         {1},
            DS32_A1F         {0},
            DS1307_CH        {7},        // for DS1307 compatibility, Clock Halt bit in Seconds register
            DS32_HR1224      {6},        // Hours register 12 or 24 hour mode (24 hour mode==0)
            DS32_CENTURY     {7},        // Century bit in Month register
            DS32_DYDT        {6};        // Day/Date flag bit in alarm Day/Date registers

        DS3231RTC(uint8_t address);
        int begin();
        static time_t get();    // static needed to work with setSyncProvider() in the Time library
        uint8_t set(time_t t);
        static int read(tmElements_t &tm);
        int write(tmElements_t &tm);
        int writeRTC(uint8_t addr, uint8_t* values, uint8_t nBytes);
        int writeRTC(uint8_t addr, uint8_t value);
        int readRTC(uint8_t addr, uint8_t* values, uint8_t nBytes);
        int readRTC(uint8_t addr);
        void setAlarm(ALARM_TYPES_t alarmType, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t daydate);
        void setAlarm(ALARM_TYPES_t alarmType, uint8_t minutes, uint8_t hours, uint8_t daydate);
        void alarmInterrupt(ALARM_NBR_t alarmNumber, bool alarmEnabled);
        bool alarm(ALARM_NBR_t alarmNumber);
        bool checkAlarm(ALARM_NBR_t alarmNumber);
        bool clearAlarm(ALARM_NBR_t alarmNumber);
        void squareWave(SQWAVE_FREQS_t freq);
        bool oscStopped(bool clearOSF = false);
        int16_t temperature();
        static uint8_t errCode;

    private:
        uint8_t dec2bcd(uint8_t n);
        static uint8_t bcd2dec(uint8_t n);
};

#endif // __RTC_DS3231__