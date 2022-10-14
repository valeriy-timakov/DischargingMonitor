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
#define ADS1115_ADDRESS                   0x48


#define ADS1X15_OK                        0
#define ADS1X15_INVALID_VOLTAGE           -100
#define ADS1X15_INVALID_GAIN              0xFF


// BIT 9-11     gain                        // (0..5) << 9
#define ADS1X15_PGA_6_144V          0x0000  // voltage
#define ADS1X15_PGA_4_096V          0x0200  //
#define ADS1X15_PGA_2_048V          0x0400  // default
#define ADS1X15_PGA_1_024V          0x0600
#define ADS1X15_PGA_0_512V          0x0800
#define ADS1X15_PGA_0_256V          0x0A00


void resetADS();
bool beginADS();
bool requestValue(uint8_t pin, uint16_t gain);
bool getValueIfExists(float* valuePtr);




#endif //ADS


