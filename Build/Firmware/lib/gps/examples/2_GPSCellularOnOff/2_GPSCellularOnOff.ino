
#include "Particle.h"

// Port of TinyGPS for the Particle AssetTracker
// https://github.com/mikalhart/TinyGPSPlus
#include "TinyGPS++.h"

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

// Forward declarations
void displayInfo();
void buttonHandler(system_event_t event, int data);

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

enum State { IDLE_DISCONNECTED_STATE, CONNECT_WAIT_STATE, IDLE_CONNECTED_STATE, DISCONNECT_WAIT_STATE };

TinyGPSPlus gps;
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
bool connectStateDesired = false;
State state = IDLE_DISCONNECTED_STATE;


void setup()
{
	Serial.begin(9600);

	System.on(button_click, buttonHandler);

	// The GPS module on the AssetTracker is connected to Serial1 and D6
	Serial1.begin(9600);

	// Settings D6 LOW powers up the GPS module
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
}

void loop()
{
	while (Serial1.available() > 0) {
		if (gps.encode(Serial1.read())) {
			displayInfo();
		}
	}

	switch(state) {
	case IDLE_DISCONNECTED_STATE:
		if (connectStateDesired) {
			Serial.println("connecting...");
			Cellular.on();
			Particle.connect();
			state = CONNECT_WAIT_STATE;
		}
		break;

	case CONNECT_WAIT_STATE:
		if (Particle.connected()) {
			Serial.println("connected!");
			state = IDLE_CONNECTED_STATE;
		}
		break;

	case IDLE_CONNECTED_STATE:
		if (!connectStateDesired) {
			Serial.println("disconnecting...");
			Particle.disconnect();
			Cellular.off();
			state = IDLE_DISCONNECTED_STATE;
		}
		break;
	}

}



void displayInfo()
{
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (gps.location.isValid() && gps.location.age() < MAX_GPS_AGE_MS) {
			snprintf(buf, sizeof(buf), "%f,%f", gps.location.lat(), gps.location.lng());
		}
		else {
			strcpy(buf, "no location");
		}
		Serial.println(buf);

		if (Particle.connected()) {
			if (millis() - lastPublish >= PUBLISH_PERIOD) {
				lastPublish = millis();
				Particle.publish("gps", buf, PRIVATE);
			}
		}
	}

}

void buttonHandler(system_event_t event, int data) {
	connectStateDesired = !connectStateDesired;
	Serial.printlnf("connectStateDesired=%d", connectStateDesired);
}



