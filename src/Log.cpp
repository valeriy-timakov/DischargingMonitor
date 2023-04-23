//
// Created by valti on 03.12.2022.
//

#include "Log.h"

void Log::error(ErrorCode errorCode) {
    errorsRegister |= 1 << (errorCode - 1);
    if (logEnabled) {
        Serial.print("(E");
        Serial.print(errorCode);
        Serial.println(")");
    }
}

void Log::log(LogBit logBitNo) {
    logRegister |= 1 << logBitNo;
}


bool Log::isLogEnabled() const {
    return logEnabled;
}

void Log::setLogEnabled(bool value) {
    logEnabled = value;
}

void Log::init() {
    logRegister = 0;
    errorsRegister = 0;
    logEnabled = false;
    countingStartTimestamp = timeKeeper.getCurrent();
    count = 0;
}

void Log::loop() {
    logBuffer.addIfAny(logRegister, count);
    errBuffer.addIfAny(errorsRegister, count);

    if (count == MAX_U16_VALUE) {
        countingStartTimestamp = timeKeeper.getCurrent();
        count = 0;
    }
    count++;
    logRegister = 0;
    errorsRegister = 0;
}

uint32_t Log::getLogRegister() const {
    return logRegister;
}

uint32_t Log::getErrRegister() const {
    return errorsRegister;
}

uint32_t Log::getCountingStartTimestamp() const {
    return countingStartTimestamp;
}

const LogBuffer &Log::getLogBuffer() const {
    return logBuffer;
}

const LogBuffer &Log::getErrBuffer() const {
    return errBuffer;
}

void LogBuffer::addIfAny(uint32_t registerValue, uint16_t cycle) {
    if (registerValue > 0) {
        data[pos].registerValue = registerValue;
        data[pos].cycle = cycle;
        if (pos == LOG_BUFFER_SIZE) {
            pos = 0;
        } else {
            pos++;
        }
    }
}

const LogRegData *LogBuffer::getData() const {
    return data;
}

void LogBuffer::print(Stream &stream) const {
    for (int i = 0; i < LOG_BUFFER_SIZE; i++) {
        stream.print(data[i].cycle);
        stream.print(":");
        stream.print(data[i].registerValue, BIN);
        if (i < LOG_BUFFER_SIZE - 1) {
            stream.println(",");
        }
    }
}
