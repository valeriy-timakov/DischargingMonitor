#include "Arduino.h"
#include "read.h"
#include "led.h"
#include "utils.h"
#include "EEPROMStorage.h"
#include "Wire.h"
#include "Informer.h"

#define CS_PIN 10

//TimeKeeper tk;
LedBlink led(A0);
EEPROMStorage storage(CS_PIN);
Reader reader(storage);
Informer informer(storage);
Communicator communicator(informer, reader);

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


void loop() {
    led.loop();
    reader.loop();
    storage.loop();
    informer.loop();
    communicator.loop();
}
