//
// Created by valti on 19.08.2022.
//

#include "Communicator.h"
#include "utils.h"



ErrorCode errorCode;

void Communicator::loop() {
    while (readCommand()) {
        processInstruction();
    }
}

bool Communicator::readCommand() {
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
                        return true;
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
    return false;
}

/*
bool Communicator::getCommandId(char **res) {
    if (commandParsed) {
        *res = cmdBuff;
        return min(curCmdBuffPos, CMD_ID_SIZE);
    }
    return 0;
}
*/

const Data * _tmpData;
float (*deserializer)(const Data *);

void writeData(Communicator *self, Stream &stream) {
    stream.print(deserializer(_tmpData), 3);
    stream.print(',');
    stream.print(_tmpData->timestamp);
}

void Communicator::processInstruction() {
    /**
     * Commands
     * First char:
     * r - read
     * s - set
     * e - execute
     * Second char (for read/set):
     * t - current timestamp
     * v - last measure voltage with timestamp, V/ms
     * c - last measure current with timestamp, A/ms
     * i - inform interval, ms
     * l - last measure timestamp, ms
     * r - measurements interval, ms
     * f - inform format, text/binary
     * n - inform data coefficients
     * o - inform values order
     * Second char (for execute):
     * r - force measurement
     * i - force inform
     * p - mark last inform package successfully proceeded
     * e - inform of last inform package proceeding error
     * No data results:
     * S - success
     * E - error (with error code)
     * Errors:
     *   OK:0 - "Ok result, no error.";
     *   E_REQUEST_DATA_NO_VALUE:1 - No value
     *   E_REQUEST_DATA_NOT_DIGITAL_VALUE:2 - Not digital value
     *   E_SENSOR_READ_TIME_OUT:3 - Time out waiting answer from ADC!
     *   E_SENSOR_READ_REQUEST_FAILED:4 - Error requesting measurement!
     *   E_SENSOR_READ_ALREADY_IN_PROGRESS:5 - Measurement is already in progress!
     *   E_INFORM_NO_DATA_TO_SEND:6 - All available data was sent by inform calls or no data pages were saved!
     *   E_INFORM_PACKAGE_ALREADY_SENT:7 - Package already sent and not proceeded yet!
     *   E_INSTRUCTION_UNRECOGIZED:8 - No command bound for this instruction!
     *   E_READ_NO_DATA_TO_SEND:9 - No one measurement was performed, so not data available for sent!
     *   E_NO_PACKAGE_WAS_SENT:10 - No inform package was sent to inform about its proceeding!
     *   E_UNDEFINED_CODE:11 - "Code for undefined errors";
     */
    bool proceeded = true;
    if (curCmdBuffPos >= CMD_ID_SIZE + CMD_INSTR_SIZE) {
        char instrFirst = cmdBuff[CMD_ID_SIZE];
        char instrSecond = cmdBuff[CMD_ID_SIZE + 1];
        if (instrFirst  == 'r') {
            if (instrSecond == 't') {
                sendAnswer('t', [](Communicator *self, Stream &stream) { self->reader.writeTimeStamp(stream); });
            } else if (instrSecond == 'v' || instrSecond == 'c') {
                const Data& tmpData = reader.getLastData();
                if (tmpData.available()) {
                    _tmpData = &tmpData;
                    deserializer = instrSecond == 'v' ? reader.deserializeVoltage : reader.deserializeCurrent;
                    sendAnswer('v', writeData);
                } else {
                    sendError(E_READ_NO_DATA_TO_SEND);
                }
            } else if (instrSecond == 'i') {
                sendAnswer('i', [](Communicator *self, Stream &stream) { self->informer.writeInformInterval(stream); });
            } else if (instrSecond == 'l') {
                sendAnswer('l', [](Communicator *self, Stream &stream) { self->reader.writeLastReadTimeStamp(stream); });
            } else if (instrSecond == 'r') {
                sendAnswer('r', [](Communicator *self, Stream &stream) { self->reader.writeReadInterval(stream); });
            } else if (instrSecond == 'f') {
                sendAnswer('f', [](Communicator *self, Stream &stream) { self->informer.writeInformFormat(stream); });
            } else if (instrSecond == 'n') {
                sendAnswer('n', [](Communicator *self, Stream &stream) { self->informer.writeInformCoefficients(stream); });
            } else if (instrSecond == 'o') {
                sendAnswer('o', [](Communicator *self, Stream &stream) { self->informer.writeInformOrder(stream); });
            } else {
                proceeded = false;
            }
        } else if (instrFirst == 's') {
            if (instrSecond == 't') {
                processIntValue([](Communicator *self, uint32_t value) { return self->reader.syncTime(value); });
                logMem();
            } else if (instrSecond == 'i') {
                logMem();
                processIntValue([](Communicator *self, uint32_t value) { return self->informer.setInformInterval(value); });
                logMem();
            } else if (instrSecond == 'r') {
                processIntValue([](Communicator *self, uint32_t value) { return self->reader.setReadInterval(value); });
                logMem();
            } else if (instrSecond == 'f') {
                processIntValue([](Communicator *self, uint32_t value) { return self->informer.setInformFormat(value); });
                logMem();
            } else {
                proceeded = false;
            }
        } else if (instrFirst == 'e') {
            ErrorCode result;
            if (instrSecond == 'r') {
                result = reader.performRead();
                logMem();
            } else if (instrSecond == 'i') {
                result = informer.inform(*this);
                logMem();
            } else if (instrSecond == 'p') {
                result = informer.proceeded();
                logMem();
            } else if (instrSecond == 'e') {
                result = informer.proceedError();
                logMem();
            } else {
                proceeded = false;
            }
            if (proceeded) {
                if (result == OK) {
                    sendSuccess();
                } else {
                    sendError(result);
                }
            }
        }
    }
    if (!proceeded) {
        sendError(E_INSTRUCTION_UNRECOGIZED);
    }
}

void Communicator::sendErrorIfNotSuccess(ErrorCode code) {
    if (code != OK) {
        sendError(code);
    }
}

size_t Communicator::getData(char **res) {
    uint8_t dataStartPos = CMD_ID_SIZE + CMD_INSTR_SIZE;
    if (commandParsed && curCmdBuffPos > dataStartPos) {
        *res = cmdBuff + dataStartPos;
        return curCmdBuffPos - dataStartPos;
    }
    return false;
}

void Communicator::processIntValue(ErrorCode (*processor)(Communicator *self, uint32_t)) {
    char *data;
    size_t size = getData(&data);
    ErrorCode code;
    if (size > 0) {
        long tmpValue = atoi(data, size);
        if (tmpValue == -1) {
            code = E_REQUEST_DATA_NOT_DIGITAL_VALUE;
        } else {
            code = processor(this, (uint32_t) tmpValue);
        }
    } else {
        code = E_REQUEST_DATA_NO_VALUE;
    }
    if (code == OK) {
        sendSuccess();
    } else {
        sendError(code);
    }
}

void Communicator::sendSuccess() {
    sendAnswer('S', NULL);
}

void Communicator::sendError(ErrorCode code) {
    errorCode = code;
    sendAnswer('E', [] (Communicator *self, Stream &stream) {
        stream.print(errorCode);
    });
}

void Communicator::sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream)) {
    Serial.print("(");
    Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
    Serial.print(answerCodeChar);
    if (writer != NULL) {
        writer(this, Serial);
    }
    Serial.print(")");
}

void Communicator::sendData(char answerCodeChar, void (*writer)(Stream &stream)) {
    Serial.print("(");
    Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
    Serial.print(answerCodeChar);
    if (writer != NULL) {
        writer(Serial);
    }
    Serial.print(")");
}

void Communicator::sendBinary(const uint8_t* data, uint16_t bytesCount) {
    uint16_t hash = 0;
    uint16_t i = 0;
    for (; i < bytesCount - 1; i += 2) {
        hash += data[i + 1] + ( data[i] << 8 );
    }
    if (i < bytesCount) {
        hash += data[i] << 8;
    }
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    Serial.write(bytesCount);
    Serial.write(hash);
    Serial.write(data, bytesCount);
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    Serial.write(1);
}
