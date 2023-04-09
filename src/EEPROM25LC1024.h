//
// Created by valti on 23.10.2022.
//

#ifndef UPS_EEPROM25LC1024_H
#define UPS_EEPROM25LC1024_H

#include <Arduino.h>
#include <SPI.h>


class EEPROM25LC1024 {
public:
    EEPROM25LC1024(uint8_t csPin = 10) : csPin(csPin) {}
    void init() const;
    bool writeComplete() const;
    void writeData(uint32_t address, void *data, size_t count);
    void readData(uint32_t address, void *data, size_t count);
    bool checkData(uint32_t address, void *data, size_t count);
    void writeByte(uint32_t address, uint8_t data);
    uint8_t readByte(uint32_t address);
    void writeChar(uint32_t address, uint16_t data);
    uint16_t readChar(uint32_t address);
    void writeInt(uint32_t address, uint32_t data);
    uint32_t readInt(uint32_t address);
    void writeLong(uint32_t address, uint64_t data);
    uint64_t readLong(uint32_t address);
    void writeFloat(uint32_t address, float data);
    float readFloat(uint32_t address);
private:
    uint8_t csPin;
    uint64_t lastWrite;

    void checkDelayAfterWrite() const;
    void beginWrite(uint32_t address) const;
    void beginRead(uint32_t address) const;
    void endRead() const;
    void endWrite();


};


#endif //UPS_EEPROM25LC1024_H
