//
// Created by valti on 03.12.2022.
//

#ifndef UPS_UTILS_H
#define UPS_UTILS_H

#include "Communicator.h"

uint32_t atoi(const char* str, int len);

template<typename T>
void shift(T *data, uint8_t last, uint8_t size);

class utils {};

void dbgData(Data &data);

#endif //UPS_UTILS_H
