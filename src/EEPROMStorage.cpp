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
        Serial.println("Sowd");
        writeData();
        /*
        for (uint8_t i = 0; i < DATA_COUNT_IN_PAGE; i++) {
            dbgData(saveBuffer[i]);
        }
         */
        Serial.println("Dwc:");
        bool dataChecked = false;
        uint8_t checkFailCount = 0;
        log.log(LB_PAGE_WRITTEN);

/*
        delay(WRITEDELAY);
        EEPROM.readData(nextPageToSave * PAGE_BUFFER_SIZE_BYTES, pageBuffer, PAGE_BUFFER_SIZE_BYTES);
        Data *pb = (Data *) pageBuffer;
        for (uint8_t i = 0; i < DATA_COUNT_IN_PAGE; i++) {
            dbgData( pb[i]);
        }
        */
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
                checkFailCount++;
                Serial.println("Ecd!");
            }
        }
        if (!dataChecked) {
            writeTryCount++;
        }
    }
}


bool EEPROMStorage::isInPermissibleVariation(Data &data, CompareConfigIdx cci) {
    /*
    Serial.print("d");
    Serial.print(cci == DCC_VOLTAGE ? "V" : "C");
    Serial.print("=");
    Serial.print(avgData.avgValue(cci));
    Serial.print("-");
    Serial.print(data.value(cci));
    Serial.print("=");
    Serial.println((int32_t) avgData.avgValue(cci) - data.value(cci) );*/
    return abs((int32_t) avgData.avgValue(cci) - data.value(cci) ) < dataPermissibleVariation[cci];
}

uint32_t  dbgSum = 0;
uint16_t dbgCount = 0;

void EEPROMStorage::add(Data &data) {
    log.log(LB_ADD_DATA_ENTERED);
    Serial.print("+");
  //  dbgData(data);
    if (avgData.notStarted()) {
        avgData.preAdd(data);
        dbgSum = data.voltage;
        dbgCount = 1;
     //   Serial.println(0);
    } else if (isInPermissibleVariation(data, DCC_CURRENT) && isInPermissibleVariation(data, DCC_VOLTAGE) && !avgData.isNearOverflow()) {
        avgData.add(data.timestamp);
        avgData.preAdd(data);
        dbgSum += data.voltage;
        dbgCount++;
 /*       Serial.print(dbgCount);
        Serial.print("/");
        Serial.print(data.voltage);
        Serial.print("/");
        Serial.print(dbgSum / dbgCount);
        Serial.print("-");
        Serial.print(avgData.avgValue(DCC_VOLTAGE));
        Serial.print("=");
        Serial.println(dbgSum / dbgCount - avgData.avgValue(DCC_VOLTAGE));*/
    } else {
        if (avgData.isNearOverflow()) {
            Serial.println("NO!Rst");
            log.log(LB_ADD_OVERFLOW);
        }
        dbgCount = 1;
        dbgSum = data.voltage;
        Serial.println();
        Serial.print("+! ");
        Serial.println(currPosition);
        if (avgData.oneElementOnly()) {
            saveBuffer[currPosition++] = avgData.getLastData();
        } else {
            avgData.add(data.timestamp);
            saveBuffer[currPosition++] = avgData.getAvgData(true);
            saveBuffer[currPosition++] = avgData.getAvgData(false);
        }
        dbgData(saveBuffer[currPosition - 1]);
        avgData.startNew(data);
    }


/*
    bool savePrevious;
    if (currPosition == 0) {
        savePrevious = true;
    } else if (currComparePosition == 0) {
        savePrevious = false;
    } else if (currComparePosition == COMPARE_BUFFER_CAPACITY) {
        savePrevious = true;
    } else {
        uint16_t (*extract)(Data&) = extractors[DCC_CURRENT];
        if (abs(getAverage(extract) - extract(data)) < dataPermissibleVariation[DCC_CURRENT]) {
            extract = extractors[DCC_VOLTAGE];
            if (abs(getAverage(extract) - extract(data)) < dataPermissibleVariation[DCC_VOLTAGE]) {
                savePrevious = false;
            } else {
                savePrevious = true;
                log.log(LB_ADD_VOLTAGE_OUT_OF_PERMISSIBLE_VARIATION);
            }
        } else {
            savePrevious = true;
            log.log(LB_ADD_CURRENT_OUT_OF_PERMISSIBLE_VARIATION);
        }
    }
    if (savePrevious) {
        Data avgData;
        avgData.timestamp = compareBuffer[currComparePosition - 1].timestamp;
        avgData.voltage = getAverage(extractors[DCC_VOLTAGE]);
        avgData.current = getAverage(extractors[DCC_CURRENT]);
        saveBuffer[currPosition] = avgData;
        currComparePosition = 0;
        currPosition++;
        log.log(LB_ADD_DATA_PREVIOUS_SAVED);
        Serial.print("+! ");
        Serial.println(currPosition);
    }
    compareBuffer[currComparePosition++] = data;
    Serial.print("+");
    Serial.print(currComparePosition);
    */

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
        Serial.println("wOk");
        inc(nextPageToSave, eepromOverflow, true);
        if (nextPageToSave == nextPageToRead) {
            inc(nextPageToRead, eepromOverflow, false);
        }
        Serial.print("cD:");
        Serial.print(nextPageToSave);
        Serial.print("/");
        Serial.println(nextPageToRead );
        Serial.print("/");
        Serial.println(eepromOverflow? "y": "n");
        log.log(LB_CHECK_DATA_OK);
    }
    return result;
}

bool EEPROMStorage::prepareData() {
    log.log(LB_PREPARE_DATA_ENTERED);
    Serial.print("pD:");
    Serial.print(nextPageToSave);
    Serial.print("/");
    Serial.print(nextPageToRead);
    Serial.print("/");
    Serial.print(eepromOverflow? "y": "n");
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
    Serial.print("dP:");
    Serial.print(nextPageToSave);
    Serial.print("/");
    Serial.print(nextPageToRead);
    Serial.print("/");
    Serial.println(eepromOverflow? "y": "n");
    log.log(LB_DATA_PROCEED_ENTERED);
}
/*
int32_t EEPROMStorage::getAverage(uint16_t (*extract)(Data&)) {
    if (currComparePosition == 0) {
        return -1;
    }
    uint32_t sum = 0;
    for (uint8_t i = 0; i < currComparePosition; i++) {
        sum += extract(compareBuffer[i]);
    }
    return sum / currComparePosition;
}


bool EEPROMStorage::isInPermissibleVariation2(Data &data, CompareConfigIdx cci) {
    if (currComparePosition == 0) {
        return true;
    }

    uint16_t (*extract)(Data&) = extractors[cci];
    return getAverage(extract) - extract(data) < dataPermissibleVariation[cci];
}
*/
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
        Serial.println("preAdd initial");
        voltageIntegralSum = data.voltage;
        currentIntegralSum = data.current;
        avgPeriodStart = data.timestamp;
        avgPeriodEnd = data.timestamp;
    } else {
        if (avgPeriodEnd != data.timestamp) {
            Serial.print("Err: ");
            Serial.print(avgPeriodEnd);
            Serial.print("!=");
            Serial.print(data.timestamp);
        }
    }
    lastData = data;
    Serial.print("pis:(");
    Serial.print(voltageIntegralSum);
    Serial.print(";");
    Serial.print(currentIntegralSum);
    Serial.print(";");
    Serial.print(avgPeriodEnd - avgPeriodStart);
    Serial.println(")");
}

void IntergalData::add(uint32_t lastTimestamp) {
    uint32_t duration = lastTimestamp - avgPeriodEnd;
    Serial.print("+ds:(");
    Serial.print(lastData.voltage * duration);
    Serial.print(";");
    Serial.print(lastData.current * duration);
    Serial.println(")");
    uint32_t newLastAddedVoltage = lastData.voltage * duration;
    uint32_t newLastAddedCurrent = lastData.current * duration;
    kV = max(kV, ceil((float) newLastAddedVoltage / lastAddedVoltage));
    kC = max(kC, ceil((float) newLastAddedCurrent / lastAddedCurrent));
    lastAddedVoltage = newLastAddedVoltage;
    lastAddedCurrent = newLastAddedCurrent;
    voltageIntegralSum += newLastAddedVoltage;
    currentIntegralSum += newLastAddedCurrent;
    avgPeriodEnd = lastTimestamp;
    Serial.print("ais:(");
    Serial.print(voltageIntegralSum);
    Serial.print(";");
    Serial.print(currentIntegralSum);
    Serial.print(";");
    Serial.print(avgPeriodEnd - avgPeriodStart);
    Serial.println(")");
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

const uint32_t MAX_U32_VALUE = 0xFFFFFFFF;

bool IntergalData::isNearOverflow() const {
    return MAX_U32_VALUE - lastAddedVoltage * kV < voltageIntegralSum ||
        MAX_U32_VALUE - lastAddedCurrent * kC < currentIntegralSum;
}
