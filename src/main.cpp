#include "Arduino.h"
#include "read.h"
#include "led.h"
#include "utils.h"
#include "EEPROMStorage.h"
#include "Wire.h"
#include "Informer.h"
#include "Log.h"

#define CS_PIN 10

TimeKeeper tk;
LedBlink led(A0);
Log logger(tk);
EEPROMStorage storage(CS_PIN, logger);
Reader reader(storage, logger, tk);
Informer informer(storage, logger);
Communicator communicator(informer, reader, storage, logger, tk);

void setup() {
    Serial.begin(9600);
    Wire.setClock(100000);
    Wire.begin();
    Serial.print("Starting...");
    tk.init();
    led.init();
    storage.init();
    reader.init();
    logger.init();
    Serial.println(" started!");
}


void loop() {
    led.loop();
    reader.loop();
    storage.loop();
    informer.loop();
    communicator.loop();
    logger.loop();
}
