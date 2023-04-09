//
// Created by valti on 04.12.2022.
//
/*
#include "FlashStorage.h"
#include "SD.h"
#include "Log.h"

const String bufferDirectory = "/buffer";
const String archiveDirectory = "/archive";

void FlashStorage::init() {
    Log::error("Initializing SD card...");
    if (!SD.begin(csPin)) {
        Serial.println("Card failed, or not present");
    }
    if (!SD.exists(bufferDirectory)) {
        SD.mkdir(bufferDirectory);
    }
    if (!SD.exists(archiveDirectory)) {
        SD.mkdir(archiveDirectory);
    }
    Serial.println("card initialized.");
}


const uint8_t* FlashStorage::getBuffer() {
    return pageBuffer;
}

uint16_t FlashStorage::getBufferSize() {
    return PAGE_BUFFER_SIZE_BYTES;
}

char *currFileName = NULL;

bool FlashStorage::prepareData() {
    File root = SD.open(bufferDirectory);
    File next = root.openNextFile();
    uint32_t minTimestamp = INT32_MAX;
    while (next) {
        if (!next.isDirectory()) {
            uint64_t currTimestamp = atoi(next.name());
            minTimestamp = min( minTimestamp, currTimestamp);
            if (minTimestamp > currTimestamp) {
                minTimestamp = currTimestamp;
                currFileName = next.name();
            }
        }
        next = root.openNextFile();
    }
    if (currFileName != nullptr) {
        File currFile = SD.open(currFileName, FILE_READ);
        currFile.readBytes(pageBuffer, PAGE_BUFFER_SIZE_BYTES);
        currFile.close();
        return true;
    }
    return false;
}

void FlashStorage::dataProcessed() {
    if (currFileName != nullptr) {
        String fromFileName = bufferDirectory + "/" + currFileName;
        File fileIn = SD.open(fromFileName, FILE_READ);
        File fileOut = SD.open(archiveDirectory + "/" + currFileName, FILE_WRITE);
        while (fileIn.available()) {
            fileOut.write(fileIn.read());
        }
        fileIn.close();
        fileOut.close();
        SD.remove(fromFileName);
        currFileName = nullptr;
    }
}


bool FlashStorage::writeData() {
    String fileName = String(saveBuffer[0].timestamp);
    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
        dataFile.write((uint8_t *) saveBuffer, saveBufferSize);
        dataFile.close();
        Serial.println("data written to SD card!!!");
        return true;
    }
    Log::error("Error opening file to write data!");
    return false;
}

bool FlashStorage::checkData() {
    String fileName = String(saveBuffer[0].timestamp);
    File dataFile = SD.open(fileName, FILE_READ);
    if (dataFile) {
        int readed = dataFile.read();
        uint8_t pos = 0;
        auto *buffPtr = (uint8_t *) saveBuffer;
        while (pos < saveBufferSize) {
            if (readed == -1 || readed != buffPtr[pos]) {
                Log::error("Error checking data!");
                return false;
            }
            readed = dataFile.read();
            pos++;
        }
        return true;
    }
    Log::error("Error opening file to check writen data!");
    return false;
}
*/