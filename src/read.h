//
// Created by valti on 03.08.2022.
//

#pragma once

#ifndef ATTINYTEST_READ_H
#define ATTINYTEST_READ_H

#include "EEPROMStorage.h"
#include "ADS.h"
#include "Log.h"
#include "TimeKeeper.h"


#define READER_ADS1115_ADDRESS    0x48
#define ADS1115_READY_PIN_DEFAULT 1

enum ReadMode {
    M_VOLTAGE = 0,
    M_CURRENT = 1,
    M_WAIT = 2
};

class Reader {
public:
    Reader(EEPROMStorage &storage, Log &log, TimeKeeper &timeKeeper): storage(storage), log(log), timeKeeper(timeKeeper),
        ads(ADS1115_READY_PIN_DEFAULT) {}
    void init();
    void loop();
    void setReadInterval(uint32_t value);
    uint32_t getReadInterval() const;
    ErrorCode performRead();
    static float deserializeCurrent(const Data *data);
    static float deserializeVoltage(const Data *data);
    static void printData(const Data &data, Stream &stream);
    static void printInformCoefficients(Stream &stream);
    static void writeInformCoefficients(Stream &stream);
    static void printInformOrder(Stream &stream);
private:
    EEPROMStorage &storage;
    Log &log;
    TimeKeeper &timeKeeper;
    ADS3x ads;


    uint32_t lastRead = 0;
    uint32_t readInterval = 5000;
    ReadMode readMode = M_WAIT;
    bool modeRequested = false;
    Data currData;


    static float deserializeMilli(uint16_t relativeValue, ReadMode mode);
    static float getCoefficient(ReadMode mode);

};


#endif //ATTINYTEST_READ_H
