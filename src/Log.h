//
// Created by valti on 03.12.2022.
//

#ifndef UPS_LOG_H
#define UPS_LOG_H

#include "Arduino.h"
#include "data.h"

class Log {
public:
    static void error(ErrorCode errorCode, const char *message);
    static void error(const char *message);
    void error(String message);

};


#endif //UPS_LOG_H
