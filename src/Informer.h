//
// Created by valti on 09.04.2023.
//

#ifndef UPS_INFORMER_H
#define UPS_INFORMER_H

#include "Arduino.h"
#include "EEPROMStorage.h"
#include "Log.h"



enum InformFormat {
    IF_TEXT = 0,
    IF_BINARY = 1
};

static EEPROMStorage *pStorage;

class Informer {
public:
    Informer(EEPROMStorage &storage, Log &log) : storage(storage), log(log) {
        pStorage = &this->storage;
    }
    void loop();
    ErrorCode inform();
    ErrorCode proceeded();
    ErrorCode proceedError();
    void setInformInterval(uint32_t value);
    void setInformFormat(InformFormat value);
    uint32_t getInformInterval();
    InformFormat getInformFormat();
    void writeInformOrder(Stream &stream);
    void writeInformCoefficients(Stream &stream);
private:
    bool packetSent = false;
    EEPROMStorage &storage;
    Log &log;

    InformFormat informFormat = IF_TEXT;
    uint32_t informInterval = 0;
    uint32_t lastInformTime = 0;


};


#endif //UPS_INFORMER_H
