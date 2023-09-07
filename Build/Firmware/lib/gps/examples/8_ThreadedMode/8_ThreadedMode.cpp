
#include "AssetTrackerRK.h"

SYSTEM_THREAD(ENABLED);

void displayInfo(); // forward declaration

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;

AssetTracker t;
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;

void setup()
{
	Serial.begin();

	// Turn on GPS module
	t.gpsOn();

	// Run in threaded mode - this eliminates the need to read Serial1 from loop or updateGPS() and dramatically
	// lowers the risk of lost or corrupted GPS data caused by blocking loop for too long and overflowing the
	// 64-byte serial buffer.
	t.startThreadedMode();

    startFix = millis();
    gettingFix = true;

    // If using an external antenna, uncomment this line:
    // t.antennaExternal();
}

void loop()
{
	// In threaded mode, you must not call updateGPS() from loop - it's handled automatically
	// from a separate thread if you call t.startThreadedMode()

	displayInfo();
}

void displayInfo()
{
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (t.gpsFix()) {
			snprintf(buf, sizeof(buf), "location:%f,%f altitude:%f satellites:%d hdop:%d", t.readLatDeg(), t.readLonDeg(), t.getAltitude(), t.getSatellites(), t.getTinyGPSPlus()->getHDOP().value());
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}
		}
		else {
			snprintf(buf, sizeof(buf), "no location satellites:%d", t.getSatellites());
			if (!gettingFix) {
				gettingFix = true;
				startFix = millis();
			}
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
