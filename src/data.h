//
// Created by valti on 03.12.2022.
//

#ifndef UPS_DATA_H
#define UPS_DATA_H

#include <Arduino.h>

struct Data {
    uint16_t voltage;
    uint16_t current;
    uint32_t timestamp;

public:
    bool available() const {
        return timestamp > 0;
    }
};

const auto UN_AVAILABLE_DATA = Data {0, 0, 0};


enum ErrorCode {
    OK = 0,
    E_REQUEST_DATA_NO_VALUE = 1,
    E_REQUEST_DATA_NOT_DIGITAL_VALUE = 2,
    E_SENSOR_READ_TIME_OUT = 3,
    E_SENSOR_READ_REQUEST_FAILED = 4,
    E_SENSOR_READ_ALREADY_IN_PROGRESS = 5,
    E_INFORM_NO_DATA_TO_SEND = 6,
    E_INFORM_PACKAGE_ALREADY_SENT = 7,
    E_INSTRUCTION_UNRECOGIZED = 8,
    E_READ_NO_DATA_TO_SEND = 9,
    E_NO_PACKAGE_WAS_SENT = 10,
    E_UNDEFINED_CODE = 11,
    S_ALREADY_SENT = -1
};


#endif //UPS_DATA_H
