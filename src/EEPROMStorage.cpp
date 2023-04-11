//
// Created by valti on 01.12.2022.
//

#include "EEPROMStorage.h"
#include "Log.h"
#include "utils.h"


const uint8_t MAX_CHECK_FAIL_COUNT = 3;
const uint8_t MAX_WRITE_FAIL_COUNT = 5;


void EEPROMStorage::loop() {
    if (currPosition > saveBufferSize && writeTryCount < MAX_WRITE_FAIL_COUNT) {
        Serial.println("Sowd");
        if (writeData()) {
            Serial.println("Dwc:");
            bool dataChecked = false;
            uint8_t checkFailCount = 0;
            while (!dataChecked && checkFailCount < MAX_CHECK_FAIL_COUNT) {
                delay(10);
                if (checkData()) {
                    dataChecked = true;
                    for (uint8_t i = saveBufferSize; i < currPosition; i++) {
                        saveBuffer[i - saveBufferSize] = saveBuffer[i];
                    }
                    currPosition -= saveBufferSize;
                    writeTryCount = 0;
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
    saveBuffer[currPosition] = data;
    currPosition++;
    Serial.print("+");
    Serial.println(currPosition);
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
    }
    return result;
}

bool EEPROMStorage::prepareData() {
    Serial.print("pD:");
    Serial.print(nextPageToSave);
    Serial.print("/");
    Serial.print(nextPageToRead);
    Serial.print("/");
    Serial.println(eepromOverflow? "y": "n");
    Serial.println((uint32_t)this);
    delay(300);
    if (nextPageToRead < nextPageToSave || eepromOverflow) {
        EEPROM.readData(nextPageToRead * PAGE_BUFFER_SIZE_BYTES, pageBuffer, PAGE_BUFFER_SIZE_BYTES);
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
}

