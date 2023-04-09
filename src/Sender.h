//
// Created by valti on 09.04.2023.
//

#ifndef UPS_SENDER_H
#define UPS_SENDER_H


#include "data.h"

class Sender {
public:
    virtual void sendBinary(const uint8_t* data, uint16_t bytesCoutn) = 0;
    virtual void sendData(char answerCodeChar, void (*writer)(Stream &stream)) = 0;

};


#endif //UPS_SENDER_H
