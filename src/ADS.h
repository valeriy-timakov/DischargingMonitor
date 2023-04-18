#pragma once
//
// Edited by valti on 01.08.2022.
//
//
//    FILE: ADS1X15.H
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.7
//    DATE: 2013-03-24
// PUPROSE: Arduino library for ADS1015 and ADS1115
//     URL: https://github.com/RobTillaart/ADS1X15
//


#include "Arduino.h"
#include "Wire.h"


#ifndef ADS
#define ADS

#define ADS1115_ADDRESS_DEFAULT             0x48



// BIT 9-11     gain                        // (0..5) << 9
#define ADS1X15_PGA_6_144V          0x0000  // voltage
#define ADS1X15_PGA_4_096V          0x0200  //
#define ADS1X15_PGA_2_048V          0x0400  // default
#define ADS1X15_PGA_1_024V          0x0600
#define ADS1X15_PGA_0_512V          0x0800
#define ADS1X15_PGA_0_256V          0x0A00

#define MAX_READ_VALUE 32767



static const uint8_t ADS_MAX_INSTANCES_COUNT = 8;

class ADS3x {
public:
    ADS3x(uint8_t readyPin, uint8_t address = ADS1115_ADDRESS_DEFAULT) : address(address),
        readyPin(readyPin), instanceIx(nextInstanceIx++) {}
    bool begin();
    bool requestValue(uint8_t pin, uint16_t gain);
    bool getRelativeValueIfExists(uint16_t *valuePtr);

    static float convertToVoltage(uint16_t value, uint16_t gain);

private:
    void requestADC(uint16_t mask);
    bool isReady();
    bool isConnected();

    static float getMaxVoltage(uint16_t gain);

    bool writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);

    static uint8_t nextInstanceIx;

    uint8_t address;
    uint8_t readyPin;
    uint8_t instanceIx;

};



#endif //ADS


