//
// Created by valti on 19.08.2022.
//

#ifndef UPS_COMMUNICATOR_H
#define UPS_COMMUNICATOR_H

#include <Arduino.h>


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
    ERROR_UNRECOGIZED = 100
};

enum Answer {
    ERROR = 0,
    TIME = 1,
    CURRENT = 2,
    VOLTAGE = 3,
    SUCCESS
};

struct Str {
    char* data;
    size_t len;
};


class Communicator {
public:
    uint8_t readCommand();
    Instruction getInstruction();
    bool getCommandId(char **res);
    size_t getData(char **res);
    void send(Answer answer, void (*writer)(Stream &stream));
    void sendSuccess();
    void sendError(const char *message);
    static Communicator& getInstance();
private:
    char cmdBuff[CMD_BUFF_SIZE];
    uint8_t curCmdBuffPos = 0;
    bool startDetected = false;
    bool commandParsed = false;
    void send(Answer answer, void (*writer)(Stream &stream), const char *data);

    static Communicator instance;

};


#endif //UPS_COMMUNICATOR_H
