//
// Edited by valti on 01.08.2022.
//
//    FILE: ADS1X15.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.7
//    DATE: 2013-03-24
// PUPROSE: Arduino library for ADS1015 and ADS1115
//     URL: https://github.com/RobTillaart/ADS1X15


#include "ADS.h"

// REGISTERS
#define ADS1X15_REG_CONVERT         0x00
#define ADS1X15_REG_CONFIG          0x01
#define ADS1X15_REG_LOW_THRESHOLD   0x02
#define ADS1X15_REG_HIGH_THRESHOLD  0x03

// CONFIG REGISTER
#define ADS1X15_OS_NOT_BUSY         0x8000

// BIT 12-14    read differential
#define ADS1X15_MUX_DIFF_0_1        0x0000
#define ADS1X15_MUX_DIFF_0_3        0x1000
#define ADS1X15_MUX_DIFF_1_3        0x2000
#define ADS1X15_MUX_DIFF_2_3        0x3000


#define ADS_CONF_CHAN_1  0x00
#define ADS_CONF_CHAN_4  0x01
#define ADS_CONF_RES_12  0x00
#define ADS_CONF_RES_16  0x04
#define ADS_CONF_NOGAIN  0x00
#define ADS_CONF_GAIN    0x10
#define ADS_CONF_NOCOMP  0x00
#define ADS_CONF_COMP    0x20


bool isReady();
float getVoltage();
void requestADC(uint16_t readmode);
uint16_t readRegister(uint8_t reg);
bool writeRegister(uint8_t reg, uint16_t value);

bool isConnected()
{
    Wire.beginTransmission(ADS1115_ADDRESS);
    return (Wire.endTransmission() == 0);
}

bool dataReady = false;

void readyHandler() {
    dataReady = true;
}

bool beginADS()
{
    Wire.begin();
    if (! isConnected()) return false;
    writeRegister(ADS1X15_REG_LOW_THRESHOLD, 0);
    writeRegister(ADS1X15_REG_HIGH_THRESHOLD, 0x8000);//1 << 15

    attachInterrupt(1, readyHandler, RISING);
    return true;
}


float maxVoltage;

float getMaxVoltage(uint16_t gain) {
    switch (gain) {
        case ADS1X15_PGA_6_144V: return 6.144;
        case ADS1X15_PGA_4_096V: return 4.096;
        case ADS1X15_PGA_2_048V: return 2.048;
        case ADS1X15_PGA_1_024V: return 1.024;
        case ADS1X15_PGA_0_512V: return 0.512;
        case ADS1X15_PGA_0_256V: return 0.256;
    }
    return 0;
}

uint16_t getPinMask(uint8_t pin) {
    switch (pin) {
        case 0: return ADS1X15_MUX_DIFF_0_3;
        case 1: return ADS1X15_MUX_DIFF_1_3;
        case 2: return ADS1X15_MUX_DIFF_2_3;
    }
    return 0;
}

bool requestValue(uint8_t pin, uint16_t gain) {
    maxVoltage = getMaxVoltage(gain);
    uint16_t pinMask = getPinMask(pin);
    if (maxVoltage == 0 || pinMask == 0) {
        return false;
    }
    requestADC(pinMask | gain);
    dataReady = false;
    return true;
}

bool getValueIfExists(float* valuePtr)
{
    if (dataReady == false) {
        return false;
    }
    if (isReady() == false) {
        return false;
    }
    *valuePtr = getVoltage();
    return true;
}

bool isReady()
{
    uint16_t val = readRegister(ADS1X15_REG_CONFIG);
    return ((val & ADS1X15_OS_NOT_BUSY) > 0);
}

float getVoltage()
{
    int16_t value = readRegister(ADS1X15_REG_CONVERT);

    if (value == 0) return 0;

    float volts = maxVoltage;
    if (volts < 0) return 0;

    volts *= value;
    volts /= 32767;
    return volts;
}

void requestADC(uint16_t mask)
{
//    1<<15 | //OS : Start a single conversion (15)
//    1 << 8 | // Mode Single-shot mode or power-down state (8)
//    0b010 << 5 //Data rate 010 : 32 SPS (7:5)
//    //Comparator mode - ignore(4)
//    1 << 3 //Comparator polarity 1 : Active high (3)
//    //Latching comparator - ignore (2)
//    0 //Comparator queue and disable 00 : Assert after one conversion (0:1)
    writeRegister(ADS1X15_REG_CONFIG, 0x8148 | mask);
}

bool writeRegister(uint8_t reg, uint16_t value)
{
    Wire.beginTransmission(ADS1115_ADDRESS);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value >> 8));
    Wire.write((uint8_t)(value & 0xFF));
    return (Wire.endTransmission() == 0);
}

uint16_t readRegister(uint8_t reg)
{
    Wire.beginTransmission(ADS1115_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();

    int rv = Wire.requestFrom((int) ADS1115_ADDRESS, (int) 2);
    if (rv == 2)
    {
        uint16_t value = Wire.read() << 8;
        value += Wire.read();
        return value;
    }
    return 0x0000;
}

