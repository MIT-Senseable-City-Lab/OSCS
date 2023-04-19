
#include "Particle.h"

#include "UbloxGPS.h" 

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler logHandler;


void displayInfo(); // forward declaration

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

AssetTracker t;             // Support for basic GPS functionality
Ublox ublox;                // Adds in special features of the u-blox GPS models
UbloxAssistNow assistNow;   // Adds in assist now for faster time-to-first-fix

void displayInfo();

unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;

void setup() {
	// This is optional, it just provides time to connect the serial monitor to observe what it's doing
    // waitFor(Serial.isConnected, 10000);

    // You must obtain a u-blox API token and paste it here
	// https://www.u-blox.com/en/assistnow-service-registration-form
    assistNow.withAssistNowKey("PASTE_UBLOX_API_KEY_HERE");
	
	// To run without providing location data (not recommended) uncomment the following line:
	// assistNow.withDisableLocation();

	// You must call setup()
    assistNow.setup();

	// Use I2C mode when running the Feather test. Note that the AssetTracker V1/V2 uses serial, not I2C!
	// t.withI2C();

	// Run in threaded mode - this eliminates the need to read Serial1 from loop or updateGPS() and dramatically
	// lowers the risk of lost or corrupted GPS data caused by blocking loop for too long and overflowing the
	// 64-byte serial buffer.
	t.startThreadedMode();

	// Turn on GPS module - required for AssetTracker V2
	t.gpsOn();

    // Set antenna (false = internal, true = external). Default is internal if not set.
    // ublox.setAntenna(true);

	Particle.connect();
}

void loop() {
	// You must call these from your application loop!
    ublox.loop();
    assistNow.loop();

    displayInfo();
}

void displayInfo() {
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (t.gpsFix()) {
			snprintf(buf, sizeof(buf), "location:%f,%f altitude:%f satellites:%d hdop:%d", t.readLatDeg(), t.readLonDeg(), t.getAltitude(), t.getSatellites(), t.getTinyGPSPlus()->getHDOP().value());
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Log.info("%lu milliseconds to get GPS fix", elapsed);
			}
		}
		else {
			snprintf(buf, sizeof(buf), "no location satellites:%d", t.getSatellites());
			if (!gettingFix) {
				gettingFix = true;
				startFix = millis();
			}
		}
		Log.info(buf);

		if (Particle.connected()) {
			if (millis() - lastPublish >= PUBLISH_PERIOD) {
				lastPublish = millis();
				Particle.publish("gps", buf, PRIVATE);
			}
		}
	}

}
