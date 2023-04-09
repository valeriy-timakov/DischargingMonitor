//
// Created by valti on 19.08.2022.
//

#ifndef UPS_COMMUNICATOR_H
#define UPS_COMMUNICATOR_H

#include <Arduino.h>
#include "data.h"
#include "Informer.h"
#include "Sender.h"
#include "read.h"


#define CMD_ID_SIZE 4
#define CMD_INSTR_SIZE 2
#define CMD_BUFF_SIZE 30
#define COMMAND_START_CHAR '('
#define COMMAND_END_CHAR ')'

enum Instruction {
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
    PERFORM_INFORM = 15,
    INFORM_DATA_PROCEEDED = 16,
    INFORM_DATA_PROCEED_ERROR = 17,
};


class Communicator : public Sender {
public:
    Communicator(Informer informer, Reader &reader) : informer(informer), reader(reader)  {}
    void loop();
private:
    char cmdBuff[CMD_BUFF_SIZE];
    uint8_t curCmdBuffPos = 0;
    bool startDetected = false;
    bool commandParsed = false;
    Informer &informer;
    Reader &reader;

    bool readCommand();
    void processInstruction();
    size_t getData(char **res);
    void processIntValue(ErrorCode (*processor)(Communicator *self, uint32_t));
    void sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream));
    void sendData(char answerCodeChar, void (*writer)(Stream &stream));
    void sendBinary(const uint8_t* data, uint16_t bytesCoutn);
    void sendSuccess();
    void sendError(ErrorCode  code);
    void sendErrorIfNotSuccess(ErrorCode code);

};


#endif //UPS_COMMUNICATOR_H
