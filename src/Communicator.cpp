//
// Created by valti on 19.08.2022.
//

#include "Communicator.h"
#include "utils.h"



ErrorCode errorCode;

void Communicator::loop() {
    if (formatChanged) {
        if (format == F_TEXT) {
            Serial.println("(Ft)");
        } else {
            Serial.write(0);
            Serial.write(IC_FORMAT_CHANGED);
            Serial.write(F_BINARY);
        }
        formatChanged = false;
    }
    if (format == F_TEXT) {
        while (readTextCommand()) {
            processTextInstruction();
        }
    } else {
        while (readBinaryCommand()) {
            processBinaryInstruction();
        }
    }
}

void sendError_(ErrorCode errorCode) {
    Serial.print("(E");
    Serial.print(errorCode);
    Serial.print(")");
}

bool Communicator::readTextCommand() {
    commandParsed = false;
    while (Serial.available()) {
        char received = Serial.read();
        if (startDetected) {
            switch (received) {
                case COMMAND_WITHOUT_ADDR_START_CHAR:
                case COMMAND_START_CHAR:
                    startDetected = false;
                    curCmdBuffPos = 0;
                    Serial.print("(E");
                    Serial.print(E_NEW_COMMAND_INSIDE);
                    Serial.print(": ");
                    if (curCmdBuffPos > 0) {
                        Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
                    }
                    Serial.print(")");
                    Serial.print(received);
                    break;
                case COMMAND_WITHOUT_ADDR_END_CHAR:
                case COMMAND_END_CHAR:
                    startDetected = false;
                    if (curCmdBuffPos > 0) {
                        commandParsed = true;
                        return true;
                    } else {
                        sendError_(E_COMMAND_EMPTY);
                    }
                    break;
                default:
                    if (curCmdBuffPos < CMD_BUFF_SIZE - 1) {
                        cmdBuff[curCmdBuffPos] = received;
                        curCmdBuffPos++;
                    } else {
                        Serial.print("(E");
                        Serial.print(E_COMMAND_SIZE_OVERFLOW);
                        Serial.print(")");
                        startDetected = false;
                    }
            }
        } else {
            if (received == COMMAND_WITHOUT_ADDR_START_CHAR || received == COMMAND_START_CHAR) {
                commandWithAddress = received == COMMAND_START_CHAR;
                curCmdBuffPos = 0;
                commandParsed = false;
                startDetected = true;
            } else if (received == COMMAND_END_CHAR) {
                sendError_(E_COMMAND_END_WITHOUT_START);
            } else {
                Serial.print(received);
            }
        }
    }
    return false;
}


const Data * _tmpData;
float (*deserializer)(const Data *);

void writeData(Communicator *self, Stream &stream) {
    stream.print(deserializer(_tmpData), 3);
    stream.print(',');
    stream.print(_tmpData->timestamp);
}

/**
 * Commands
 * First char:
 * r - read
 * s - set
 * e - execute
 * Second char (for read/set):
 * t - current timestamp
 * v - avgData measure voltage with timestamp, V/ms
 * c - avgData measure current with timestamp, A/ms
 * i - inform interval, ms
 * l - last read data timestamp, ms
 * r - measurements interval, ms
 * f - inform format, text/binary
 * n - inform data coefficients
 * o - inform values order
 * a - voltage permissible variation, dimensionless int data
 * b - current permissible variation, dimensionless int data
 * d - logging is enabled
 * g - log register values
 * h - err register values
 * j - external timestamp ID
 * k - storage state
 * p - avgData prepared data timestamp
 * s - avgData saved data timestamp
 * Second char (for execute):
 * r - force measurement
 * i - force inform
 * p - mark avgData inform package successfully proceeded
 * e - inform of avgData inform package proceeding error
 * No data results or inform messages:
 * S - success
 * E - error (with error code)
 * I - inform (with data)
 * F - format changed
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
void Communicator::processTextInstruction() {
    bool proceeded = true;
    uint8_t pos = commandWithAddress ? CMD_ID_SIZE : 0;
    if (curCmdBuffPos >= pos + CMD_INSTR_SIZE) {
        char instrFirst = cmdBuff[pos];
        char instrSecond = cmdBuff[pos + 1];
        if (instrFirst  == 'r') {
            if (instrSecond == 't') {
                sendAnswer('t', [](Communicator *self, Stream &stream) { stream.print(self->timeKeeper.getCurrent() ); });
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
                sendAnswer('i', [](Communicator *self, Stream &stream) { stream.print( self->informer.getInformInterval() ); });
            } else if (instrSecond == 'l') {
                sendAnswer('l', [](Communicator *self, Stream &stream) { stream.print( self->reader.getLastReadTimeStamp() ); });
            } else if (instrSecond == 'r') {
                sendAnswer('r', [](Communicator *self, Stream &stream) { stream.print( self->reader.getReadInterval() ); });
            } else if (instrSecond == 'f') {
                sendAnswer('f', [](Communicator *self, Stream &stream) { stream.print(self->informer.getInformFormat() == F_BINARY ? "b" : "t"); });
            } else if (instrSecond == 'n') {
                sendAnswer('n', [](Communicator *self, Stream &stream) { self->informer.writeInformCoefficients(stream); });
            } else if (instrSecond == 'o') {
                sendAnswer('o', [](Communicator *self, Stream &stream) { self->informer.writeInformOrder(stream); });
            } else if (instrSecond == 'a') {
                sendAnswer('a', [](Communicator *self, Stream &stream) { stream.print( self->storage.getDataPermissibleVariation(DCC_VOLTAGE) ); });
            } else if (instrSecond == 'b') {
                sendAnswer('b', [](Communicator *self, Stream &stream) { stream.print( self->storage.getDataPermissibleVariation(DCC_CURRENT) ); });
            } else if (instrSecond == 'd') {
                sendAnswer('d', [](Communicator *self, Stream &stream) { stream.print( self->log.isLogEnabled() ); });
            } else if (instrSecond == 'p') {
                sendAnswer('p', [](Communicator *self, Stream &stream) { stream.print( self->storage.getLastPreparedTimestamp() ); });
            } else if (instrSecond == 's') {
                sendAnswer('s', [](Communicator *self, Stream &stream) { stream.print( self->storage.getLastSavedTimestamp() ); });
            } else if (instrSecond == 'j') {
                sendAnswer('j', [](Communicator *self, Stream &stream) { stream.print( (unsigned long) self->timeKeeper.getCurrentId() ); });
            } else if (instrSecond == 'g') {
                sendAnswer('g', [](Communicator *self, Stream &stream) {
                    stream.print( self->log.getCountingStartTimestamp());
                    stream.print(";");
                    self->log.getErrBuffer().print(stream);
                });
            } else if (instrSecond == 'h') {
                sendAnswer('h', [](Communicator *self, Stream &stream) {
                    stream.print( self->log.getCountingStartTimestamp());
                    stream.print(";");
                    self->log.getLogBuffer().print(stream);
                });
            } else if (instrSecond == 'k') {
                sendAnswer('k', [](Communicator *self, Stream &stream) { self->storage.printState(stream); });
            } else {
                proceeded = false;
            }
        } else if (instrFirst == 's') {
            if (instrSecond == 't') {
                processIntValue([](Communicator *self, uint32_t value) { self->timeKeeper.syncTime(value); return OK; });
            } else if (instrSecond == 'i') {
                processIntValue([](Communicator *self, uint32_t value) { self->informer.setInformInterval(value); return OK; });
            } else if (instrSecond == 'r') {
                processIntValue([](Communicator *self, uint32_t value) { self->reader.setReadInterval(value); return OK; });
            } else if (instrSecond == 'f') {
                processIntValue([](Communicator *self, uint32_t value) { self->informer.setInformFormat( value > 0 ? F_BINARY : F_TEXT); return OK; });
            } else if (instrSecond == 'a') {
                processIntValue([](Communicator *self, uint32_t value) { self->storage.setDataPermissibleVariation(DCC_VOLTAGE, value); return OK; });
            } else if (instrSecond == 'b') {
                processIntValue([](Communicator *self, uint32_t value) { self->storage.setDataPermissibleVariation(DCC_CURRENT, value); return OK; });
            } else if (instrSecond == 'd') {
                processIntValue([](Communicator *self, uint32_t value) { self->log.setLogEnabled(value > 0); return OK; });
            } else if (instrSecond == 'j') {
                processIntValue([](Communicator *self, uint32_t value) { self->timeKeeper.setCurrentId(value); return OK; });
            } else {
                proceeded = false;
            }
        } else if (instrFirst == 'e') {
            ErrorCode result;
            if (instrSecond == 'r') {
                result = reader.performRead();
            } else if (instrSecond == 'i') {
                result = informer.inform();
            } else if (instrSecond == 'p') {
                result = informer.proceeded();
            } else if (instrSecond == 'e') {
                result = informer.proceedError();
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
        } else if (instrFirst == 'b') {
            format = F_BINARY;
            formatChanged = true;
            informer.setInformFormat(F_BINARY);
        }
    }
    if (!proceeded) {
        sendError(E_INSTRUCTION_UNRECOGIZED);
    }
}

size_t Communicator::getData(char **res) {
    uint8_t pos = commandWithAddress ? CMD_ID_SIZE : 0;
    uint8_t dataStartPos = pos + CMD_INSTR_SIZE;
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
    if (commandWithAddress) {
        Serial.print("(");
        Serial.write(cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
    } else {
        Serial.print("[");
    }
    Serial.print(answerCodeChar);
    if (writer != NULL) {
        writer(this, Serial);
    }
    if (commandWithAddress) {
        Serial.print(")");
    } else {
        Serial.print("]");
    }
}

bool Communicator::readBinaryCommand() {
    uint8_t available = Serial.available();
    if (available) {
        if (Serial.read() != IC_NONE) {
            Serial.write(IC_NONE);
            Serial.write(IC_ERROR);
            Serial.write(E_INSTRUCTION_WRONG_START);
            return false;
        }
        curCmdBuffPos = 0;
        commandParsed = false;
        available = Serial.available();
        if (available == 0) {
            Serial.write(IC_NONE);
            Serial.write(IC_ERROR);
            Serial.write(E_COMMAND_EMPTY);
            return false;
        }
        if (available > CMD_BUFF_SIZE) {
            while (Serial.available()) Serial.read();
            Serial.write(IC_NONE);
            Serial.write(IC_ERROR);
            Serial.write(E_COMMAND_SIZE_OVERFLOW);
            return false;
        }
        Serial.readBytes(cmdBuff, available);
        curCmdBuffPos += available;
        bool commandUnrecognized = false;
        if (curCmdBuffPos < 2) {
            commandUnrecognized = true;
        } else {
            uint8_t firstCode = cmdBuff[0];
            commandUnrecognized = firstCode != IC_READ && firstCode != IC_SET && firstCode != IC_EXECUTE && firstCode != IC_FORMAT_CHANGED;
        }
        if (commandUnrecognized) {
            Serial.write(IC_NONE);
            Serial.write(IC_ERROR);
            Serial.write(E_INSTRUCTION_UNRECOGIZED);
            return false;
        }
        commandParsed = true;
        return true;

    }
    return false;
}

void Communicator::processBinaryInstruction() {

}

