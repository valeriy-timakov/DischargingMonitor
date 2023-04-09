//
// Created by valti on 19.08.2022.
//

#include "Communicator.h"
#include "utils.h"


uint8_t Communicator::readCommand() {
    commandParsed = false;
    while (Serial.available()) {
        char received = Serial.read();
        if (startDetected) {
            switch (received) {
                case COMMAND_START_CHAR:
                    startDetected = false;
                    curCmdBuffPos = 0;
                    Serial.print("(Error");
                    if (curCmdBuffPos > 0) {
                        Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
                    }
                    Serial.print(" new cmd inside!)");
                    Serial.print(received);
                    break;
                case COMMAND_END_CHAR:
                    startDetected = false;
                    if (curCmdBuffPos > 0) {
                        commandParsed = true;
                        Instruction instruction = getInstruction();
                        if (instruction == ERROR_UNRECOGIZED) {
                            commandParsed = false;
                            Serial.print("(Error command not recognized! Got: ");
                            Serial.write(cmdBuff, curCmdBuffPos);
                            Serial.print(")");
                        } else {
                            return curCmdBuffPos;
                        }
                    } else {
                        Serial.print("(Error command empty!)");
                    }
                    break;
                default:
                    if (curCmdBuffPos < CMD_BUFF_SIZE - 1) {
                        cmdBuff[curCmdBuffPos] = received;
                        curCmdBuffPos++;
                    } else {
                        Serial.print("(Error");
                        Serial.write(cmdBuff, CMD_ID_SIZE);
                        Serial.print(" cmd size overflow!)");
                        startDetected = false;
                    }
            }
        } else {
            if (received == COMMAND_START_CHAR) {
                curCmdBuffPos = 0;
                commandParsed = false;
                startDetected = true;
            } else if (received == COMMAND_END_CHAR) {
                Serial.print("(Error end without start!)");
            } else {
                Serial.print(received);
            }
        }
    }
    return 0;
}

bool Communicator::getCommandId(char **res) {
    if (commandParsed) {
        *res = cmdBuff;
        return min(curCmdBuffPos, CMD_ID_SIZE);
    }
    return 0;
}

Instruction Communicator::getInstruction() {
    if (!commandParsed) {
        return ERROR_NOT_READY;
    }
    if (curCmdBuffPos >= CMD_ID_SIZE + CMD_INSTR_SIZE) {
        char instrFirst = cmdBuff[CMD_ID_SIZE];
        char instrSecond = cmdBuff[CMD_ID_SIZE + 1];
        if (instrFirst  == 'r') {
            if (instrSecond == 't') {
                return GET_TIME;
            } else if (instrSecond == 'v') {
                return GET_VOLTAGE;
            } else if (instrSecond == 'c') {
                return GET_CURRENT;
            } else if (instrSecond == 'i') {
                return GET_INFORM_INTERVAL;
            } else if (instrSecond == 'l') {
                return GET_LAST_READ_TIMESTAMP;
            } else if (instrSecond == 'r') {
                return GET_READ_INTERVAL;
            } else if (instrSecond == 'f') {
                return GET_INFORM_FORMAT;
            } else if (instrSecond == 'n') {
                return GET_INFORM_COEFFICIENTS;
            } else if (instrSecond == 'o') {
                return GET_INFORM_ORDER;
            }
        } else if (instrFirst == 's') {
            if (instrSecond == 't') {
                return SYNC_TIME;
            } else if (instrSecond == 'i') {
                return SET_INFORM_INTERVAL;
            } else if (instrSecond == 'r') {
                return SET_READ_INTERVAL;
            } else if (instrSecond == 'f') {
                return SET_INFORM_FORMAT;
            }
        } else if (instrFirst == 'e') {
            if (instrSecond == 'r') {
                return PERFORM_READ;
            }
        }
    }
    return ERROR_UNRECOGIZED;
}

size_t Communicator::getData(char **res) {
    uint8_t dataStartPos = CMD_ID_SIZE + CMD_INSTR_SIZE;
    if (commandParsed && curCmdBuffPos > dataStartPos) {
        *res = cmdBuff + dataStartPos;
        return curCmdBuffPos - dataStartPos;
    }
    return false;
}


void Communicator::processIntValue(void (*processor)(uint32_t)) {
    char *data;
    size_t size = getData(&data);
    ErrorCode code;
    if (size > 0) {
        long tmpValue = atoi(data, size);
        if (tmpValue == -1) {
            code = E_REQUEST_DATA_NO_VALUE;
        }
        processor((uint32_t)tmpValue);
        code = OK;
    } else {
        code = E_REQUEST_DATA_NOT_DIGITAL_VALUE;
    }
    if (code == OK) {
        sendSuccess();
    } else {
        sendError(code, message(code));
    }
}

/*
void Communicator::send(Answer answer, Str &data, Str &commandId) {
    Serial.print("(");
    if (answer == ERROR) {
        Serial.print("Error");
    }
    Serial.write(commandId.data, commandId.len);
    switch (answer) {
        case A_CURRENT:
            Serial.print("c");
            break;
        case A_VOLTAGE:
            Serial.print("v");
            break;
        case A_TIME:
            Serial.print("t");
            break;
    }
    Serial.SPIwrite(data.data, data.len);
    Serial.print(")");
}

void Communicator::send(Answer answer, Str &data) {
    uint8_t idSize = min(curCmdBuffPos, CMD_ID_SIZE);
    Str id = {cmdBuff, idSize};
    sendAnswer(answer, data, id);
}
*/

void Communicator::sendSuccess() {
    sendAnswer(A_SUCCESS, NULL);
}

ErrorCode errorCode;
const char *errorMessage;

void writeError(Stream &stream) {
    stream.print(';');
    stream.print(errorCode);
    stream.print(';');
    stream.print(errorMessage);
}

void Communicator::sendError(ErrorCode code, const char *message) {
    errorCode = code;
    errorMessage = message;
    sendAnswer(ERROR, writeError);
}

void Communicator::sendAnswer(Answer answer, void (*writer)(Stream &)) {
    Serial.print("(");
    Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
    Serial.print(answerChars[answer]);
    if (writer != NULL) {
        writer(Serial);
    }
    Serial.print(")");
}

bool Communicator::sendData(Answer answer, void (*writer)(Stream &stream)) {
    sendAnswer(answer, writer);
    return true;
}

bool Communicator::sendBinary(const uint8_t* data, uint16_t bytesCoutn) {
    uint16_t hash = 0;
    uint16_t i = 0;
    for (; i < bytesCoutn - 1; i += 2) {
        hash += data[i + 1] + ( data[i] << 8 );
    }
    if (i < bytesCoutn) {
        hash += data[i] << 8;
    }
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(bytesCoutn);
    Serial.write(hash);
    Serial.write(data, bytesCoutn);
    Serial.write(4);
    Serial.write(4);
    Serial.write(4);
    return true;
}
