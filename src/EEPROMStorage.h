//
// Created by valti on 01.12.2022.
//

#ifndef UPS_EEPROMSTORAGE_H
#define UPS_EEPROMSTORAGE_H


#include "Arduino.h"
#include "data.h"
#include "EEPROM25LC1024.h"
#include "Log.h"

struct IntergalData {
private:
    uint32_t avgPeriodStart = 0;
    uint32_t avgPeriodEnd = 0;
    uint32_t voltageIntegralSum = 0;
    uint32_t currentIntegralSum = 0;
    uint32_t lastAddedCurrent = 0;
    uint32_t lastAddedVoltage = 0;
    uint8_t kV = 1;
    uint8_t kC = 1;
    Data lastData;
public:

    bool notStarted() const;
    uint32_t getAvgPeriod() const;
    uint16_t avgValue(CompareConfigIdx idx) const;
    const Data &getLastData() const;
    bool oneElementOnly() const;
    Data getAvgData(bool start) const;
    void preAdd(Data &data);
    void add(uint32_t lastTimestamp);
    void startNew(Data &data);
    bool isNearOverflow() const;
};

class EEPROMStorage {

private:
    EEPROM25LC1024 EEPROM;
    Log &log;
    static const uint16_t PAGE_BUFFER_SIZE_BYTES = 256;
    static const uint8_t DATA_COUNT_IN_PAGE = PAGE_BUFFER_SIZE_BYTES / sizeof (Data);
    static const uint8_t SAVE_BUFFER_CAPACITY = 40;
 //   static const uint8_t COMPARE_BUFFER_CAPACITY = 20;
    Data saveBuffer[SAVE_BUFFER_CAPACITY];
 //   Data compareBuffer[COMPARE_BUFFER_CAPACITY];
    uint8_t currPosition = 0;
    uint8_t writeTryCount = 0;

    uint8_t pageBuffer[PAGE_BUFFER_SIZE_BYTES];
    uint16_t nextPageToSave = 0;
    uint16_t nextPageToRead = 0;
   // uint8_t currComparePosition = 0;
    uint8_t dataPermissibleVariation[2];
    bool eepromOverflow = false;



//    uint32_t voltageIntegralSum = 0;
//    uint32_t currentIntegralSum = 0;
//    uint32_t avgPeriodStart = 0;
//    uint32_t avgPeriodEnd = 0;
    IntergalData avgData;


    //Data avgData = Data {0, 0, 0};
    //uint32_t lastTimestamp = 0;

    void writeData();
    bool checkData();
   // int32_t getAverage(uint16_t (*extract)(Data&));
    //bool isInPermissibleVariation2(Data &data, CompareConfigIdx cci);
    bool isInPermissibleVariation(Data &data, CompareConfigIdx cci);

public:
    void loop();
    void add(Data &currData);
    const Data& getLast();
    void enumerate(void (*consume)(Data &));


    static const uint16_t PAGES_COUNT = 512;
    EEPROMStorage(uint8_t csPin, Log &log) : EEPROM(EEPROM25LC1024(csPin)), log(log) {
        for (uint8_t & i : dataPermissibleVariation) i = 0;
        for (auto & i : saveBuffer) i = Data {0, 0, 0};
        for (uint8_t & i : pageBuffer) i = 0;
    }
    void init();
    const uint8_t *getBuffer();
    static uint16_t getBufferSize();
    bool prepareData();
    void dataProcessed();
    void setDataPermissibleVariation(CompareConfigIdx cci, uint8_t value);
    uint8_t getDataPermissibleVariation(CompareConfigIdx cci);
    uint32_t getLastPreparedTimestamp();
    uint32_t getLastSavedTimestamp();
};


#endif //UPS_EEPROMSTORAGE_H


