//
// Created by valti on 27.10.2022.
//

#ifndef UPS_TIMEKEEPER_H
#define UPS_TIMEKEEPER_H

#include <Arduino.h>

class TimeKeeper {

public:
    void init();
    uint64_t millis();
private:
    uint64_t localTimestamp;
};


#endif //UPS_TIMEKEEPER_H
