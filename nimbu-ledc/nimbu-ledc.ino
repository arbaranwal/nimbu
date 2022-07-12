//#include <SoftwareSerial.h>
#include "LED.h"
#include "definitions.h"
#include "adcHandler.h"
#include <Wire.h>
#include "appl.h"
#define PRU_SLAVE_ADDRESS 0x8
#define PRU_SELF_ID 0x1
//String red = "", green = "", blue = "";
//SoftwareSerial blt(8,7);
#define I2C_CMD_SIZE    (8)
#define I2C_BUFFER_SIZE (I2C_CMD_SIZE*2)
byte i2cRxBuffer[I2C_BUFFER_SIZE] = {0};
byte validCommand[I2C_CMD_SIZE] = {0};
byte i2cTxResp = 0;
uint8_t respPending = 0;
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
    Serial.println("Init");

    setupADC();
    initADC();

    // wait for ADC to stabilise before calibrating
    delay(500);
    calibrateBass();
    Wire.begin(PRU_SLAVE_ADDRESS);
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
    // Red.routeBass(true, 2, 4);
    // Red.routeMid(true);
    Red.routeTreble(true);
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
    // Green.routeMid(true);
    Green.routeTreble(true);
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

    #ifdef DEBUG_FPS
    prevFPSMillis = millis();
    #endif
    #ifdef ADC_INTERRUPT_MODE
    doFrameUpdate = 1;
    #endif

    // update once to reset to zero (in case of invert(true)))
    updateFrame();
}

/**
 * @brief Callback function for Wire.onReceive()
 * Increments the writePointer to a ring-buffer maintained in global memory.
 * The readPointer gets incremented when the data is read and a valid
 * I2C_CMD_SIZE-byte command is read at once.
 * 
 * @param byteCount 
 * @return true 
 * @return false 
 */
bool receiveData(int byteCount)
{
    /// capture I2C_CMD_SIZE bytes at once, helps keep ring-buffer logic simple
    if(byteCount != I2C_CMD_SIZE)
        return false;

    /// set i2cTxResp as RX_FIFO_FULL if wr-ptr is just behind rd-ptr
    if(i2cRxBufferRdPtr == INCWRAP(i2cRxBufferWrPtr, I2C_BUFFER_SIZE))
    {
        i2cTxResp = RX_FIFO_FULL;
        return false;
    }
    
    /// write all I2C_CMD_SIZE bytes into ring-buffer
    for (int i = 0; i < byteCount; i++)
    {
        i2cRxBuffer[i2cRxBufferWrPtr] = Wire.read();
        i2cRxBufferWrPtr = INCWRAP(i2cRxBufferWrPtr, I2C_BUFFER_SIZE);
    }
}

/**
 * @brief function to check if there is a valid command based on read and write pointers.
 * Due to the receiveData function ingesting only I2C_CMD_SIZE bytes at once. The rd/wr pointers will
 * simply ping-pong around values in granularity of I2C_CMD_SIZE units.
 * 
 * @see receiveData
 * @return true 
 * @return false 
 */
bool isCommandAvailable()
{
    return (i2cRxBufferRdPtr != i2cRxBufferWrPtr);
}

/**
 * @brief parse the command
 * 
 * @return true 
 * @return false 
 */
bool parseCommand()
{
    /// check if something is available
    if(!isCommandAvailable())
    {
        return false;
    }

    /// get first byte and check if it's the correct starting opcode
    byte rxByte = i2cRxBuffer[i2cRxBufferRdPtr];
    if((rxByte & 0xF0) != (PRU_SELF_ID << 4))
    {
        /// increment rdPtr by the I2C_CMD_SIZE
        i2cRxBufferRdPtr = INCSWRAP(i2cRxBufferRdPtr, I2C_BUFFER_SIZE, I2C_CMD_SIZE);
        return false;
    }

    respPending++;
    /// extract relevant bytes
    uint8_t rxByteCount = 0;
    validCommand[rxByteCount++] = rxByte;   // store starting opcode in the 0th byte
    i2cRxBufferRdPtr = INCWRAP(i2cRxBufferRdPtr, I2C_BUFFER_SIZE);  // increment read-pointer in RxBuffer

    /// keep fetching I2C_CMD_SIZE bytes
    while(rxByteCount < (I2C_CMD_SIZE))
    {
        rxByte = i2cRxBuffer[i2cRxBufferRdPtr];
        validCommand[rxByteCount] = rxByte;
        i2cRxBufferRdPtr = INCWRAP(i2cRxBufferRdPtr, I2C_BUFFER_SIZE);
        rxByteCount++;
    }
    validCommand[--rxByteCount] = 0xFF;
    #ifdef DEBUG_COMMAND
    Serial.print(validCommand[0],HEX); Serial.print("-");
    Serial.print(validCommand[1],HEX); Serial.print("-");
    Serial.print(validCommand[2],HEX); Serial.print("-");
    Serial.print(validCommand[3],HEX); Serial.print("-");
    Serial.print(validCommand[4],HEX); Serial.print("-");
    Serial.print(validCommand[5],HEX); Serial.print("-");
    Serial.print(validCommand[6],HEX); Serial.print("-");
    Serial.println(validCommand[7],HEX);
    #endif // DEBUG_COMMAND
}

/**
 * @brief send the currently held response
 * 
 * @return none
 */
void sendData(int byteCount)
{
    // Serial.println(byteCount);
    if(respPending)
    {
        Wire.write(i2cTxResp);
        respPending--;
    }
}

int redBright = 0, decrCount = 0, incrCount = 0;
uint64_t prevRedMillis = 0;

/**
 * @brief updates the entire frame (each colour pixel once)
 * 
 */
void updateFrame()
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

uint64_t previ2cRXBufMillis = 0;

/**
 * @brief main loop for ATMega devices
 * 
 */
void loop()
{
    /// update frame
    #ifndef DEBUG_WAVE
        if(doFrameUpdate)
        {
            updateFrame();
            #ifdef ADC_INTERRUPT_MODE
            doFrameUpdate = 0;
            // fire next reading from ADC
            ADCSRA |= (1 << ADSC);
            #endif
        }

    #else
        pollADC();
    #endif
    /// parse I2C command if available
    parseCommand();

    if(millis() - previ2cRXBufMillis > 5000)
    {
        Serial.print(i2cRxBuffer[0],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[1],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[2],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[3],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[4],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[5],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[6],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[7],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[8],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[9],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[10],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[11],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[12],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[13],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[14],HEX); Serial.print("-");
        Serial.print(i2cRxBuffer[15],HEX); Serial.print(" ");
        Serial.print(i2cRxBufferRdPtr,HEX); Serial.print(" ");
        Serial.println(i2cRxBufferWrPtr,HEX);
        previ2cRXBufMillis = millis();
    }
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
