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

void writeInform(Stream &stream) {
    pStream = &stream;
    pStorage->enumerate(write);
}

void Informer::writeInformOrder(Stream &stream) {
    stream.print("v,c,t;");
}

void Informer::writeInformCoefficients(Stream &stream) {
    stream.print(Reader::getCoefficient(M_VOLTAGE), 5);
    stream.print(",");
    stream.print(Reader::getCoefficient(M_CURRENT), 5);
}

void Informer::loop() {
    if (informInterval > 0 && (millis() - lastInformTime) >= informInterval) {
        log.log(LB_INFORM_STARTED);
        Serial.println("Tti");
        inform();
        Serial.println("Nmdl");
        lastInformTime = millis();
    }
}

ErrorCode Informer::inform() {
    if (packetSent) {
        return E_INFORM_PACKAGE_ALREADY_SENT;
    }
    if (storage.prepareData()) {
        if (informFormat == IF_BINARY) {
            const uint8_t* data = storage.getBuffer();
            uint16_t bytesCount = storage.getBufferSize();
            uint16_t hash = 0;
            uint16_t i = 0;
            for (; i < bytesCount - 1; i += 2) {
                hash += data[i + 1] + ( data[i] << 8 );
            }
            if (i < bytesCount) {
                hash += data[i] << 8;
            }
            Serial.write(0);
            Serial.write(0);
            Serial.write(0);
            Serial.write(bytesCount);
            Serial.write(hash);
            Serial.write(data, bytesCount);
            Serial.write(0);
            Serial.write(0);
            Serial.write(0);
            Serial.write(1);
        } else {
            Serial.print("(");
            Serial.print('I');
            writeInform(Serial);
            Serial.print(")");
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

InformFormat Informer::getInformFormat() {
    return informFormat;
}

void Informer::setInformInterval(uint32_t value) {
    informInterval = value;
}

void Informer::setInformFormat(InformFormat value) {
    informFormat = value;
}


