//
// Created by valti on 27.10.2022.
//

#include "TimeKeeper.h"

uint64_t time = 0;

ISR(TIMER2_COMPA_vect) {
    time++;
}

void initOscilatorCounter2() {
    TCCR2A = 1 << WGM21;
    TCCR2B = 0;
    OCR2A = 16000000UL / 64 / 1000 - 1;
    TIMSK2 = 1 << OCF2A;
    TCCR2B |= 1 << CS22;
}

void initExternalCounter0() {
//    TCCR0A = 1 << WGM01;
//    TCCR0B = 0;
//    OCR0A = 40;
//    TIMSK0 = 1 << OCF0A;
//    TCCR0B |= 1 << CS02;
}

void TimeKeeper::init() {
    cli();
    initOscilatorCounter2();
    initExternalCounter0();
    sei();
}

uint64_t TimeKeeper::millis() {
    return time;
}
