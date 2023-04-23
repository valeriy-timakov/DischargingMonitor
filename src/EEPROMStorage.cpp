//
// Created by valti on 01.12.2022.
//

#include "EEPROMStorage.h"
#include "Log.h"
#include "utils.h"
#include "read.h"


#define TIMESTAMP_OFFSET_IN_DATA (sizeof(Data) - offsetof(Data, timestamp))

const uint8_t MAX_CHECK_FAIL_COUNT = 3;
const uint8_t MAX_WRITE_FAIL_COUNT = 5;

void EEPROMStorage::loop() {
    if (currPosition > DATA_COUNT_IN_PAGE && writeTryCount < MAX_WRITE_FAIL_COUNT) {
        log.log(LB_PAGE_READY);
        writeData();
        bool dataChecked = false;
        uint8_t checkFailCount = 0;
        log.log(LB_PAGE_WRITTEN);

        while (!dataChecked && checkFailCount < MAX_CHECK_FAIL_COUNT) {
            if (checkData()) {
                dataChecked = true;
                for (uint8_t i = DATA_COUNT_IN_PAGE; i < currPosition; i++) {
                    saveBuffer[i - DATA_COUNT_IN_PAGE] = saveBuffer[i];
                }
                currPosition -= DATA_COUNT_IN_PAGE;
                writeTryCount = 0;
                log.log(LB_PAGE_CHECKED);
            } else {
                log.error(E_CHECK_FAILED_AFTER_WRITE);
                checkFailCount++;
            }
        }
        if (!dataChecked) {
            writeTryCount++;
            if (writeTryCount == MAX_WRITE_FAIL_COUNT) {
                log.error(E_WRITE_MAX_ATTEMPTS_EXCEDED);
            }
        }
    }
}


bool EEPROMStorage::isInPermissibleVariation(Data &data, CompareConfigIdx cci) {
    return abs((int32_t) avgData.avgValue(cci) - data.value(cci) ) < dataPermissibleVariation[cci];
}

void EEPROMStorage::add(Data &data) {
    log.log(LB_ADD_DATA_ENTERED);
    if (avgData.notStarted()) {
        avgData.preAdd(data);
    } else if (isInPermissibleVariation(data, DCC_CURRENT) && isInPermissibleVariation(data, DCC_VOLTAGE) && !avgData.isNearOverflow()) {
        avgData.add(data.timestamp);
        avgData.preAdd(data);
    } else {
        if (avgData.isNearOverflow()) {
            log.log(LB_ADD_OVERFLOW);
        }
        if (avgData.oneElementOnly()) {
            saveBuffer[currPosition++] = avgData.getLastData();
        } else {
            avgData.add(data.timestamp);
            saveBuffer[currPosition++] = avgData.getAvgData(true);
            saveBuffer[currPosition++] = avgData.getAvgData(false);
        }
        avgData.startNew(data);
    }
}

const Data& EEPROMStorage::getLast() {
    if (currPosition > 0) {
        return saveBuffer[currPosition - 1];
    } else {
        return UN_AVAILABLE_DATA;
    }
}

void EEPROMStorage::init() {
    EEPROM.init();
}

const uint8_t *EEPROMStorage::getBuffer() {
    return pageBuffer;
}

uint16_t EEPROMStorage::getBufferSize() {
    return PAGE_BUFFER_SIZE_BYTES;
}

void EEPROMStorage::writeData() {
    EEPROM.writeData(nextPageToSave * PAGE_BUFFER_SIZE_BYTES, saveBuffer, PAGE_BUFFER_SIZE_BYTES);
}

void inc(uint16_t &value, bool &overflow, boolean nextOverflowValue) {
    value++;
    if (value == EEPROMStorage::PAGES_COUNT) {
        value = 0;
        overflow = nextOverflowValue;
    }
}

bool EEPROMStorage::checkData() {
    bool result = EEPROM.checkData(nextPageToSave * PAGE_BUFFER_SIZE_BYTES, saveBuffer, PAGE_BUFFER_SIZE_BYTES);
    log.log(LB_CHECK_DATA_ENTERED);
    if (result) {
        inc(nextPageToSave, eepromOverflow, true);
        if (nextPageToSave == nextPageToRead) {
            inc(nextPageToRead, eepromOverflow, false);
        }
        log.log(LB_CHECK_DATA_OK);
    }
    return result;
}

bool EEPROMStorage::prepareData() {
    log.log(LB_PREPARE_DATA_ENTERED);
    if (nextPageToRead < nextPageToSave || eepromOverflow) {
        EEPROM.readData(nextPageToRead * PAGE_BUFFER_SIZE_BYTES, pageBuffer, PAGE_BUFFER_SIZE_BYTES);
        log.log(LB_PREPARE_DATA_OK);
        return true;
    }
    return false;
}

void EEPROMStorage::enumerate(void (*consume)(Data &)) {
    for (uint16_t i = 0; i < DATA_COUNT_IN_PAGE; i++) {
        consume(((Data*)pageBuffer)[i] );
    }
}

void EEPROMStorage::dataProcessed() {
    inc(nextPageToRead, eepromOverflow, false);
    log.log(LB_DATA_PROCEED_ENTERED);
}

void EEPROMStorage::setDataPermissibleVariation(CompareConfigIdx cci, uint8_t value) {
    dataPermissibleVariation[cci] = value;
}

uint8_t EEPROMStorage::getDataPermissibleVariation(CompareConfigIdx cci) {
    return dataPermissibleVariation[cci];
}

uint32_t EEPROMStorage::getLastPreparedTimestamp() {
    if (currPosition > 0) {
        return saveBuffer[currPosition - 1].timestamp;
    } else {
        return 0;
    }
}


uint32_t EEPROMStorage::getLastSavedTimestamp() {
    if (nextPageToSave > 0) {
        return EEPROM.readInt(nextPageToSave * PAGE_BUFFER_SIZE_BYTES - TIMESTAMP_OFFSET_IN_DATA);
    } else {
        return 0;
    }
}

void EEPROMStorage::printState(Stream &stream) {
    stream.print(nextPageToSave);
    stream.print(";");
    stream.print(nextPageToRead);
    stream.print(";");
    stream.print(currPosition);
    stream.print(";");
    stream.print(eepromOverflow ? 'o' : 'n');
}


bool IntergalData::notStarted() const {
    return avgPeriodStart == 0;
}

uint32_t IntergalData::getAvgPeriod() const {
    return avgPeriodEnd - avgPeriodStart;
}

uint16_t IntergalData::avgValue(CompareConfigIdx idx) const {
    uint32_t avgPeriod = getAvgPeriod();
    avgPeriod = avgPeriod == 0 ? 1 : avgPeriod;
    return (idx == DCC_CURRENT ? currentIntegralSum : voltageIntegralSum) / avgPeriod;
}

const Data& IntergalData::getLastData() const {
    return lastData;
}

bool IntergalData::oneElementOnly() const {
    return avgPeriodStart == avgPeriodEnd;
}

void IntergalData::preAdd(Data &data) {
    if (avgPeriodStart == 0) {
        voltageIntegralSum = data.voltage;
        currentIntegralSum = data.current;
        avgPeriodStart = data.timestamp;
        avgPeriodEnd = data.timestamp;
    } else if (avgPeriodEnd != data.timestamp) {
        log.error(E_ROUNDING_DATA_ADD_WRONG_TIME);
    }
    lastData = data;
}

void IntergalData::add(uint32_t lastTimestamp) {
    uint32_t duration = lastTimestamp - avgPeriodEnd;
    uint32_t newLastAddedVoltage = lastData.voltage * duration;
    uint32_t newLastAddedCurrent = lastData.current * duration;
    kV = max(kV, ceil((float) newLastAddedVoltage / lastAddedVoltage));
    kC = max(kC, ceil((float) newLastAddedCurrent / lastAddedCurrent));
    lastAddedVoltage = newLastAddedVoltage;
    lastAddedCurrent = newLastAddedCurrent;
    voltageIntegralSum += newLastAddedVoltage;
    currentIntegralSum += newLastAddedCurrent;
    avgPeriodEnd = lastTimestamp;
}

void IntergalData::startNew(Data &data) {
    avgPeriodStart = 0;
    avgPeriodEnd = 0;
    preAdd(data);
}

Data IntergalData::getAvgData(bool start) const {
    uint32_t periodDuration = getAvgPeriod();
    return Data {
            (uint16_t) (voltageIntegralSum / periodDuration),
            (uint16_t) (currentIntegralSum / periodDuration),
            start ? avgPeriodStart : avgPeriodEnd
    };
}

bool IntergalData::isNearOverflow() const {
    return MAX_U32_VALUE - lastAddedVoltage * kV < voltageIntegralSum ||
        MAX_U32_VALUE - lastAddedCurrent * kC < currentIntegralSum;
}
