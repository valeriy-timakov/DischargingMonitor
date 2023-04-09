//
// Created by valti on 03.08.2022.
//

#pragma once

#ifndef ATTINYTEST_READ_H
#define ATTINYTEST_READ_H

#include "Communicator.h"
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
    void processInstruction(Instruction instruction, Communicator &communicator);
    static float deserializeMilli(uint16_t relativeValue, ReadMode mode);
    const Data& getLastData();
private:
    EEPROMStorage &storage;

    static float deserializeCurrent(Data &data);
    static float deserializeVoltage(Data &data);

};


#endif //ATTINYTEST_READ_H
