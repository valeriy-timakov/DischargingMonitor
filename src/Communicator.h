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
#include "TimeKeeper.h"


#define CMD_ID_SIZE 4
#define CMD_INSTR_SIZE 2
#define CMD_BUFF_SIZE 30
#define COMMAND_START_CHAR '('
#define COMMAND_END_CHAR ')'
#define COMMAND_WITHOUT_ADDR_START_CHAR '['
#define COMMAND_WITHOUT_ADDR_END_CHAR ']'

static const int MAX_COMMAND_READ_TIME = 100;

class Communicator {
public:
    Communicator(Informer &informer, Reader &reader, EEPROMStorage &storage, Log &log, TimeKeeper &timeKeeper) :
        informer(informer), reader(reader), storage(storage), log(log), timeKeeper(timeKeeper)  {}
    void loop();
private:
    char cmdBuff[CMD_BUFF_SIZE];
    uint8_t curCmdBuffPos = 0;
    bool startDetected = false;
    bool commandParsed = false;
    bool commandWithAddress;
    Format format = F_TEXT;
    bool formatChanged = false;
    uint32_t lastPacketTime = 0;
    uint8_t lastPacketSize = 0;
    Informer &informer;
    Reader &reader;
    EEPROMStorage &storage;
    Log &log;
    TimeKeeper &timeKeeper;


    bool readTextCommand();
    void processTextInstruction();
    size_t getData(char **res);
    void processIntValue(ErrorCode (*processor)(Communicator *self, uint32_t));
    void sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream, void *additionalData), void *additionalData = nullptr);
    void sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream));
    void sendAnswer(char answerCodeChar);
    void sendSuccess();
    void sendError(ErrorCode  code);

    bool readBinaryCommand();
    void processBinaryInstruction();

    ErrorCode processBinaryRead(uint8_t code);
    ErrorCode processBinarySet(uint8_t code);
    ErrorCode processBinaryExecute(uint8_t code);
    uint8_t getByteFromData();
    uint16_t getShortFromData();
    uint32_t getIntFromData();
    uint64_t getLongFromData();

    void startAnswer(char answerCodeChar) const;

    void endAnswer() const;
};


#endif //UPS_COMMUNICATOR_H
