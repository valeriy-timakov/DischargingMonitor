#include "Arduino.h"
#include "read.h"
#include "led.h"
#include "utils.h"
#include "EEPROMStorage.h"
#include "Wire.h"

#define CS_PIN 10




//TimeKeeper tk;
LedBlink led(A0);
Communicator communicator;
EEPROMStorage storage(CS_PIN);
Reader reader(storage);


void setup() {
    Serial.begin(9600);
    Wire.setClock(100000);
    Wire.begin();
    Serial.print("Starting...");
    //tk.init();
    led.init();
    storage.init();
    reader.init();
    Serial.println(" started!");
}


uint8_t informFormat = 0;
uint32_t informInterval = 0;
uint32_t lastInformTime = 0;


void writeInformInterval(Stream &stream) {
    stream.print(informInterval);
}

void writeInformFormat(Stream &stream) {
    stream.print(informFormat);
}

void setInformInterval(uint32_t value) {
    informInterval = value;
}

void setInformFormat(uint32_t value) {
    informFormat = value;
}

Stream *pStream;

void write(Data &currData) {
    pStream->print(Reader::deserializeMilli(currData.voltage, M_VOLTAGE), 3);
    pStream->print(',');
    pStream->print(Reader::deserializeMilli(currData.current, M_CURRENT), 3);
    pStream->print(',');
    pStream->print(currData.timestamp);
    pStream->print(';');
}
//void writeInform(Stream &stream);
void writeInform(Stream &stream) {
    pStream = &stream;
    storage.enumerate(write);
}




void loop() {
    led.loop();

    reader.loop();
    storage.loop();

    if (informInterval > 0 && (millis() - lastInformTime) >= informInterval) {
        Serial.println("Tti");
        while (storage.prepareData()) {
            Serial.println("Dpr");
            bool dataProcessed;

            if (informFormat == IF_BINARY) {
                Serial.println("Sb.");
                dataProcessed = communicator.sendBinary(storage.getBuffer(), storage.getBufferSize());
            } else {
                Serial.println("St.");
                dataProcessed = communicator.sendData(A_INFORM, writeInform);
            }

            if (dataProcessed) {
                Serial.println("Ss");
                storage.dataProcessed();
            }
            delay(1000);
        }
        Serial.println("Nmdl");
        lastInformTime = millis();
    }

    while (communicator.readCommand() > 0) {
        Instruction instruction = communicator.getInstruction();
        switch (instruction) {
            case SET_INFORM_INTERVAL:
                communicator.processIntValue(setInformInterval);
                break;
            case GET_INFORM_INTERVAL:
                communicator.sendAnswer(A_INFORM_INTERVAL, writeInformInterval);
                break;
            case SET_INFORM_FORMAT:
                communicator.processIntValue(setInformFormat);
                break;
            case GET_INFORM_FORMAT:
                communicator.sendAnswer(A_INFORM_INTERVAL, writeInformFormat);
                break;
        }

        reader.processInstruction(instruction, communicator);
    }
}


/*

#include "Arduino.h"
#include "OneWireHub.h"
#include "DS2502.h"
#include "ADS1X15.h"


constexpr uint8_t pin_onewire   { 2 };
constexpr uint8_t charger240W[3] = {0x32,0x34,0x30};
auto hub       = OneWireHub(pin_onewire);
auto dellCH    = DS2502( 0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A);

ADS1115 ADS(0x48);


void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(A0, OUTPUT);
    Serial.begin(9600);
    Serial.println(__FILE__);


    hub.attach(dellCH);
    dellCH.writeMemory(charger240W, sizeof(charger240W), 0x08);

}

void loop() {
    digitalWrite(A0, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(A0, LOW);    // turn the LED off by making the voltage LOW
    delay(800);                       // wait for a second
    Serial.println("Ping ...");


    hub.poll();
}

 */