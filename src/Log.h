//
// Created by valti on 03.12.2022.
//

#ifndef UPS_LOG_H
#define UPS_LOG_H

#include "Arduino.h"
#include "data.h"

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

class Log {
public:
    void error(ErrorCode errorCode);
    void log(LogBit logBitNo);
    bool isLogEnabled();
    void setLogEnabled(bool value);
    uint32_t getLogResister();
    void clearLogRegister();
private:
    uint32_t logRegister;
    bool logEnabled;
};


#endif //UPS_LOG_H
