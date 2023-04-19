#include "Particle.h"


SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);


static const int GPS_POWER_PIN = D6;
static const int GPS_BAUD = 9600;

void setup() {
    Serial.begin();
    Serial1.begin(GPS_BAUD);

    // Turn on GPS (pin D7)
    pinMode(GPS_POWER_PIN, OUTPUT);
    digitalWrite(GPS_POWER_PIN, LOW);
}

void loop() {
    if (true || Serial.isConnected()) {
        while(Serial1.available()) {
            Serial.write(Serial1.read());
        }        
        while(Serial.available()) {
            Serial1.write(Serial.read());
        }
    }
    else {
        // Discard data from the GPS
        while(Serial1.available()) {
            Serial1.read();
        }
    }
}

