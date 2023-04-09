//
// Created by valti on 04.12.2022.
//

#include "data.h"

const char* message(ErrorCode errorCode) {
    switch (errorCode) {
        case E_REQUEST_DATA_NO_VALUE:
            return "Not digital value";
        case E_REQUEST_DATA_NOT_DIGITAL_VALUE:
            return "No value";
    }
    return "";
}