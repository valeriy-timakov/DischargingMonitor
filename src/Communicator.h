//
// Created by valti on 19.08.2022.
//

#ifndef UPS_COMMUNICATOR_H
#define UPS_COMMUNICATOR_H

#include <Arduino.h>
#include "data.h"
#include "Informer.h"
#include "read.h"
#include "EEPROMStorage.h"
#include "Log.h"


#define CMD_ID_SIZE 4
#define CMD_INSTR_SIZE 2
#define CMD_BUFF_SIZE 30
#define COMMAND_START_CHAR '('
#define COMMAND_END_CHAR ')'

class Communicator {
public:
    Communicator(Informer &informer, Reader &reader, EEPROMStorage &storage, Log &log) : informer(informer), reader(reader),
        storage(storage), log(log)  {}
    void loop();
private:
    char cmdBuff[CMD_BUFF_SIZE];
    uint8_t curCmdBuffPos = 0;
    bool startDetected = false;
    bool commandParsed = false;
    Informer &informer;
    Reader &reader;
    EEPROMStorage &storage;
    Log &log;


    bool readCommand();
    void processInstruction();
    size_t getData(char **res);
    void processIntValue(ErrorCode (*processor)(Communicator *self, uint32_t));
    void sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream));
    void sendSuccess();
    void sendError(ErrorCode  code);

};


#endif //UPS_COMMUNICATOR_H
