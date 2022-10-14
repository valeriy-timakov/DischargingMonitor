//
// Created by valti on 03.08.2022.
//

#ifndef ATTINYTEST_DS2502OPT_H
#define ATTINYTEST_DS2502OPT_H

void initDS5202(const uint8_t pin, uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);
bool pollDS5202();
bool    writeDS5202Memory(const uint8_t* source, uint8_t length, uint8_t position = 0);



#endif //ATTINYTEST_DS2502OPT_H
