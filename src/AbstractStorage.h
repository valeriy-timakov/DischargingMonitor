//
// Created by valti on 16.12.2022.
//

#ifndef UPS_ABSTRACTSTORAGE_H
#define UPS_ABSTRACTSTORAGE_H
/*
#include "Arduino.h"
#include "data.h"

class AbstractStorage {
protected:
    static const uint8_t  SAVE_BUFFER_CAPACITY = 50;
    uint8_t saveBufferSize = 20;
    Data saveBuffer[SAVE_BUFFER_CAPACITY];
    uint8_t currPosition = 0;
    uint8_t writeTryCount = 0;

    virtual bool writeData() = 0;
    virtual bool checkData() = 0;

public:
    AbstractStorage(uint8_t savesBufferSize = 20) : saveBufferSize(savesBufferSize) {}
    virtual void init() = 0;
    void loop();
    void add(Data &currData);
    const Data& getLast();
    void enumerate(void (*consume)(Data &));
    virtual const uint8_t *getBuffer() = 0;
    virtual uint16_t getBufferSize() = 0;
    virtual bool prepareData() = 0;
    virtual void dataProcessed() = 0;

};

*/
#endif //UPS_ABSTRACTSTORAGE_H
