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
    uint32_t getReadInterval() const;
    uint32_t getLastReadTimeStamp() const;
    uint32_t getTimeStamp() const;
    ErrorCode performRead();
    uint32_t getCurrentId() const;
    void setCurrentId(uint32_t value);

    static float getCoefficient(ReadMode mode);
    static float deserializeCurrent(const Data *data);
    static float deserializeVoltage(const Data *data);
private:
    EEPROMStorage &storage;
    Log &log;
    ADS3x ads;


    uint64_t lastRead = 0;
    uint32_t readInterval = 5000;
    uint32_t baseTime = 0;
    uint64_t baseLocalTime = 0;
    uint32_t currentId = 0;
    ReadMode readMode = M_WAIT;
    bool modeRequested = false;
    Data currData;


    static float deserializeMilli(uint16_t relativeValue, ReadMode mode);

};


#endif //ATTINYTEST_READ_H
