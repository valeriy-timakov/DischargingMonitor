//
// Created by valti on 01.12.2022.
//

#ifndef UPS_EEPROMSTORAGE_H
#define UPS_EEPROMSTORAGE_H


#include "Arduino.h"
#include "data.h"
#include "EEPROM25LC1024.h"
#include "AbstractStorage.h"




class EEPROMStorage {

private:
    EEPROM25LC1024 EEPROM;

    static const uint8_t  SAVE_BUFFER_CAPACITY = 40;
    uint8_t saveBufferSize = 20;
    Data saveBuffer[SAVE_BUFFER_CAPACITY];
    uint8_t currPosition = 0;
    uint8_t writeTryCount = 0;

    static const uint16_t PAGE_BUFFER_SIZE_BYTES = 256;
    uint8_t pageBuffer[PAGE_BUFFER_SIZE_BYTES];
    uint8_t nextPageToSave = 0;
    uint8_t nextPageToRead = 0;
    bool eepromOverflow = false;


    bool writeData();
    bool checkData();

public:
    void loop();
    void add(Data &currData);
    const Data& getLast();
    void enumerate(void (*consume)(Data &));


    static const uint16_t PAGES_COUNT = 512;
    EEPROMStorage(uint8_t csPin) : saveBufferSize(PAGE_BUFFER_SIZE_BYTES / sizeof (Data)), EEPROM(EEPROM25LC1024(csPin)) {}
    void init();
    const uint8_t *getBuffer();
    uint16_t getBufferSize();
    bool prepareData();
    void dataProcessed();

};




#endif //UPS_EEPROMSTORAGE_H
