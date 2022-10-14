#include "Arduino.h"
#include "DS2502Opt.h"
#include "read.h"
#include "led.h"


constexpr uint8_t pin_onewire   { 2 };
constexpr uint8_t charger240W[3] = {0x32,0x34,0x30};
LedBlink led(A0);


void setup() {
    led.init();
    initRead();
    //initDS5202(pin_onewire, 0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A);
    //writeDS5202Memory(charger240W, sizeof(charger240W), 0x08);
}


void loop() {
    led.idle();
    loopRead();
    //pollDS5202();
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