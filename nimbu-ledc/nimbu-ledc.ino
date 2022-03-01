//#include <SoftwareSerial.h>
#include "LED.h"
#include "definitions.h"
#include "adcHandler.h"
#include <Wire.h>
#include "appl.h"
#define LEDC_SLAVE_ADDRESS 0x8
//String red = "", green = "", blue = "";
//SoftwareSerial blt(8,7);
#define I2C_BUFFER_SIZE (32)
#define I2C_CMD_SIZE    (4)
byte i2cRxBuffer[I2C_BUFFER_SIZE] = {0};
byte validCommand[I2C_CMD_SIZE] = {0};
byte i2cTxResp = 0;
uint8_t i2cRxBufferRdPtr, i2cRxBufferWrPtr = 0;

#ifdef DEBUG_FPS
uint64_t prevFPSMillis = 0;
uint32_t fps = 0;
#endif

LED Red(9);
LED Green(10);
LED Blue(11);
LED heartbeat(13);

#ifdef DEBUG_WAVE
void pollADC()
{
    Serial.print("0:");
    Serial.print(ANALOGREAD(CHANNEL0));
    Serial.print("\t1:");
    Serial.print(ANALOGREAD(CHANNEL1));
    Serial.print("\t2:");
    Serial.print(ANALOGREAD(CHANNEL2));
    Serial.print("\t3:");
    Serial.println(ANALOGREAD(CHANNEL3));
}
#endif

void setup()
{
    // blt.begin(9600);
    Serial.begin(115200);

    setupADC();
    initADC();

    // wait for ADC to stabilise before calibrating
    delay(500);
    calibrateBass();
    Wire.begin(LEDC_SLAVE_ADDRESS);
    Wire.onReceive(receiveData);
    Wire.onRequest(sendData);

    Red.setTime(4000, 4000);
    Green.setTime(4000, 2236);//2236);
    Blue.setTime(4000, 4236);//4236);

    // Red.invert(true);
    // Red.pulse(true);
    // Red.limitPulseBrightness(0,40);
    // Red.flash(true);
    // Red.setColourDepth(8);
    // Red.watchExtLight(true,127);
    // Red.routeRandom(true);
    // Red.setRandomStep(24,2);
    Red.routeBass(true, 2, 4);
    // Red.routeMid(true);
    // Red.routeTreble(true);
    // Red.routeUser(true);
    // Red.setUserBrightness(1);
    // Serial.println(Red.getActiveChannels());

    // Green.invert(true);
    // Green.pulse(true);
    // Green.limitPulseBrightness(0,10);
    // Green.flash(true);
    // Green.setTime(2,67);
    // Green.routeRandom(true);
    // Green.setRandomStep(8,1);
    // Green.setColourDepth(4);
    // Green.watchExtLight(true,200);
    // Green.routeBass(true);
    Green.routeMid(true);
    // Green.routeTreble(true);
    // Green.routeUser(true);
    // Green.setUserBrightness(255);
    // Serial.println(Green.getActiveChannels());

    // Blue.invert(true);
    // Blue.pulse(true);
    // Blue.limitPulseBrightness(0,127);
    // Blue.flash(true);
    // Blue.setTime(255,255);
    // Blue.setColourDepth(8);
    // Blue.routeRandom(true);
    // Blue.setRandomStep(16,1);
    // Blue.watchExtLight(true,127);
    // Blue.routeBass(true);
    // Blue.routeMid(true);
    Blue.routeTreble(true);
    // Blue.routeUser(true);
    // Blue.setUserBrightness(255);
    // Blue.limit(40);
    // randomSeed(A2);

    heartbeat.flash(true);

    // update once to reset to zero (in case of invert(true)))
    updateFrame();
}

bool receiveData(int byteCount)
{
    if(i2cRxBufferWrPtr < i2cRxBufferRdPtr)
    if(i2cRxBufferRdPtr == INCWRAP(i2cRxBufferWrPtr, I2C_BUFFER_SIZE))
    {
        i2cTxResp = RX_FIFO_FULL;
        return false;
    }
    
    for (int i = 0; i < byteCount; i++)
    {
        i2cRxBuffer[i2cRxBufferWrPtr] = Wire.read();
        i2cRxBufferWrPtr = INCWRAP(i2cRxBufferWrPtr, I2C_BUFFER_SIZE);
    }
}

bool isCommandAvailable()
{
    return (i2cRxBufferRdPtr != i2cRxBufferWrPtr);
}

bool parseCommand()
{
    // check if something is available
    if(!isCommandAvailable())
    {
        return false;
    }
    // get first byte and check if it's the correct starting opcode
    byte rxByte = i2cRxBuffer[i2cRxBufferRdPtr++];
    if(rxByte != '~')
    {
        return false;
    }
    // extract relevant bytes
    uint8_t rxByteCount = 0;
    while(rxByte != '#' && rxByteCount < (I2C_CMD_SIZE+1))
    {
        rxByte = i2cRxBuffer[i2cRxBufferRdPtr++];
        validCommand[rxByteCount] = rxByte;
        rxByteCount++;
    }
    validCommand[--rxByteCount] = '\0';
    Serial.println((const char *)validCommand);
}

void sendData()
{
    Wire.write("OK");
}

int redBright = 0, decrCount = 0, incrCount = 0;
uint64_t prevRedMillis = 0;
// updates the entire frame (each colour pixel once)
inline void updateFrame()
{
    Red.update();
    Green.update();
    Blue.update();
    heartbeat.update();
    #ifdef DEBUG_FPS
        fps++;
        if(millis() - prevFPSMillis > 1000)
        {
            heartbeat.setTime(4, fps>>2);
            Serial.print("FPS:\t");
            Serial.println(fps);
            fps = 0;
            prevFPSMillis = millis();
        }
    #endif
}

void loop()
{
    // uint8_t flashTime = constrain(GETBASS<<2,50,255);
    // Serial.println(flashTime);
    // Serial.print("\t");
    // Blue.setTime(flashTime);
    #ifndef DEBUG_WAVE
        updateFrame();
    #else
        pollADC();
    #endif
    parseCommand();
}
//
//    int i;
//    String rx = "", red = "", green = "", blue = "";
//    while(blt.available())
//    {
//        char c = blt.read();
//        /*Serial.print(c);
//        Serial.print("\t");
//        Serial.print(char(c));
//        Serial.print("\t");
//        Serial.println(uint8_t(c));*/
//        rx += c;
//        delay(2);
//    }
//    if(rx != "")
//    {
//        Serial.println(rx);
//        uint8_t n = rx.length();
//        for(uint8_t i = 0; i < n; i++)
//        {
//            if(rx.charAt(i) == 'R')
//            {
//                for(uint8_t j = i+1; j < n; j++)
//                {
//                    if(isDigit(rx.charAt(j)))
//                        red += rx.charAt(j);
//                    else
//                        break;
//                }
//            }
//            else if(rx.charAt(i) == 'G')
//            {
//                for(uint8_t j = i+1; j < n; j++)
//                {
//                    if(isDigit(rx.charAt(j)))
//                        green += rx.charAt(j);
//                    else
//                        break;
//                }
//            }
//            else if(rx.charAt(i) == 'B')
//            {
//                for(uint8_t j = i+1; j < n; j++)
//                {
//                    if(isDigit(rx.charAt(j)))
//                        blue += rx.charAt(j);
//                    else
//                        break;
//                }
//            }
//        }
//        n = rx.indexOf('~');
//        if(rx.charAt(n+1) == 'C')
//            if(rx.charAt(n+2) == 'R')
//                Red.setControlParameters(true);
//            if(rx.charAt(n+2) == 'G')
//                Green.setControlParameters(true);
//            if(rx.charAt(n+2) == 'B')
//                Blue.setControlParameters(true);
//
//        if(rx.charAt(n+2) == 'F')
//            Green.flash(true);
//        else if(rx.charAt(n+2) == 'P')
//            Green.pulse(true);
//        else
//        {
//            Green.flash(false);
//            Green.pulse(false);
//        }
//        if(rx.charAt(n+3) == 'F')
//            Blue.flash(true);
//        else if(rx.charAt(n+3) == 'P')
//            Blue.pulse(true);
//        else
//        {
//            Blue.flash(false);
//            Blue.pulse(false);
//        }
//        Serial.print(red);
//        Serial.print("\t");
//        Serial.print(green);
//        Serial.print("\t");
//        Serial.println(blue);
//        if(!red.toInt())
//            Red.routeUser(false);
//        else
//        {
//            Red.routeUser(true);
//            Red.setUserBrightness(red.toInt());
//        }
//        if(!green.toInt())
//            Green.routeUser(false);
//        else
//        {
//            Green.routeUser(true);
//            Green.setUserBrightness(green.toInt());
//        }
//        if(!blue.toInt())
//            Blue.routeUser(false);
//        else
//        {
//            Blue.routeUser(true);
//            Blue.setUserBrightness(blue.toInt());
//        }
//        blt.write("#1~");
//    }
//
//    /*
//    if(rx.length() == 1)
//    rxBuff = rx;
//    if(rx != "")
//    {
//        rx = rxBuff + rx;
//        //Serial.print("Raw:");
//        //Serial.println(rx);
//        for(i = rx.length() - 2; i >= 0; i--)
//        {
//            char c = rx.charAt(i);
//            if(c == ')')
//            {
//                rx = rx.substring(i+1,(rx.length() - 1));
//                break;
//                //Serial.println(rx);
//            }
//        }
//        Serial.println(rx);
//        getVars(rx);
//        redControl = rx[0];
//        greenControl = rx[1];
//        blueControl = rx[2];
//        redControl = rx[3];
//        greenControl = rx[4];
//        blueControl = rx[5];
//    }*/
//}
//
//void getVars(String rx)
//{
//  // Assume we got 3 bytes here, each for red, green and blue
//  // B M T uint8_t
//  /*
//  Incoming Message: 6 bytes
//
//      #X.X.X.R.G.B~
//
//  X = Status Byte of respective colour
//  */
//  Serial.println("Updating...");
//  String findRed = "" , findGreen = "" , findBlue = "";
//  boolean fred = false, fgreen = false, fblue = false;
//  char c;
//  uint8_t length = rx.length();
//  for(int i = 0; i < length; i++)
//  {
//    c = rx.charAt(i);
//    if(isDigit(c) && !fred)
//    {
//        findRed += c;
//        continue;
//    }
//    else if(!fred)
//    {
//        fred = true;
//        continue;
//    }
//    if(isDigit(c) && !fgreen)
//    {
//        findGreen += c;
//        continue;
//    }
//    else if(!fgreen)
//    {
//        fgreen = true;
//        continue;
//    }
//    if(isDigit(c) && !fblue)
//    {
//        findBlue += c;
//        continue;
//    }
//    else if (!fblue)
//    {
//        fblue = true;
//        continue;
//    }
//    }
//    //Serial.println(findRed);
//    //Serial.println(findGreen);
//    //Serial.println(findBlue);
//    //red = findRed.toInt();
//    //green = findGreen.toInt();
//    //blue = findBlue.toInt();
//}
