//
// Created by valti on 03.12.2022.
//

#include "utils.h"


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
