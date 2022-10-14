//
// Created by valti on 19.08.2022.
//

#include "Communicator.h"


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
        if (cmdBuff[CMD_ID_SIZE]  == 'r') {
            if (cmdBuff[CMD_ID_SIZE + 1] == 't') {
                return GET_TIME;
            } else if (cmdBuff[CMD_ID_SIZE + 1] == 'v') {
                return GET_VOLTAGE;
            } else if (cmdBuff[CMD_ID_SIZE + 1] == 'c') {
                return GET_CURRENT;
            }
        } else if (cmdBuff[CMD_ID_SIZE] == 's') {
            if (cmdBuff[CMD_ID_SIZE + 1] == 't') {
                return SYNC_TIME;
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
/*
void Communicator::send(Answer answer, Str &data, Str &commandId) {
    Serial.print("(");
    if (answer == ERROR) {
        Serial.print("Error");
    }
    Serial.write(commandId.data, commandId.len);
    switch (answer) {
        case CURRENT:
            Serial.print("c");
            break;
        case VOLTAGE:
            Serial.print("v");
            break;
        case TIME:
            Serial.print("t");
            break;
    }
    Serial.write(data.data, data.len);
    Serial.print(")");
}

void Communicator::send(Answer answer, Str &data) {
    uint8_t idSize = min(curCmdBuffPos, CMD_ID_SIZE);
    Str id = {cmdBuff, idSize};
    send(answer, data, id);
}
*/
void Communicator::send(Answer answer, void (*writer)(Stream &stream)) {
    send(answer, writer, NULL);
}

void Communicator::sendSuccess() {
    send(SUCCESS, NULL);
}

void Communicator::sendError(const char *message) {
    send(ERROR, NULL, message);
}

void Communicator::send(Answer answer, void (*writer)(Stream &), const char *data) {
    Serial.print("(");
    if (answer == ERROR) {
        Serial.print("Error");
    }
    Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE)) + 1;
    switch (answer) {
        case CURRENT:
            Serial.print("c");
            break;
        case VOLTAGE:
            Serial.print("v");
            break;
        case TIME:
            Serial.print("t");
            break;
    }
    if (writer != NULL) {
        writer(Serial);
    }
    if (data != NULL) {
        Serial.print(data);
    }
    Serial.print(")");

}
