//
// Created by valti on 03.08.2022.
//

#include "read.h"
#include "ADS.h"
#include "TimeKeeper.h"
#include "data.h"
#include "utils.h"
#include "Log.h"

const float baseResistance = 0.1;
#define R1 9.94
#define R2 467.1
#define READ_TIME_OUT 10000
const float voltageDividerCfnt = (R1 + R2) / R1;


static ReadMode readMode = M_WAIT;
static bool modeRequested = false;
static uint64_t lastRead = 0;
static uint16_t readInterval = 5000;
static uint32_t baseTime = 0;
static uint64_t baseLocalTime = 0;
static Data currData;

struct ReadData {
    uint8_t pin;
    uint16_t gain;
    float cfnt;
    ReadMode nextMode;
    void (*setter)(Data &, uint16_t);

    ReadData(uint8_t pin, uint16_t gain, float cfnt, ReadMode nextMode, void (*setter)(Data &, uint16_t)) :
            pin(pin), gain(gain), cfnt(cfnt), nextMode(nextMode), setter(setter) {}
};

void setVoltage(Data &data, uint16_t value) {
    data.voltage = value;
}

void setCurrent(Data &data, uint16_t value) {
    data.current = value;
}

const ReadData readConfig [] = {
    ReadData(2, ADS1X15_PGA_1_024V, voltageDividerCfnt, M_WAIT, setVoltage),
    ReadData(0, ADS1X15_PGA_1_024V, 1.0f / baseResistance, M_CURRENT, setCurrent)
};


uint32_t getTimeStamp() {
    return baseTime + (millis() - baseLocalTime);
}

Reader *pSelf;


void Reader::init() {
    beginADS();
    currData.timestamp = 0;
    pSelf = this;
}

void Reader::loop() {
    if (modeRequested) {
        uint16_t value;
        if (getRelativeValueIfExists(&value)) {
            ReadData conf = readConfig[readMode];
            conf.setter(currData, value);
            readMode = conf.nextMode;
            if (readMode == M_WAIT) {
                currData.timestamp = (currData.timestamp + getTimeStamp()) / 2;
                storage.add(currData);
            }
            modeRequested = false;
        } else if (millis() - lastRead > READ_TIME_OUT) {
            Log::error(E_SENSOR_READ_TIME_OUT, "Read value time out");
            modeRequested = false;
        }
    } else {
        if (readMode == M_WAIT) {
            if (millis() - lastRead > readInterval) {
                readMode = M_VOLTAGE;
            }
        } else {
            ReadData conf = readConfig[readMode];
            bool requested = requestValue(conf.pin, conf.gain);
            if (requested) {
                lastRead = millis();
                currData.timestamp = getTimeStamp();
                modeRequested = true;
            } else {
                Log::error(E_SENSOR_READ_REQUEST_FAILED, "Value request failed");
            }
        }
    }
}

void writeTimeStamp(Stream &stream) {
    uint32_t timestamp = baseTime + (millis() - baseLocalTime);
    stream.print(timestamp);
}

void writeLastReadTimeStamp(Stream &stream) {
    stream.print((uint32_t)(baseTime + (lastRead - baseLocalTime)));
}

void writeReadInterval(Stream &stream) {
    stream.print(readInterval);
}

float getCoefficient(ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return convertToVoltage(1, conf.gain) * conf.cfnt;
}

void writeInformCoefficients(Stream &stream) {
    stream.print(getCoefficient(M_VOLTAGE), 5);
    stream.print(",");
    stream.print(getCoefficient(M_CURRENT), 5);
}

void writeInformOrder(Stream &stream) {
    stream.print("v,c,t;");
}

float Reader::deserializeMilli(uint16_t relativeValue, ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return convertToVoltage(relativeValue, conf.gain) * conf.cfnt;
}

float (*deserializer)(Data &);

float Reader::deserializeCurrent(Data &data) {
    return deserializeMilli(data.current, M_CURRENT);
}

float Reader::deserializeVoltage(Data &data) {
    return deserializeMilli(data.voltage, M_VOLTAGE);
}

const Data& Reader::getLastData() {
    if (readMode == M_WAIT && currData.timestamp != 0) {
        return currData;
    } else {
        return storage.getLast();
    }
}

void writeValueWithTimestamp(Stream &stream) {
    const Data &tmpData = pSelf->getLastData();
    if (currData.timestamp == 0) {
        stream.print("Error! Nothing to read.");
    }
    stream.print(deserializer(currData), 3);
    stream.print(',');
    stream.print(currData.timestamp);
}

void syncTime(uint32_t value) {
    baseTime = value;
    baseLocalTime = millis();
}

void setReadInterval(uint32_t value) {
    readInterval = value;
}

void Reader::processInstruction(Instruction instruction, Communicator &communicator) {
    switch (instruction) {
        case GET_CURRENT:
            deserializer = deserializeCurrent;
            communicator.sendAnswer(A_CURRENT, writeValueWithTimestamp);
            break;
        case GET_VOLTAGE:
            deserializer = deserializeVoltage;
            communicator.sendAnswer(A_VOLTAGE, writeValueWithTimestamp);
            break;
        case GET_TIME:
            communicator.sendAnswer(A_TIME, writeTimeStamp);
            break;
        case SYNC_TIME:
            communicator.processIntValue(syncTime);
            break;
        case PERFORM_READ:
            if (readMode == M_WAIT) {
                readMode = M_VOLTAGE;
                communicator.sendSuccess();
            } else {
                communicator.sendError(E_SENSOR_READ_ALREADY_IN_PROGRESS, "Read already in progress");
            }
            break;
        case GET_LAST_READ_TIMESTAMP:
            communicator.sendAnswer(A_LAST_READ_TIMESTAMP, writeLastReadTimeStamp);
            break;
        case SET_READ_INTERVAL:
            communicator.processIntValue(setReadInterval);
            break;
        case GET_READ_INTERVAL:
            communicator.sendAnswer(A_READ_INTERVAL, writeReadInterval);
            break;
        case GET_INFORM_COEFFICIENTS:
            communicator.sendAnswer(A_INFORM_COEFFICIENTS, writeInformCoefficients);
            break;
        case GET_INFORM_ORDER:
            communicator.sendAnswer(A_INFORM_ORDER, writeInformOrder);
            break;
    }
}


/*
bool dataWriten = false;
#define PAGE_SIZE 256
uint8_t pageBuffer [PAGE_SIZE];
EEPROM25LC1024 EEPROM(10);
//EEPROM.init();
uint16_t pageToSave = 0;
static const int MAX_SAVE_TRY_COUNT = 5;
static const int MAX_CHECK_TRY_COUNT = 3;
uint8_t tryCheckCount = 0;
uint8_t saveTryCount = 0;
bool bufferToSaveReady = false;
#define BUFF_COUNT 35
Data saveBuffer [BUFF_COUNT];
const uint8_t DATA_PER_PAGE = PAGE_SIZE / DATA_SIZE;
uint8_t curPosition = 0;
uint8_t nextToSave = 0;
uint32_t nextPageToRead = 0;

void Reader::loopSave() {
    if (dataWriten) {
        Serial.println("Checking data in EEPROM...");
        if (EEPROM.checkData(pageToSave * PAGE_SIZE, pageBuffer, PAGE_SIZE)) {
            pageToSave++;
            if (pageToSave == PAGES_COUNT) {
                pageToSave = 0;
            }
            saveTryCount = 0;
            Serial.println("done. OK!");
            dataWriten = false;
        } else {
            tryCheckCount++;
            if (tryCheckCount >= MAX_SAVE_TRY_COUNT) {
                tryCheckCount = 0;
                saveTryCount++;
                Serial.println("done. ERROR!");
                dataWriten = false;
            } else {
                delay(200);
                Serial.println("done. Temporal error!");
            }
        }
    } else if (saveTryCount < MAX_SAVE_TRY_COUNT) {
        if (!bufferToSaveReady) {
            uint8_t countToSave = nextToSave <= curPosition ? curPosition - nextToSave : curPosition + BUFF_COUNT - nextToSave;
            uint16_t bytesCount = countToSave * DATA_SIZE;
            if (bytesCount >= PAGE_SIZE) {
                Serial.print("loop: bytesCount=");
                Serial.println(bytesCount);

                if (nextToSave >= curPosition) {
                    uint16_t bytesToCopy = (BUFF_COUNT - nextToSave) * DATA_SIZE;
                    memcpy(pageBuffer, ((uint8_t *) saveBuffer) + nextToSave * DATA_SIZE,
                           min(PAGE_SIZE, bytesToCopy));
                    if (bytesToCopy < PAGE_SIZE) {
                        memcpy(pageBuffer, saveBuffer, PAGE_SIZE - bytesToCopy);
                    }
                } else {
                    memcpy(pageBuffer, saveBuffer, PAGE_SIZE);
                }

                nextToSave += DATA_PER_PAGE;
                if (nextToSave >= BUFF_COUNT) {
                    nextToSave -= BUFF_COUNT;
                }

                bufferToSaveReady = true;
            }
        }
        if (bufferToSaveReady) {
            Serial.print("Writing data to EEPROM...");
            EEPROM.writeData(pageToSave * PAGE_SIZE, pageBuffer, PAGE_SIZE);
            dataWriten = true;
            Serial.println("done!");
        }
    }
}
*/
