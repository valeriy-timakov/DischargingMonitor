//
// Created by valti on 01.12.2022.
//

#include "EEPROMStorage.h"
#include "Log.h"

#define TIMESTAMP_OFFSET_IN_DATA sizeof(Data) - offsetof(Data, timestamp)

const uint8_t MAX_CHECK_FAIL_COUNT = 3;
const uint8_t MAX_WRITE_FAIL_COUNT = 5;


static uint16_t (*extractors [2])(Data&) = {
        [](Data &data) { return data.voltage; },
        [](Data &data) { return data.current; }
};


void EEPROMStorage::loop() {
    if (currPosition > saveBufferSize && writeTryCount < MAX_WRITE_FAIL_COUNT) {
        log.log(LB_PAGE_READY);
        Serial.println("Sowd");
        if (writeData()) {
            Serial.println("Dwc:");
            bool dataChecked = false;
            uint8_t checkFailCount = 0;
            log.log(LB_PAGE_WRITTEN);
            while (!dataChecked && checkFailCount < MAX_CHECK_FAIL_COUNT) {
                delay(WRITEDELAY);
                if (checkData()) {
                    dataChecked = true;
                    for (uint8_t i = saveBufferSize; i < currPosition; i++) {
                        saveBuffer[i - saveBufferSize] = saveBuffer[i];
                    }
                    currPosition -= saveBufferSize;
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
}



void EEPROMStorage::add(Data &data) {
    log.log(LB_ADD_DATA_ENTERED);
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
}

const Data& EEPROMStorage::getLast() {
    if (currPosition > 0) {
        return saveBuffer[currPosition - 1];
    } else {
        return UN_AVAILABLE_DATA;
    }
}

void EEPROMStorage::enumerate(void (*consume)(Data &)) {
    uint8_t count = getBufferSize() / sizeof (Data);
    for (uint16_t i = 0; i < count; i++) {
        consume(((Data*)this->getBuffer())[i] );
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

bool EEPROMStorage::writeData() {
    EEPROM.writeData(nextPageToSave * PAGE_BUFFER_SIZE_BYTES, saveBuffer, PAGE_BUFFER_SIZE_BYTES);
    return true;
}

void inc(uint8_t &value, bool &overflow, boolean nextOverflowValue) {
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
    Serial.println(eepromOverflow? "y": "n");
    Serial.println((uint32_t)this);
    if (nextPageToRead < nextPageToSave || eepromOverflow) {
        EEPROM.readData(nextPageToRead * PAGE_BUFFER_SIZE_BYTES, pageBuffer, PAGE_BUFFER_SIZE_BYTES);
        log.log(LB_PREPARE_DATA_OK);
        return true;
    }
    return false;
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


bool EEPROMStorage::isInPermissibleVariation(Data &data, CompareConfigIdx cci) {
    if (currComparePosition == 0) {
        return true;
    }

    uint16_t (*extract)(Data&) = extractors[cci];
    return getAverage(extract) - extract(data) < dataPermissibleVariation[cci];
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

