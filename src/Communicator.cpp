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
            sendSerial(IC_NONE);
            sendSerial(IC_FORMAT_CHANGED);
            sendSerial(F_BINARY);
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
                        sendSerial((uint8_t*) cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
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

void writeData(const Data *data, float (*deserializer)(const Data *), Stream &stream) {
    stream.print(deserializer(data), 3);
    stream.print(',');
    stream.print(data->timestamp);
}

/**
 * Commands
 * First char:
 * r - read
 * s - set
 * e - execute
 * Second char (for read/set):
 * a - voltage permissible variation, dimensionless int data
 * b - current permissible variation, dimensionless int data
 * c - avgData measure current with timestamp, A/ms
 * d - logging is enabled
 * e - get saved page
 * f - inform format, text/binary
 * g - err register values
 * h - log register values
 * i - inform interval, ms
 * j - external timestamp ID
 * k - storage state
 * l - last read data timestamp, ms
 * m - get not saved data
 * n - inform data coefficients
 * o - inform values order
 * p - avgData prepared data timestamp
 * q -
 * r - measurements interval, ms
 * s - avgData saved data timestamp
 * t - current timestamp
 * u -
 * v - avgData measure voltage with timestamp, V/ms
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
            } else if (instrSecond == 'v' || instrSecond == 'c' || instrSecond == 'l') {
                if (storage.getLast().available()) {
                    if (instrSecond == 'l') {
                        sendAnswer('l', [](Communicator *self, Stream &stream) {
                            stream.print( self->storage.getLast().timestamp );
                        });
                    } else if (instrSecond == 'v') {
                        sendAnswer('v', [](Communicator *self, Stream &stream) {
                            writeData( &self->storage.getLast(), self->reader.deserializeVoltage, stream);
                        });
                    } else if (instrSecond == 'c') {
                        sendAnswer('c', [](Communicator *self, Stream &stream) {
                            writeData( &self->storage.getLast(), self->reader.deserializeCurrent, stream);
                        });
                    }
                } else {
                    sendError(E_READ_NO_DATA_TO_SEND);
                }
            } else if (instrSecond == 'i') {
                sendAnswer('i', [](Communicator *self, Stream &stream) { stream.print( self->informer.getInformInterval() ); });
            } else if (instrSecond == 'r') {
                sendAnswer('r', [](Communicator *self, Stream &stream) { stream.print( self->reader.getReadInterval() ); });
            } else if (instrSecond == 'f') {
                sendAnswer('f', [](Communicator *self, Stream &stream) { stream.print(self->informer.getInformFormat() == F_TEXT ? "t" : "b"); });
            } else if (instrSecond == 'n') {
                sendAnswer('n', [](Communicator *self, Stream &stream) { self->reader.printInformCoefficients(stream); });
            } else if (instrSecond == 'o') {
                sendAnswer('o', [](Communicator *self, Stream &stream) { self->reader.printInformOrder(stream); });
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
            } else if (instrSecond == 'm') {
                sendAnswer('m', [](Communicator *self, Stream &stream) { self->storage.printNotSaved(stream); });
            } else if (instrSecond == 'e') {
                processIntValue([](Communicator *self, uint32_t value) {
                    self->sendAnswer('e', [](Communicator *self, Stream &stream, void *valuePtr) {
                        uint32_t value = *((uint32_t *) valuePtr);
                        uint8_t force = (uint8_t) (value >> 16);
                        uint16_t page = (uint16_t) value;
                        if (self->storage.loadPage(page, (bool)force)) {
                            self->storage.printNextSavedDataPage(stream, 'e');
                        } else {
                            self->sendError(E_PAGE_NOT_READY);
                        }
                    }, &value);
                    return E_UNDEFINED_CODE;
                });
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
        } else if (instrFirst == 'f') {
            processIntValue([](Communicator *self, uint32_t value) {
                self->format = value == 0 ? F_TEXT : F_BINARY;
                self->formatChanged = true;
                self->informer.setInformFormat(self->format);
                return OK;
            });
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
    return 0;
}

void Communicator::processIntValue(ErrorCode (*processor)(Communicator *self, uint32_t)) {
    char *data;
    size_t size = getData(&data);
    ErrorCode code;
    if (size > 0) {
        uint32_t tmpValue = 0;
        if (atou(data, size, tmpValue)) {
            code = processor(this, tmpValue);
        } else {
            code = E_REQUEST_DATA_NOT_DIGITAL_VALUE;
        }
    } else {
        code = E_REQUEST_DATA_NO_VALUE;
    }
    if (code == OK) {
        sendSuccess();
    } else if (code != E_UNDEFINED_CODE) {
        sendError(code);
    }
}

void Communicator::sendSuccess() {
    sendAnswer('S');
}

void Communicator::sendError(ErrorCode code) {
    errorCode = code;
    sendAnswer('E', [] (Communicator *self, Stream &stream) {
        stream.print(errorCode);
    });
}

void Communicator::sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream, void *additionalData), void *additionalData) {
    startAnswer(answerCodeChar);
    writer(this, Serial, additionalData);
    endAnswer();
}

void Communicator::sendAnswer(char answerCodeChar, void (*writer)(Communicator *self, Stream &stream)) {
    startAnswer(answerCodeChar);
    writer(this, Serial);
    endAnswer();
}

void Communicator::sendAnswer(char answerCodeChar) {
    startAnswer(answerCodeChar);
    endAnswer();
}

void Communicator::endAnswer() const {
    if (commandWithAddress) {
        Serial.print(")");
    } else {
        Serial.print("]");
    }
}

void Communicator::startAnswer(char answerCodeChar) const {
    if (commandWithAddress) {
        Serial.print("(");
        sendSerial((uint8_t*) cmdBuff, min(curCmdBuffPos, CMD_ID_SIZE));
    } else {
        Serial.print("[");
    }
    Serial.print(answerCodeChar);
}

bool Communicator::readBinaryCommand() {
    uint8_t available = Serial.available();
    if (!available) {
        return false;
    }

    uint32_t currTime = (uint32_t) millis();
    bool commandReady = available - lastPacketSize == 0 || (currTime - lastPacketTime) > MAX_COMMAND_READ_TIME;
    lastPacketSize = available;
    lastPacketTime = currTime;
    if (!commandReady) {
        return false;
    }
    if (Serial.read() != IC_NONE) {
        sendSerial(IC_NONE);
        sendSerial(IC_ERROR);
        sendSerial(E_INSTRUCTION_WRONG_START);
        return false;
    }
    curCmdBuffPos = 0;
    commandParsed = false;
    available = Serial.available();
    if (available == 0) {
        sendSerial(IC_NONE);
        sendSerial(IC_ERROR);
        sendSerial(E_COMMAND_EMPTY);
        return false;
    }
    if (available > CMD_BUFF_SIZE) {
        while (Serial.available()) Serial.read();
        sendSerial(IC_NONE);
        sendSerial(IC_ERROR);
        sendSerial(E_COMMAND_SIZE_OVERFLOW);
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
        sendSerial(IC_NONE);
        sendSerial(IC_ERROR);
        sendSerial(E_INSTRUCTION_UNRECOGIZED);
        return false;
    }
    commandParsed = true;
    return true;
}

void Communicator::processBinaryInstruction() {
    uint8_t mainCode = cmdBuff[0];
    uint8_t instrCode = cmdBuff[1];
    ErrorCode result;
    switch (mainCode) {
        case IC_READ:
            result = processBinaryRead(instrCode);
            break;
        case IC_SET:
            if (curCmdBuffPos < 3) {
                result = E_REQUEST_DATA_NO_VALUE;
                return;
            }
            result = processBinarySet(instrCode);
            if (result == OK) {
                sendSuccess();
            }
            break;
        case IC_EXECUTE:
            result = processBinaryExecute(instrCode);
            if (result == OK) {
                sendSuccess();
            }
            break;
        case IC_FORMAT_CHANGED:
            if (curCmdBuffPos < 3) {
                sendError(E_REQUEST_DATA_NO_VALUE);
                return;
            }
            format = getByteFromData() == 0 ? F_TEXT : F_BINARY;
            informer.setInformFormat(format);
            formatChanged = true;
            result = OK;
            break;
    }
    if (result != OK) {
        sendError(result);
    }
}

void startBinaryAnswer(InstructionDataCode code) {
    sendSerial(IC_NONE);
    sendSerial(code);
}

ErrorCode sendDataIfAvailable(const Data &data, InstructionDataCode code) {
    if (data.available()) {
        startBinaryAnswer(code);
        sendSerial(data.current);
        sendSerial(data.voltage);
        sendSerial(data.timestamp);
        return OK;
    } else {
        return E_READ_NO_DATA_TO_SEND;
    }
}

ErrorCode Communicator::processBinaryRead(uint8_t code) {
    switch (code) {
        case IDC_CURRENT: // c
        case IDC_VOLTAGE: // v
        case IDC_LAST_READ_TIME: //l
            return sendDataIfAvailable(storage.getLast(), (InstructionDataCode) code);
        case IDC_TIME:  //t
            startBinaryAnswer(IDC_TIME);
            sendSerial(timeKeeper.getCurrent());
            return OK;
        case IDC_INFORM_INTERVAL:   //i
            startBinaryAnswer(IDC_INFORM_INTERVAL);
            sendSerial(informer.getInformInterval());
            return OK;
        case IDC_MEASUREMENTS_INTERVAL: //r
            startBinaryAnswer(IDC_MEASUREMENTS_INTERVAL);
            sendSerial(reader.getReadInterval());
            return OK;
        case IDC_VOLTAGE_PERMISSIBLE_VARIATION: //a
            startBinaryAnswer(IDC_VOLTAGE_PERMISSIBLE_VARIATION);
            sendSerial(storage.getDataPermissibleVariation(DCC_VOLTAGE));
            return OK;
        case IDC_CURRENT_PERMISSIBLE_VARIATION: //b
            startBinaryAnswer(IDC_CURRENT_PERMISSIBLE_VARIATION);
            sendSerial(storage.getDataPermissibleVariation(DCC_CURRENT));
            return OK;
        case IDC_LOG_ENABLED: //d
            startBinaryAnswer(IDC_LOG_ENABLED);
            sendSerial(log.isLogEnabled());
            return OK;
        case IDC_INFORM_DATA_COEFFICIENTS: //n
            startBinaryAnswer(IDC_INFORM_DATA_COEFFICIENTS);
            reader.writeInformCoefficients(Serial);
            return OK;
        case IDC_LOG_REGISTER_VALUES: //h
            startBinaryAnswer(IDC_LOG_REGISTER_VALUES);
            sendSerial(log.getCountingStartTimestamp());
            log.getLogBuffer().write(Serial);
            return OK;
        case IDC_ERR_REGISTER_VALUES: //g
            startBinaryAnswer(IDC_ERR_REGISTER_VALUES);
            sendSerial(log.getCountingStartTimestamp());
            log.getErrBuffer().write(Serial);
            return OK;
        case IDC_AVG_DATA_PREPARED_DATA_TIMESTAMP: //p
            startBinaryAnswer(IDC_AVG_DATA_PREPARED_DATA_TIMESTAMP);
            sendSerial(storage.getLastPreparedTimestamp());
            return OK;
        case IDC_AVG_DATA_SAVED_DATA_TIMESTAMP: //s
            startBinaryAnswer(IDC_AVG_DATA_SAVED_DATA_TIMESTAMP);
            sendSerial(storage.getLastSavedTimestamp());
            return OK;
        case IDC_EXTERNAL_TIMESTAMP_ID: //j
            startBinaryAnswer(IDC_EXTERNAL_TIMESTAMP_ID);
            sendSerial(timeKeeper.getCurrentId());
            return OK;
        case IDC_STORAGE_STAGE_DUMP: //k
            startBinaryAnswer(IDC_STORAGE_STAGE_DUMP);
            storage.writeState(Serial);
            return OK;
        case IDC_NOT_SAVED_DATA: //m
            startBinaryAnswer(IDC_NOT_SAVED_DATA);
            storage.writeNotSaved(Serial);
            return OK;
        case IDC_DATA_PAGE: //e
            uint16_t page = 0;
            uint8_t force = 0;
            if (storage.loadPage(page, force)) {
                storage.writeNextSavedDataPage(Serial, IDC_DATA_PAGE);
                return OK;
            } else {
                return E_PAGE_NOT_READY;
            }
            startBinaryAnswer(IDC_NOT_SAVED_DATA);
            storage.writeNotSaved(Serial);
            return OK;
    }
    return E_UNDEFINED_OPERATION;
}

ErrorCode Communicator::processBinarySet(uint8_t code) {
    switch (code) {
        case IDC_TIME: //t
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            timeKeeper.syncTime(getIntFromData());
            return OK;
        case IDC_INFORM_INTERVAL: //i
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            informer.setInformInterval(getIntFromData());
            return OK;
        case IDC_MEASUREMENTS_INTERVAL: //r
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            reader.setReadInterval(getIntFromData());
            return OK;
        case IDC_VOLTAGE_PERMISSIBLE_VARIATION: //a
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            storage.setDataPermissibleVariation(DCC_VOLTAGE, getIntFromData());
            return OK;
        case IDC_CURRENT_PERMISSIBLE_VARIATION: //b
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            storage.setDataPermissibleVariation(DCC_CURRENT, getIntFromData());
            return OK;
        case IDC_LOG_ENABLED: //d
            if (curCmdBuffPos < 3) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            log.setLogEnabled(getByteFromData());
            return OK;
        case IDC_EXTERNAL_TIMESTAMP_ID: //j
            if (curCmdBuffPos < 6) {
                return E_REQUEST_DATA_NO_VALUE;
            }
            timeKeeper.setCurrentId(getIntFromData());
            return OK;
    }
    return E_UNDEFINED_OPERATION;
}

ErrorCode Communicator::processBinaryExecute(uint8_t code) {
    switch (code) {
        case IO_FORCE_INFORM:
            return informer.inform();
        case IO_FORCE_MEASUREMENT:
            return reader.performRead();
        case IO_MARK_INFORM_HANDLED:
            return reader.performRead();
        case IO_MARK_INFORM_HANDLE_ERROR:
            return reader.performRead();
    }
    return E_UNDEFINED_OPERATION;
}

uint8_t Communicator::getByteFromData() {
    if (curCmdBuffPos < 3) {
        return 0;
    }
    return cmdBuff[2];
}


uint16_t Communicator::getShortFromData() {
    if (curCmdBuffPos < 4) {
        return 0;
    }
    uint16_t res = 0;
    for (int i = 0; i < 2; i++) {
        ((uint8_t*)&res)[i] = cmdBuff[2 + i];
    }
    return res;
}

uint32_t Communicator::getIntFromData() {
    if (curCmdBuffPos < 6) {
        return 0;
    }
    uint32_t res = 0;
    for (int i = 0; i < 4; i++) {
        ((uint8_t*)&res)[i] = cmdBuff[2 + i];
    }
    return res;
}

uint64_t Communicator::getLongFromData() {
    if (curCmdBuffPos < 10) {
        return 0;
    }
    uint64_t res = 0;
    for (int i = 0; i < 8; i++) {
        ((uint8_t*)&res)[i] = cmdBuff[2 + i];
    }
    return res;
}
