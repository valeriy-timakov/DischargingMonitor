//
// Created by valti on 03.08.2022.
//

#include "read.h"
#include "ADS.h"
#include "Communicator.h"


#define BUFF_SIZE 10
const float baseResistance = 0.1;
#define R1 9.94
#define R2 467.1
const float voltageDividerCfnt = (R1 + R2) / R1;


long lastRead = 0;
bool readCurrentMode = true;
uint16_t nextTimeout = 1000;
bool modeRequested = false;
uint32_t lastModeRequested = 0;
float currentsBuffer[BUFF_SIZE];
float voltagesBuffer[BUFF_SIZE];
uint8_t curPosition = 0;
uint32_t baseTime = 0;
uint64_t baseLocalTime = 0;
Communicator communicator;



void setupBlueToothConnection();




void initRead() {
    Serial.begin(9600); //Set BluetoothBee BaudRate to default baud rate 38400
//    Serial.println("Entered initRead");
    setupBlueToothConnection();
    Wire.setClock(100000);
    beginADS();

    for (uint8_t i = 0; i < BUFF_SIZE; i++) {
        currentsBuffer[i] = 0;
        voltagesBuffer[i] = 0;
    }
//    Serial.println("Exiting initRead");
}



void writeTimeStamp(Stream &stream) {
    uint32_t timestamp = baseTime + (millis() - baseLocalTime);
    stream.print(timestamp);
}

void writeValue(Stream &stream, float buff[]) {
    stream.print(buff[curPosition] * 1000, 0);
    stream.print(';');
    writeTimeStamp(stream);
}

void writeCurrentWithTimestamp(Stream &stream) {
    writeValue(stream, currentsBuffer);
}

void writeVoltageWithTimestamp(Stream &stream) {
    writeValue(stream, voltagesBuffer);
}

uint32_t atoi(const char* str, int len) {
    Serial.println('atoi');
    uint32_t ret = 0;
    for(uint8_t i = 0; i < len; ++i) {
        Serial.println(ret);
        Serial.println(str[i]);
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

void syncTime() {
    Serial.println("syncTime");
    char *data;
    size_t size = communicator.getData(&data);
    uint8_t t = size;
    Serial.println(t);
    if (size > 0) {
        Serial.write(data, size);
        Serial.println();
        baseTime = atoi(data, size);
        Serial.println(baseTime);
        communicator.sendSuccess();
    }
}

void loopRead() {
    if (modeRequested) {
        float value;
        if (getValueIfExists(&value)) {
            curPosition++;
            if (curPosition >= BUFF_SIZE) {
                curPosition = 0;
            }
            if (readCurrentMode) {
                currentsBuffer[curPosition] = value / baseResistance;
                nextTimeout = 200;
                readCurrentMode = false;
                Serial.print("current=");
                Serial.println(currentsBuffer[curPosition], 6);
            } else {
                voltagesBuffer[curPosition] = value * voltageDividerCfnt;
                nextTimeout = 1000;
                readCurrentMode = true;
                Serial.print("voltage=");
                Serial.println(voltagesBuffer[curPosition], 6);
            }
            modeRequested = false;
        } else if (millis() - lastModeRequested > 10000) {
            Serial.println("read value time out!!!");
            modeRequested = false;
        }
    } else {
        if (millis() - lastRead > nextTimeout) {
            bool requested;
            if (readCurrentMode) {
                requested = requestValue(0, ADS1X15_PGA_1_024V);
            } else {
                requested = requestValue(2, ADS1X15_PGA_1_024V);
            }
            if (requested) {
                lastRead = millis();
                modeRequested = true;
                lastModeRequested = millis();
            } else {
                Serial.println("Error request value!");
            }
        }
    }

    while (communicator.readCommand() > 0) {
        switch (communicator.getInstruction()) {
            case GET_CURRENT:
                communicator.send(Answer::CURRENT, writeCurrentWithTimestamp);
                break;
            case GET_VOLTAGE:
                communicator.send(Answer::VOLTAGE, writeVoltageWithTimestamp);
                break;
            case GET_TIME:
                communicator.send(Answer::TIME, writeTimeStamp);
                break;
            case SYNC_TIME:
                syncTime();
                break;
        }
    }
}



void setupBlueToothConnection() {
    Serial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
    Serial.print("\r\n+STNA=HC-05\r\n"); //set the bluetooth name as "HC-05"
    Serial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
    Serial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here

    delay(2000); // This delay is required.
    //Serial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable
    Serial.print("bluetooth connected!\n");

    delay(2000); // This delay is required.
    Serial.flush();
}
