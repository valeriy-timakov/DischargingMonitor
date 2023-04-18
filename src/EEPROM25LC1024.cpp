//
// Created by valti on 23.10.2022.
// initially based on https://github.com/dndubins/EEPROMsimple but completely rewritten later
//

#include "EEPROM25LC1024.h"

#define DEBUG

#define WRITE 0b0010
#define READ  0b0011
#define WREN  0b0110
#define RDSR  0b0101
#define WIP_BIT 0


SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

void EEPROM25LC1024::init() const {
    pinMode(csPin, OUTPUT);
    SPIClass::begin();
}

bool EEPROM25LC1024::writeComplete() const {
    SPIClass::beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
    SPIClass::transfer(RDSR);
    uint8_t data = SPIClass::transfer(0x00);
    endRead();
    return (data & _BV(WIP_BIT)) == 0;
}

static void SPIwrite(void *buf, size_t count);

void EEPROM25LC1024::writeData(uint32_t address, void *data, size_t count){
    beginWrite(address);
    SPIwrite(data, count);
    endWrite();
}

void EEPROM25LC1024::readData(uint32_t address, void *data, size_t count){
    beginRead(address);
    SPIClass::transfer(data, count);
    endRead();
}

bool EEPROM25LC1024::checkData(uint32_t address, void *data, size_t count) {
    beginRead(address);
    bool result = true;
    auto *pData = (uint8_t *)data;
    for (uint16_t i = 0; i < count; i++) {
        auto saved =  SPIClass::transfer(0x00);
        if (*pData != saved) {
            if (*pData > 127) {
                endRead();
                delay(1);
                beginRead(address + i);
                saved =  SPIClass::transfer(0x00);
                if (*pData != saved) {
                    result = false;
                    break;
                }
            }
        }
        pData++;
    }
    endRead();
    return result;
}

void EEPROM25LC1024::writeByte(uint32_t address, uint8_t data) {
    beginWrite(address);
    SPIClass::transfer(data);
    endWrite();
}

uint8_t EEPROM25LC1024::readByte(uint32_t address) {
    beginRead(address);
    uint8_t data = SPIClass::transfer(0x00);
    endRead();
    return data;
}

void EEPROM25LC1024::writeChar(uint32_t address, uint16_t data) {
    beginWrite(address);
    SPIwrite(&data, sizeof data);
    endWrite();
}

uint16_t EEPROM25LC1024::readChar(uint32_t address) {
    uint16_t result;
    beginRead(address);
    SPIClass::transfer(&result, sizeof result);
    endRead();
    return result;
}

void EEPROM25LC1024::writeInt(uint32_t address, uint32_t data){
    beginWrite(address);
    SPIwrite(&data, sizeof data);
    endWrite();
}

uint32_t EEPROM25LC1024::readInt(uint32_t address){
    uint32_t result;
    beginRead(address);
    SPIClass::transfer(&result, sizeof result);
    endRead();
    return result;
}

void EEPROM25LC1024::writeLong(uint32_t address, uint64_t data){
    beginWrite(address);
    SPIwrite(&data, sizeof data);
    endWrite();
}

uint64_t EEPROM25LC1024::readLong(uint32_t address){
    uint64_t result;
    beginRead(address);
    SPIClass::transfer(&result, sizeof result);
    endRead();
    return result;
}

void EEPROM25LC1024::writeFloat(uint32_t address, float data){
    beginWrite(address);
    SPIwrite(&data, sizeof data);
    endWrite();
}

float EEPROM25LC1024::readFloat(uint32_t address){
    float result;
    beginRead(address);
    SPIClass::transfer(&result, sizeof result);
    endRead();
    return result;
}

void EEPROM25LC1024::checkDelayAfterWrite() const {
    uint32_t afterLastWrite = millis() - lastWrite;
    if (afterLastWrite < WRITEDELAY) {
        if (!writeComplete()) {
            delay(WRITEDELAY - afterLastWrite + 2);
        }
    }
}

//modified copy of Arduino void SPI.transfer(void *buf, size_t count)
inline static void SPIwrite(void *buf, size_t count) {
    if (count == 0) return;
    auto *p = (uint8_t *) buf;
    SPDR = *p;
    while (--count > 0) {
        uint8_t out = *(p + 1);
        while (!(SPSR & _BV(SPIF)));
        SPDR = out;
        p++;
    }
    while (!(SPSR & _BV(SPIF))) ;
}

inline void writeAddress(uint32_t address) {
    SPIClass::transfer((byte)(address >> 16));
    SPIClass::transfer((byte)(address >> 8));
    SPIClass::transfer((byte)address);
}

inline void EEPROM25LC1024::beginWrite(uint32_t address) const {
    checkDelayAfterWrite();
    SPIClass::beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
    SPIClass::transfer(WREN);
    digitalWrite(csPin, HIGH);
    digitalWrite(csPin, LOW);
    SPIClass::transfer(WRITE);
    writeAddress(address);
}

inline void EEPROM25LC1024::beginRead(uint32_t address) const {
    checkDelayAfterWrite();
    SPIClass::beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
    SPIClass::transfer(READ);
    writeAddress(address);
}

inline void EEPROM25LC1024::endRead() const {
    digitalWrite(csPin, HIGH);
    SPIClass::endTransaction();
}

inline void EEPROM25LC1024::endWrite() {
    endRead();
    lastWrite = millis();
}