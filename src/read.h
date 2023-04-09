//
// Created by valti on 03.08.2022.
//

#pragma once

#ifndef ATTINYTEST_READ_H
#define ATTINYTEST_READ_H

#include "EEPROMStorage.h"



enum ReadMode {
    M_VOLTAGE = 0,
    M_CURRENT = 1,
    M_WAIT = 2
};

class Reader {
public:
    Reader(EEPROMStorage &storage): storage(storage) {}
    void init();
    void loop();
    const Data& getLastData();
    void writeReadInterval(Stream &stream);
    ErrorCode setReadInterval(uint32_t value);
    void writeLastReadTimeStamp(Stream &stream);
    ErrorCode syncTime(uint32_t value);
    void writeTimeStamp(Stream &stream);
    ErrorCode performRead();

    static float getCoefficient(ReadMode mode);
    static float deserializeCurrent(const Data *data);
    static float deserializeVoltage(const Data *data);
private:
    EEPROMStorage &storage;

    static float deserializeMilli(uint16_t relativeValue, ReadMode mode);

};


#endif //ATTINYTEST_READ_H
