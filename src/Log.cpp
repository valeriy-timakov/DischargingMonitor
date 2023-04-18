//
// Created by valti on 03.12.2022.
//

#include "Log.h"

void Log::error(ErrorCode errorCode) {
    if (logEnabled) {
        Serial.print("E");
        Serial.print(errorCode);
    }
}

void Log::log(LogBit logBitNo) {
    logRegister |= 1 << logBitNo;
}


bool Log::isLogEnabled() {
    return logEnabled;
}


void Log::setLogEnabled(bool value) {
    logEnabled = value;
}


uint32_t Log::getLogResister() {
    return logRegister;
}


void Log::clearLogRegister() {
    logRegister = 0;
}

