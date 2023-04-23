//
// Created by valti on 03.12.2022.
//

#ifndef UPS_LOG_H
#define UPS_LOG_H

#include "Arduino.h"
#include "data.h"
#include "TimeKeeper.h"

enum LogBit {
    LB_CHECK_DATA_ENTERED = 0,
    LB_CHECK_DATA_OK = 1,
    LB_PREPARE_DATA_ENTERED = 2,
    LB_PREPARE_DATA_OK = 3,
    LB_DATA_PROCEED_ENTERED = 4,
    LB_ADD_DATA_ENTERED = 5,
    LB_ADD_CURRENT_OUT_OF_PERMISSIBLE_VARIATION = 6,
    LB_ADD_VOLTAGE_OUT_OF_PERMISSIBLE_VARIATION = 7,
    LB_ADD_DATA_PREVIOUS_SAVED = 8,
    LB_PAGE_READY = 9,
    LB_PAGE_WRITTEN = 10,
    LB_PAGE_CHECKED = 11,
    LB_INFORM_STARTED = 12,
    LB_READ_READED = 13,
    LB_READ_SWITHCED_TO_WAIT = 14,
    LB_READ_TIMEOUT = 15,
    LB_READ_SWITCHED_TO_VOLTAGE= 16,
    LB_READ_REQUESTED = 17,
    LB_READ_REQUEST_FAILED = 18,
    LB_ADD_OVERFLOW = 19,
};

struct LogRegData {
    uint32_t registerValue;
    uint16_t cycle;
    uint16_t noActivityCycleCount;
};

#define LOG_BUFFER_SIZE 10

struct LogBuffer {
public:
    void print(Stream &stream) const;
    LogBuffer() : pos(0) {}
    void addIfAny(uint32_t registerValue, uint16_t cycle);

private:
    LogRegData data[LOG_BUFFER_SIZE];
    uint8_t pos;
    uint16_t noActivityCycleCount = 0;
};

class Log {
public:
    Log(TimeKeeper &timeKeeper) : timeKeeper(timeKeeper) {}
    void error(ErrorCode errorCode);
    void log(LogBit logBitNo);
    bool isLogEnabled() const;
    void setLogEnabled(bool value);
    void init();
    void loop();
    uint32_t getCountingStartTimestamp() const;
    const LogBuffer &getLogBuffer() const;
    const LogBuffer &getErrBuffer() const;

private:
    LogBuffer logBuffer;
    LogBuffer errBuffer;
    uint32_t logRegister;
    uint32_t errorsRegister;
    bool logEnabled;
    uint32_t countingStartTimestamp;
    uint16_t count;

    TimeKeeper &timeKeeper;
};


#endif //UPS_LOG_H
