//
// Created by valti on 16.12.2022.
//

#include "AbstractStorage.h"
/*
const uint8_t MAX_CHECK_FAIL_COUNT = 3;
const uint8_t MAX_WRITE_FAIL_COUNT = 5;


void AbstractStorage::loop() {
    if (currPosition > saveBufferSize && writeTryCount < MAX_WRITE_FAIL_COUNT) {
        Serial.println("Start of writting data!");
        if (writeData()) {
            Serial.println("Data written, checking:");
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
                    Serial.println("Error checking data!");
                }
            }
            if (!dataChecked) {
                writeTryCount++;
            }
        }
    }
}


void AbstractStorage::add(Data &data) {
    saveBuffer[currPosition] = data;
    currPosition++;
    Serial.print("\nData added, count: ");
    Serial.println(currPosition);
}

const Data& AbstractStorage::getLast() {
    if (currPosition > 0) {
        return saveBuffer[currPosition - 1];
    } else {
        return saveBuffer[0];
    }
}

void AbstractStorage::enumerate(void (*consume)(Data &)) {
    Serial.print("AbstractStorage::enumerate, getBufferSize()=");
    Serial.println(getBufferSize());
    for (uint8_t i = 0; i < getBufferSize(); i++) {
        Serial.print("i=");
        Serial.println(i);
        consume(((Data*)this->getBuffer())[i] );
    }
}
*/