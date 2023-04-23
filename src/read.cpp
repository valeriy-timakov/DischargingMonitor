//
// Created by valti on 03.08.2022.
//

#include "read.h"
#include "ADS.h"
#include "data.h"
#include "Log.h"

const float baseResistance = 0.130;
#define R1 9.99
#define R2 467.1
#define READ_TIME_OUT 10000
const float voltageDividerCfnt = (R1 + R2) / R1;//48.462


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
    ReadData(2, ADS1X15_PGA_1_024V, voltageDividerCfnt, M_CURRENT, setVoltage),
    ReadData(0, ADS1X15_PGA_0_256V, 1.0f / baseResistance, M_WAIT, setCurrent)
};



void Reader::init() {
    ads.begin();
    currData.timestamp = 0;
}

void Reader::loop() {
    if (modeRequested) {
        uint16_t value;
        if (ads.getRelativeValueIfExists(&value)) {
            log.log(LB_READ_READED);
            ReadData conf = readConfig[readMode];
            conf.setter(currData, value);
            readMode = conf.nextMode;
            if (readMode == M_WAIT) {
                log.log(LB_READ_SWITHCED_TO_WAIT);
                currData.timestamp = (currData.timestamp + timeKeeper.getCurrent()) / 2;
                storage.add(currData);
            }
            modeRequested = false;
        } else if (timeKeeper.getCurrent() - lastRead > READ_TIME_OUT) {
            log.log(LB_READ_TIMEOUT);
            log.error(E_SENSOR_READ_TIME_OUT);
            modeRequested = false;
        }
    } else {
        if (readMode == M_WAIT) {
            if (timeKeeper.getCurrent() - lastRead > readInterval) {
                log.log(LB_READ_SWITCHED_TO_VOLTAGE);
                readMode = M_VOLTAGE;
            }
        } else {
            ReadData conf = readConfig[readMode];
            if (ads.requestValue(conf.pin, conf.gain)) {
                log.log(LB_READ_REQUESTED);
                lastRead = timeKeeper.getCurrent();
                currData.timestamp = lastRead;
                modeRequested = true;
            } else {
                log.log(LB_READ_REQUEST_FAILED);
                log.error(E_SENSOR_READ_REQUEST_FAILED);
            }
        }
    }
}

uint32_t Reader::getLastReadTimeStamp() const {
    return lastRead;
}

float Reader::getCoefficient(ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return ADS3x::convertToVoltage(1, conf.gain) * conf.cfnt;
}

float Reader::deserializeMilli(uint16_t relativeValue, ReadMode mode) {
    const ReadData &conf = readConfig[mode];
    return ADS3x::convertToVoltage(relativeValue, conf.gain) * conf.cfnt;
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

uint32_t Reader::getReadInterval() const {
    return readInterval;
}

void Reader::setReadInterval(uint32_t value) {
    readInterval = value;
}

ErrorCode Reader::performRead() {
    if (readMode == M_WAIT) {
        readMode = M_VOLTAGE;
        return OK;
    } else {
        return E_SENSOR_READ_ALREADY_IN_PROGRESS;
    }
}
