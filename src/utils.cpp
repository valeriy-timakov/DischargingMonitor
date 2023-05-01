//
// Created by valti on 03.12.2022.
//

#include "utils.h"



size_t sendSerial(bool value, Stream &serial) {
    return serial.write(value ? 1 : 0);
}

size_t sendSerial(uint8_t value, Stream &serial) {
    return serial.write(value);
}

size_t sendSerial(InstructionCode value, Stream &serial) {
    return sendSerial((uint8_t)value, serial);
}

size_t sendSerial(InstructionDataCode value, Stream &serial) {
    return sendSerial((uint8_t)value, serial);
}

size_t sendSerial(Format value, Stream &serial) {
    return sendSerial((uint8_t)value, serial);
}

size_t sendSerial(ErrorCode buffer, Stream &serial) {
    return serial.write(buffer);
}

size_t sendSerial(uint16_t value, Stream &serial) {
    uint8_t sum = 0;
    sum += serial.write(value >> 8);
    sum += serial.write(value & 0xFF);
    return sum;
}

size_t sendSerial(uint32_t value, Stream &serial) {
    uint8_t sum = 0;
    sum += serial.write(value >> 24);
    sum += serial.write((value >> 16) & 0xFF);
    sum += serial.write((value >> 8) & 0xFF);
    sum += serial.write(value & 0xFF);
    return sum;
}

size_t sendSerial(uint64_t value, Stream &serial) {
    uint8_t sum = 0;
    sum += serial.write((uint8_t)(value >> 56));
    sum += serial.write((uint8_t)((value >> 48) & 0xFF));
    sum += serial.write((uint8_t)((value >> 40) & 0xFF));
    sum += serial.write((uint8_t)((value >> 32) & 0xFF));
    sum += serial.write((uint8_t)((value >> 24) & 0xFF));
    sum += serial.write((uint8_t)((value >> 16) & 0xFF));
    sum += serial.write((uint8_t)((value >> 8) & 0xFF));
    sum += serial.write((uint8_t)(value & 0xFF));
    return sum;
}

size_t sendSerial(const uint8_t *buffer, size_t size, Stream &serial) {
    return serial.write(buffer, size);
}

size_t sendSerial(const char *buffer, Stream &serial) {
    return serial.write(buffer);
}

void dbgData(Data &data) {
    Serial.print("(");
    Serial.print(Reader::deserializeVoltage(&data), 4);
    Serial.print(";");
    Serial.print(Reader::deserializeCurrent(&data), 7);
    Serial.print(";");
    Serial.print(data.timestamp);
    Serial.println(")");
}

uint32_t atoi(const char* str, int len) {
    uint32_t ret = 0;
    uint8_t charValue;
    for(int i = 0; i < len; ++i) {
        charValue = str[i] - '0';
        if (charValue > 9) {
            return -1;
        }
        ret = ret * 10 + charValue;
    }
    return ret;
}

uint8_t getNext(uint8_t pos, uint8_t last, uint8_t size) {
    uint8_t dLast = last + 1;
    if (pos < size - dLast) {
        return pos + dLast;
    } else {
        return pos - (size - dLast);
    }
}

template<typename T>
void shift(T *data, uint8_t last, uint8_t size) {
    uint32_t moveCount = 0;
    uint64_t mask = 0;
    while (moveCount < size) {
        uint8_t start = size - 1;
        while ((mask & (1 << start)) > 0) {
            start--;
        }
        T tmp = data[start];
        uint8_t prevNext = start, next = getNext(start, last, size);
        while (next != start) {
            data[prevNext] = data[next];
            moveCount++;
            mask |= 1 << prevNext;
            prevNext = next;
            next = getNext(next, last, size);
        }
        data[next] = tmp;
        moveCount++;
        mask |= 1 << prevNext;
    }
}


void debugArr(byte* arr, uint16_t size) {
    Serial.print('[');
    for (uint16_t i = 0; i < size; i++) {
        Serial.print(arr[i]);
        Serial.print(' ');
    }
    Serial.println(']');
}
