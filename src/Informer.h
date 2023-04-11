//
// Created by valti on 09.04.2023.
//

#ifndef UPS_INFORMER_H
#define UPS_INFORMER_H

#include "Arduino.h"
#include "EEPROMStorage.h"



enum InformFormat {
    IF_TEXT = 0,
    IF_BINARY = 1
};

static EEPROMStorage *pStorage;

class Informer {
public:
    Informer(EEPROMStorage &storage) : storage(storage) {
        pStorage = &this->storage;
    }
    void loop();
    ErrorCode inform();
    ErrorCode proceeded();
    ErrorCode proceedError();
    ErrorCode setInformInterval(uint32_t value);
    ErrorCode setInformFormat(uint32_t value);
    void writeInformInterval(Stream &stream);
    void writeInformFormat(Stream &stream);
    void writeInformOrder(Stream &stream);
    void writeInformCoefficients(Stream &stream);
private:
    bool packetSent = false;
    EEPROMStorage &storage;

    InformFormat informFormat = IF_TEXT;
    uint32_t informInterval = 0;
    uint32_t lastInformTime = 0;


};


#endif //UPS_INFORMER_H
