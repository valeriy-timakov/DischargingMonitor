//
// Created by valti on 03.12.2022.
//

#ifndef UPS_LOG_H
#define UPS_LOG_H

#include "Arduino.h"
#include "data.h"

class Log {
public:
    static void error(ErrorCode errorCode);
};


#endif //UPS_LOG_H
