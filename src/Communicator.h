//
// Created by valti on 19.08.2022.
//

#ifndef UPS_COMMUNICATOR_H
#define UPS_COMMUNICATOR_H

#include <Arduino.h>
#include "data.h"


#define CMD_ID_SIZE 4
#define CMD_INSTR_SIZE 2
#define CMD_BUFF_SIZE 30
#define COMMAND_START_CHAR '('
#define COMMAND_END_CHAR ')'

enum Instruction {
    ERROR_NOT_READY = 0,
    SYNC_TIME = 1,
    GET_TIME = 2,
    GET_CURRENT = 3,
    GET_VOLTAGE = 4,
    SET_INFORM_INTERVAL = 5,
    GET_INFORM_INTERVAL = 6,
    PERFORM_READ = 7,
    GET_LAST_READ_TIMESTAMP = 8,
    SET_READ_INTERVAL = 9,
    GET_READ_INTERVAL = 10,
    SET_INFORM_FORMAT = 11,
    GET_INFORM_FORMAT = 12,
    GET_INFORM_COEFFICIENTS = 13,
    GET_INFORM_ORDER = 14,
    ERROR_UNRECOGIZED = 100
};

enum Answer {
    ERROR,
    A_TIME,
    A_CURRENT,
    A_VOLTAGE,
    A_INFORM,
    A_LAST_READ_TIMESTAMP,
    A_INFORM_INTERVAL,
    A_READ_INTERVAL,
    A_INFORM_FORMAT,
    A_SUCCESS,
    A_INFORM_COEFFICIENTS,
    A_INFORM_ORDER,
    //always should be last in this enum!!!
    A_ITEMS_COUNT
};

enum InformFormat {
    IF_TEXT = 0,
    IF_BINARY = 1
};


class Communicator {
public:
    Communicator()  {
        answerChars[ERROR] = 'E';
        answerChars[A_CURRENT] = 'c';
        answerChars[A_VOLTAGE] = 'v';
        answerChars[A_TIME] = 't';
        answerChars[A_LAST_READ_TIMESTAMP] = 'l';
        answerChars[A_INFORM] = 'i';
        answerChars[A_SUCCESS] = 's';
        answerChars[A_READ_INTERVAL] = 'r';
        answerChars[A_INFORM_FORMAT] = 'f';
        answerChars[A_INFORM_COEFFICIENTS] = 'n';
        answerChars[A_INFORM_ORDER] = 'o';
    }
    uint8_t readCommand();
    Instruction getInstruction();
    bool getCommandId(char **res);
    size_t getData(char **res);
    void processIntValue(void (*processor)(uint32_t));
    void sendAnswer(Answer answer, void (*writer)(Stream &stream));
    bool sendData(Answer answer, void (*writer)(Stream &stream));
    bool sendBinary(const uint8_t* data, uint16_t bytesCoutn);
    void sendSuccess();
    void sendError(ErrorCode  code, const char *message);
    static Communicator& getInstance();
private:
    char cmdBuff[CMD_BUFF_SIZE];
    uint8_t curCmdBuffPos = 0;
    bool startDetected = false;
    bool commandParsed = false;
    char answerChars[A_ITEMS_COUNT];


    static Communicator instance;

};


#endif //UPS_COMMUNICATOR_H
