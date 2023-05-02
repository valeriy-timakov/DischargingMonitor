//
// Created by valti on 03.12.2022.
//

#ifndef UPS_UTILS_H
#define UPS_UTILS_H

#include "Communicator.h"



size_t sendSerial(bool value, Stream &serial = Serial);
size_t sendSerial(InstructionCode value, Stream &serial = Serial);
size_t sendSerial(InstructionDataCode value, Stream &serial = Serial);
size_t sendSerial(ErrorCode buffer, Stream &serial = Serial);
size_t sendSerial(Format value, Stream &serial = Serial);
size_t sendSerial(uint8_t value, Stream &serial = Serial);
size_t sendSerial(uint16_t value, Stream &serial = Serial);
size_t sendSerial(uint32_t value, Stream &serial = Serial);
size_t sendSerial(uint64_t value, Stream &serial = Serial);
size_t sendSerial(const uint8_t *buffer, size_t size, Stream &serial = Serial);
size_t sendSerial(const char *buffer, Stream &serial = Serial);

bool atou(const char* str, int len, uint32_t &result);

template<typename T>
void shift(T *data, uint8_t last, uint8_t size);

class utils {};

void dbgData(Data &data);

#endif //UPS_UTILS_H
