//
// Created by valti on 03.12.2022.
//

#include "Log.h"

void Log::error(ErrorCode errorCode) {
    Serial.print("E");
    Serial.print(errorCode);
}
