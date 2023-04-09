//
// Created by valti on 04.12.2022.
//

#ifndef UPS_FLASHSTORAGE_H
#define UPS_FLASHSTORAGE_H
/*
#include "Arduino.h"
#include "data.h"
#include "AbstractStorage.h"


class FlashStorage : public AbstractStorage {
protected:
    static const uint8_t  PAGE_BUFFER_SIZE_BYTES = 128;
    uint8_t pageBuffer[PAGE_BUFFER_SIZE_BYTES];
    uint8_t csPin;//const int chipSelectSD = 9;


    bool writeData() override;
    bool checkData() override;

public:
    FlashStorage(uint8_t csPin) : AbstractStorage(PAGE_BUFFER_SIZE_BYTES / sizeof (Data)), csPin(csPin) {}
    void init() override;
    const uint8_t *getBuffer() override;
    uint16_t getBufferSize() override;
    bool prepareData() override;
    void dataProcessed() override;

};

*/
#endif //UPS_FLASHSTORAGE_H
