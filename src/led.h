//
// Created by valti on 17.08.2022.
//

#ifndef UPS_LED_H
#define UPS_LED_H

#include "Arduino.h"

class LedBlink {
public:
    LedBlink(uint8_t pin, /*TimeKeeper &timeKeeper,*/ uint32_t onTimeMs = 1000, uint32_t offTimeMs = 500) :
        pin(pin), onTimeMs(onTimeMs), offTimeMs(offTimeMs)/*, timeKeeper(timeKeeper) */{}
    void init();
    void loop();
private:
    bool ledOn;
    long ledTime;
    uint8_t pin;
    uint32_t onTimeMs;
    uint32_t offTimeMs;
    //TimeKeeper timeKeeper;
};


#endif //UPS_LED_H
