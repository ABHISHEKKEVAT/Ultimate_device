#include <Arduino.h>
#include "SIM800L.h"

// Initialize the SIM800L module
SIM800L sim800(2, 3); // RX, TX

void setup() {
    Serial.begin(115200);
    sim800.begin(9600);
    sim800.initialize();
    sim800.connectGPRS("airtelgprs.com"); // Replace with your network's APN
}

void loop() {
    sim800.handleIncomingMessages();
}
