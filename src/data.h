//
// Created by valti on 03.12.2022.
//

#ifndef UPS_DATA_H
#define UPS_DATA_H

#include <Arduino.h>

enum CompareConfigIdx {
    DCC_CURRENT = 0,
    DCC_VOLTAGE = 1
};

enum Format {
    F_TEXT = 0,
    F_BINARY = 1
};

const uint32_t MAX_U32_VALUE = 0xFFFFFFFF;
const uint16_t MAX_U16_VALUE = 0xFFFF;

struct Data {
    uint16_t voltage;
    uint16_t current;
    uint32_t timestamp;

    uint16_t value(CompareConfigIdx idx) const {
        return idx == DCC_CURRENT ? current : voltage;
    }

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
    E_NEW_COMMAND_INSIDE = 11,
    E_COMMAND_EMPTY = 12,
    E_COMMAND_SIZE_OVERFLOW = 13,
    E_COMMAND_END_WITHOUT_START = 14,
    E_CHECK_FAILED_AFTER_WRITE = 15,
    E_INSTRUCTION_WRONG_START = 16,
    E_WRITE_MAX_ATTEMPTS_EXCEDED = 17,
    E_ROUNDING_DATA_ADD_WRONG_TIME = 18,
    E_UNDEFINED_CODE = 100
};

enum InstructionCode {
    IC_NONE = 0,
    IC_READ = 1,
    IC_SET = 2,
    IC_EXECUTE = 3,
    IC_INFORM = 4,
    IC_SUCCESS = 5,
    IC_ERROR = 6,
    IC_FORMAT_CHANGED = 7,
    IC_REBOOT = 8,
    IC_UNKNOWN = 9
};

enum InstructionDataCode {
    IDC_NONE = 0,
    IDC_VOLTAGE = 1,
    IDC_CURRENT = 2,
    IDC_TIME = 3,
    IDC_LAST_READ_TIME = 4,
    IDC_INFORM_INTERVAL = 5,
    IDC_MEASUREMENTS_INTERVAL = 6,
    IDC_VOLTAGE_PERMISSIBLE_VARIATION = 7,
    IDC_CURRENT_PERMISSIBLE_VARIATION = 8,
    IDC_LOG_ENABLED = 9,
    IDC_INFORM_DATA_COEFFICIENTS = 10,
    IDC_LOG_REGISTER_VALUES = 11,
    IDC_ERR_REGISTER_VALUES = 12,
    IDC_AVG_DATA_PREPARED_DATA_TIMESTAMP = 13,
    IDC_AVG_DATA_SAVED_DATA_TIMESTAMP = 14,
    IDC_EXTERNAL_TIMESTAMP_ID = 15,
    IDC_UNKNOWN = 20
};


#endif //UPS_DATA_H
