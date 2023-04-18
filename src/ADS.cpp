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

uint8_t ADS3x::nextInstanceIx = 0;


uint16_t readRegister(uint8_t reg);
bool writeRegister(uint8_t reg, uint16_t value);
static uint16_t lastGain;

bool ADS3x::isConnected()
{
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

static volatile uint8_t dataReady = 0;

inline void readyHandler(uint8_t idx) {
    dataReady = 1 << idx;
}

void readyHandler0() {
    readyHandler(0);
}

void readyHandler1() {
    readyHandler(1);
}

void readyHandler2() {
    readyHandler(2);
}

void readyHandler3() {
    readyHandler(3);
}

void readyHandler4() {
    readyHandler(4);
}

void readyHandler5() {
    readyHandler(5);
}

void readyHandler6() {
    readyHandler(6);
}

void readyHandler7() {
    readyHandler(7);
}

bool ADS3x::begin() {
    if (! isConnected()) return false;
    writeRegister(ADS1X15_REG_LOW_THRESHOLD, 0);
    writeRegister(ADS1X15_REG_HIGH_THRESHOLD, 0x8000);//1 << 15

    void (*pRadeyHandler)(void)  = NULL;
    switch (instanceIx) {
        case 0: pRadeyHandler = &readyHandler0;
            break;
        case 1: pRadeyHandler = &readyHandler1;
            break;
        case 2: pRadeyHandler = &readyHandler2;
            break;
        case 3: pRadeyHandler = &readyHandler3;
            break;
        case 4: pRadeyHandler = &readyHandler4;
            break;
        case 5: pRadeyHandler = &readyHandler5;
            break;
        case 6: pRadeyHandler = &readyHandler6;
            break;
        case 7: pRadeyHandler = &readyHandler7;
            break;
    }

    attachInterrupt(readyPin, pRadeyHandler, RISING);
    return true;
}


float ADS3x::getMaxVoltage(uint16_t gain) {
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

bool ADS3x::requestValue(uint8_t pin, uint16_t gain) {
    float maxVoltage = getMaxVoltage(gain);
    uint16_t pinMask = getPinMask(pin);
    if (maxVoltage == 0 || pinMask == 0) {
        return false;
    }
    requestADC(pinMask | gain);
    dataReady &= ~(1 << instanceIx);
    lastGain = gain;
    return true;
}

bool ADS3x::isReady() {
    uint16_t val = readRegister(ADS1X15_REG_CONFIG);
    return ((val & ADS1X15_OS_NOT_BUSY) > 0);
}

bool ADS3x::getRelativeValueIfExists(uint16_t* valuePtr) {
    if ( ( dataReady & (instanceIx + 1) ) == 0 ) {
        return false;
    }
    if (isReady() == false) {
        return false;
    }
    *valuePtr = readRegister(ADS1X15_REG_CONVERT);
    return true;
}

float ADS3x::convertToVoltage(uint16_t value, uint16_t gain) {
    if (value == 0) return 0;

    float volts = getMaxVoltage(gain);
    if (volts < 0) return 0;

    volts *= value;
    volts /= MAX_READ_VALUE;
    return volts;

}

void ADS3x::requestADC(uint16_t mask) {
//    1<<15 | //OS : Start a single conversion (15)
//    1 << 8 | // Mode Single-shot mode or power-down state (8)
//    0b010 << 5 //Data rate 010 : 32 SPS (7:5)
//    //Comparator mode - ignore(4)
//    1 << 3 //Comparator polarity 1 : Active high (3)
//    //Latching comparator - ignore (2)
//    0 //Comparator queue and disable 00 : Assert after one conversion (0:1)
    writeRegister(ADS1X15_REG_CONFIG, 0x8148 | mask);
}

bool ADS3x::writeRegister(uint8_t reg, uint16_t value)
{
    Wire.beginTransmission(address);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value >> 8));
    Wire.write((uint8_t)(value & 0xFF));
    return (Wire.endTransmission() == 0);
}

uint16_t ADS3x::readRegister(uint8_t reg)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();

    int rv = Wire.requestFrom((int) address, (int) 2);
    if (rv == 2)
    {
        uint16_t value = Wire.read() << 8;
        value += Wire.read();
        return value;
    }
    return 0x0000;
}

