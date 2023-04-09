//
// Created by valti on 03.08.2022.
//

#include "read.h"
#include "ADS.h"
#include "data.h"
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

void Reader::init() {
    beginADS();
    currData.timestamp = 0;
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

void Reader::writeTimeStamp(Stream &stream) {
    stream.print( (uint32_t)( baseTime + (millis() - baseLocalTime) ) );
}

void Reader::writeLastReadTimeStamp(Stream &stream) {
    stream.print( (uint32_t)( baseTime + (lastRead - baseLocalTime) ) );
}

void Reader::writeReadInterval(Stream &stream) {
    stream.print(readInterval);
}

float Reader::getCoefficient(ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return convertToVoltage(1, conf.gain) * conf.cfnt;
}

float Reader::deserializeMilli(uint16_t relativeValue, ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return convertToVoltage(relativeValue, conf.gain) * conf.cfnt;
}

float Reader::deserializeCurrent(const Data *data) {
    return deserializeMilli(data->current, M_CURRENT);
}

float Reader::deserializeVoltage(const Data *data) {
    return deserializeMilli(data->voltage, M_VOLTAGE);
}

const Data& Reader::getLastData() {
    if (readMode == M_WAIT && currData.timestamp != 0) {
        return currData;
    } else {
        return storage.getLast();
    }
}

ErrorCode Reader::syncTime(uint32_t value) {
    baseTime = value;
    baseLocalTime = millis();
    return OK;
}

ErrorCode Reader::setReadInterval(uint32_t value) {
    readInterval = value;
    return OK;
}

ErrorCode Reader::performRead() {
    if (readMode == M_WAIT) {
        readMode = M_VOLTAGE;
        return OK;
    } else {
        return E_SENSOR_READ_ALREADY_IN_PROGRESS;
    }
}

