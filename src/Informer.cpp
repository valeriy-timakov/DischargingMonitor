//
// Created by valti on 09.04.2023.
//

#include "Informer.h"


void Informer::loop() {
    if (informInterval > 0 && ( (millis() - lastInformTime) >= informInterval ) && !packetSent) {
        inform();
        log.log(LB_INFORM_STARTED);
        lastInformTime = millis();
    }
}

ErrorCode Informer::inform() {
    if (packetSent) {
        return E_INFORM_PACKAGE_ALREADY_SENT;
    }
    if (storage.prepareData()) {
        if (informFormat == F_TEXT) {
            storage.printNextSavedDataPage(Serial, 'I');
        } else {
            storage.writeNextSavedDataPage(Serial, IDC_INFORM);
        }
        packetSent = true;
        return OK;
    }
    return E_INFORM_NO_DATA_TO_SEND;
}

ErrorCode Informer::proceeded() {
    if (packetSent) {
        storage.dataProcessed();
        packetSent = false;
        return OK;
    }
    return E_NO_PACKAGE_WAS_SENT;
}

ErrorCode Informer::proceedError() {
    if (packetSent) {
        packetSent = false;
        return OK;
    }
    return E_NO_PACKAGE_WAS_SENT;
}
uint32_t Informer::getInformInterval() {
    return informInterval;
}

Format Informer::getInformFormat() {
    return informFormat;
}

void Informer::setInformInterval(uint32_t value) {
    informInterval = value;
}

void Informer::setInformFormat(Format value) {
    informFormat = value;
}

