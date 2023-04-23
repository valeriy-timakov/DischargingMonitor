//
// Created by valti on 27.10.2022.
//

#ifndef UPS_TIMEKEEPER_H
#define UPS_TIMEKEEPER_H

#include <Arduino.h>

class TimeKeeper {

public:
    void init();
    uint64_t _millis();
    uint32_t getCurrent() const;
    void syncTime(uint32_t value);
    uint32_t getCurrentId() const;
    void setCurrentId(uint32_t value);
private:
    uint64_t localTimestamp;
    uint32_t baseTime = 0;
    uint64_t baseLocalTime = 0;
    uint32_t currentId = 0;
};


#endif //UPS_TIMEKEEPER_H
