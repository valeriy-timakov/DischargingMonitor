//
// Created by valti on 01.12.2022.
//

#ifndef UPS_EEPROMSTORAGE_H
#define UPS_EEPROMSTORAGE_H


#include "Arduino.h"
#include "data.h"
#include "EEPROM25LC1024.h"
#include "Log.h"

enum CompareConfigIdx {
    DCC_CURRENT = 0,
    DCC_VOLTAGE = 1
};

class EEPROMStorage {

private:
    EEPROM25LC1024 EEPROM;
    uint8_t saveBufferSize;
    Log &log;


    static const uint8_t SAVE_BUFFER_CAPACITY = 40;
    static const uint8_t COMPARE_BUFFER_CAPACITY = 20;
    Data saveBuffer[SAVE_BUFFER_CAPACITY];
    Data compareBuffer[COMPARE_BUFFER_CAPACITY];
    uint8_t currPosition = 0;
    uint8_t writeTryCount = 0;

    static const uint16_t PAGE_BUFFER_SIZE_BYTES = 256;
    uint8_t pageBuffer[PAGE_BUFFER_SIZE_BYTES];
    uint8_t nextPageToSave = 0;
    uint8_t nextPageToRead = 0;
    uint8_t currComparePosition = 0;
    uint8_t dataPermissibleVariation[2];
    bool eepromOverflow = false;

    bool writeData();
    bool checkData();
    int32_t getAverage(uint16_t (*extract)(Data&));
    bool isInPermissibleVariation(Data &data, CompareConfigIdx cci);

public:
    void loop();
    void add(Data &currData);
    const Data& getLast();
    void enumerate(void (*consume)(Data &));


    static const uint16_t PAGES_COUNT = 512;
    EEPROMStorage(uint8_t csPin, Log &log) : EEPROM(EEPROM25LC1024(csPin)), saveBufferSize(PAGE_BUFFER_SIZE_BYTES / sizeof (Data)), log(log) {}
    void init();
    const uint8_t *getBuffer();
    uint16_t getBufferSize();
    bool prepareData();
    void dataProcessed();
    void setDataPermissibleVariation(CompareConfigIdx cci, uint8_t value);
    uint8_t getDataPermissibleVariation(CompareConfigIdx cci);
    uint32_t getLastPreparedTimestamp();
    uint32_t getLastSavedTimestamp();
};




#endif //UPS_EEPROMSTORAGE_H
