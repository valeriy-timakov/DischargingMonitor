//
// Created by valti on 03.08.2022.
//

#pragma once

#ifndef ATTINYTEST_READ_H
#define ATTINYTEST_READ_H

#include "EEPROMStorage.h"
#include "ADS.h"
#include "Log.h"


#define READER_ADS1115_ADDRESS    0x48
#define ADS1115_READY_PIN_DEFAULT 1

enum ReadMode {
    M_VOLTAGE = 0,
    M_CURRENT = 1,
    M_WAIT = 2
};

class Reader {
public:
    Reader(EEPROMStorage &storage, Log &log): storage(storage), log(log), ads(ADS1115_READY_PIN_DEFAULT) {}
    void init();
    void loop();
    const Data& getLastData();
    void setReadInterval(uint32_t value);
    void syncTime(uint32_t value);
    uint32_t getReadInterval();
    uint32_t getLastReadTimeStamp();
    uint32_t getTimeStamp();
    ErrorCode performRead();

    static float getCoefficient(ReadMode mode);
    static float deserializeCurrent(const Data *data);
    static float deserializeVoltage(const Data *data);
private:
    EEPROMStorage &storage;
    Log &log;
    ADS3x ads;


    static float deserializeMilli(uint16_t relativeValue, ReadMode mode);

};


#endif //ATTINYTEST_READ_H
