
#include "Particle.h"

// Port of TinyGPS for the Particle AssetTracker
// https://github.com/mikalhart/TinyGPSPlus

#include "TinyGPS++.h"

SYSTEM_THREAD(ENABLED);

/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object directly.
 */

void displayInfo(); // forward declaration

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const size_t SENTENCE_BUF_SIZE = 256;

// The TinyGPS++ object
TinyGPSPlus gps;
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;
char sentenceBuf[SENTENCE_BUF_SIZE];
size_t sentenceBufOffset = 0;

void setup()
{
	Serial.begin(9600);

	// The GPS module on the AssetTracker is connected to Serial1 and D6
	Serial1.begin(9600);

	// Settings D6 LOW powers up the GPS module
	pinMode(D6, OUTPUT);
	digitalWrite(D6, LOW);
	startFix = millis();
	gettingFix = true;
}

void loop()
{
	while (Serial1.available() > 0) {
		int c = Serial1.read();

		if (sentenceBufOffset < (SENTENCE_BUF_SIZE - 1)) {
			sentenceBuf[sentenceBufOffset++] = (char)c;
		}

		if (gps.encode(c)) {
			displayInfo();

			sentenceBuf[sentenceBufOffset] = 0;
			Serial.println(sentenceBuf);
			sentenceBufOffset = 0;
		}
	}

}

void displayInfo()
{
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (gps.location.isValid()) {
			snprintf(buf, sizeof(buf), "%f,%f,%f", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}
		}
		else {
			strcpy(buf, "no location");
			if (!gettingFix) {
				gettingFix = true;
				startFix = millis();
			}
			Serial.printlnf("charsProcessed=%d sentencesWithFix=%d failedChecksum=%d passedChecksum=%d",
					gps.charsProcessed(), gps.sentencesWithFix(), gps.failedChecksum(), gps.passedChecksum());

		}
		Serial.println(buf);

		if (Particle.connected()) {
			if (millis() - lastPublish >= PUBLISH_PERIOD) {
				lastPublish = millis();
				Particle.publish("gps", buf);
			}
		}
	}

}
