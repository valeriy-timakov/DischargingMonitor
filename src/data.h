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
};

const uint8_t DATA_SIZE = sizeof(Data);

enum ErrorCode {
    OK = 0,
    E_REQUEST_DATA_NO_VALUE = 1,
    E_REQUEST_DATA_NOT_DIGITAL_VALUE = 2,
    E_SENSOR_READ_TIME_OUT = 3,
    E_SENSOR_READ_REQUEST_FAILED = 4,
    E_SENSOR_READ_ALREADY_IN_PROGRESS = 5
};

const char* message(ErrorCode errorCode);

#endif //UPS_DATA_H
