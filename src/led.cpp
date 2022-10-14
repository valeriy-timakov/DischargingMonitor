//
// Created by valti on 17.08.2022.
//
#include "led.h"

void LedBlink::init() {
    pinMode(pin, OUTPUT);
    ledOn = true;
    ledTime = millis();
    digitalWrite(pin, HIGH);
}

void LedBlink::idle() {
    if (ledOn) {
        if (millis() - ledTime > onTimeMs) {
            digitalWrite(pin, LOW);
            ledOn = false;
            ledTime = millis();
        }
    } else {
        if (millis() - ledTime > offTimeMs) {
            digitalWrite(pin, HIGH);
            ledOn = true;
            ledTime = millis();

        }
    }
}

