//
// Created by valti on 09.04.2023.
//

#ifndef UPS_INFORMER_H
#define UPS_INFORMER_H

#include "Arduino.h"
#include "EEPROMStorage.h"
#include "Log.h"


class Informer {
public:
    Informer(EEPROMStorage &storage, Log &log) : storage(storage), log(log) {}
    void loop();
    ErrorCode inform();
    ErrorCode proceeded();
    ErrorCode proceedError();
    void setInformInterval(uint32_t value);
    void setInformFormat(Format value);
    uint32_t getInformInterval();
    Format getInformFormat();
private:
    bool packetSent = false;
    EEPROMStorage &storage;
    Log &log;

    Format informFormat = F_TEXT;
    uint32_t informInterval = 0;
    uint32_t lastInformTime = 0;


};


#endif //UPS_INFORMER_H
