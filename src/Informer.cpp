//
// Created by valti on 09.04.2023.
//

#include "Informer.h"
#include "read.h"

Stream *pStream;


void write(Data &currData) {
    pStream->print(Reader::deserializeVoltage(&currData), 3);
    pStream->print(',');
    pStream->print(Reader::deserializeCurrent(&currData), 3);
    pStream->print(',');
    pStream->print(currData.timestamp);
    pStream->print(';');
}

void Informer::writeInformOrder(Stream &stream) {
    stream.print("v,c,t;");
}

void Informer::writeInformCoefficients(Stream &stream) {
    stream.print(Reader::getCoefficient(M_VOLTAGE), 5);
    stream.print(",");
    stream.print(Reader::getCoefficient(M_CURRENT), 5);
}

void writeInform(Stream &stream) {
    pStream = &stream;
    pStorage->enumerate(write);
}

void Informer::loop(Sender &communicator) {

    if (informInterval > 0 && (millis() - lastInformTime) >= informInterval) {
        Serial.println("Tti");
        inform(communicator);
        Serial.println("Nmdl");
        lastInformTime = millis();
    }

}

ErrorCode Informer::inform(Sender &communicator) {
    if (packetSent) {
        return E_INFORM_PACKAGE_ALREADY_SENT;
    }
    if (storage.prepareData()) {
        if (informFormat == IF_BINARY) {
            communicator.sendBinary(storage.getBuffer(), storage.getBufferSize());
        } else {
            communicator.sendData('i', writeInform);
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

void Informer::writeInformInterval(Stream &stream) {
    stream.print(informInterval);
}

void Informer::writeInformFormat(Stream &stream) {
    stream.print(informFormat == IF_BINARY ? "bin" : "txt");
}

ErrorCode Informer::setInformInterval(uint32_t value) {
    informInterval = value;
    return OK;
}

ErrorCode Informer::setInformFormat(uint32_t value) {
    informFormat = value > 0 ? IF_BINARY : IF_TEXT;
    return OK;
}


