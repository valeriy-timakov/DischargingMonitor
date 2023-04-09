//
// Created by valti on 03.12.2022.
//

#include "Log.h"

void Log::error(ErrorCode errorCode, const char *message) {
    Serial.print("Error! ");
    Serial.print(errorCode);
    Serial.println(message);
}


void Log::error(const char *message) {
    Serial.print("Error! ");
    Serial.println(message);
}

void Log::error(String message) {
    error(message.c_str());
}