
#include "Particle.h"

#include "LIS3DH.h"
#include "TinyGPS++.h"
#include "AssetTrackerRK.h" // Only for AssetTracker::antennaExternal

// Example of Wake On Move with the AssetTracker and the Electron
//
// Official project location:
// https://github.com/rickkas7/LIS3DH


// System threading is required for this project
SYSTEM_THREAD(ENABLED);

// Global objects
FuelGauge batteryMonitor;
LIS3DHSPI accel(SPI, A2, WKP);

TinyGPSPlus gps;


void displayInfo();

// This is the name of the Particle event to publish for battery or movement detection events
// It is a private event.
const char *eventName = "accel";

// Various timing constants
const unsigned long MAX_TIME_TO_PUBLISH_MS = 60000; // Only stay awake for 60 seconds trying to connect to the cloud and publish
const unsigned long MAX_TIME_FOR_GPS_FIX_MS = 180000; // Only stay awake for 3 minutes trying to get a GPS fix
const unsigned long TIME_AFTER_PUBLISH_MS = 4000; // After publish, wait 4 seconds for data to go out
const unsigned long TIME_AFTER_BOOT_MS = 5000; // At boot, wait 5 seconds before going to sleep again (after coming online)
const unsigned long TIME_PUBLISH_BATTERY_SEC = 22 * 60; // every 22 minutes send a battery update to keep the cellular connection up
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

const uint8_t movementThreshold = 16;

// Stuff for the finite state machine
enum State { ONLINE_WAIT_STATE, RESET_STATE, RESET_WAIT_STATE, PUBLISH_STATE, SLEEP_STATE, SLEEP_WAIT_STATE, CALIBRATE_STATE, BOOT_WAIT_STATE, GPS_WAIT_STATE };
State state = ONLINE_WAIT_STATE;
unsigned long stateTime = 0;
int awake = 0;
unsigned long lastSerial = 0;
unsigned long startFix = 0;
bool gettingFix = false;


void setup() {
	Serial.begin(9600);

	// The GPS module on the AssetTracker is connected to Serial1 and D6
	Serial1.begin(9600);

	// Settings D6 LOW powers up the GPS module
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
    startFix = millis();
    gettingFix = true;

    // To use an external antenna, uncomment this line:
    // { AssetTracker t; t.antennaExternal(); }
}


void loop() {
	while (Serial1.available() > 0) {
		if (gps.encode(Serial1.read())) {
			displayInfo();
		}
	}

	switch(state) {
	case ONLINE_WAIT_STATE:
		if (Particle.connected()) {
			state = RESET_STATE;
		}
		if (millis() - stateTime > 5000) {
			stateTime = millis();
			Serial.println("waiting to come online");
		}
		break;

	case RESET_STATE: {
		Serial.println("resetting accelerometer");

		LIS3DHConfig config;
		config.setLowPowerWakeMode(16);

		if (!accel.setup(config)) {
			Serial.println("accelerometer not found");
			state = SLEEP_STATE;
			break;
		}

		state = BOOT_WAIT_STATE;
		}
		break;

	case GPS_WAIT_STATE:
		if (gps.location.isValid() && gps.location.age() < MAX_GPS_AGE_MS) {
			// Got a GPS fix
			state = PUBLISH_STATE;
			break;
		}
		if (millis() - stateTime >= MAX_TIME_FOR_GPS_FIX_MS) {
			Serial.println("failed to get GPS fix");
			state = SLEEP_STATE;
			break;

		}
		break;

	case PUBLISH_STATE:
		if (Particle.connected()) {
			// The publish data contains 3 comma-separated values:
			// whether movement was detected (1) or not (0) The not detected publish is used for battery status updates
			// cell voltage (decimal)
			// state of charge (decimal)
			char data[64];
			float cellVoltage = batteryMonitor.getVCell();
			float stateOfCharge = batteryMonitor.getSoC();
			snprintf(data, sizeof(data), "%d,%.02f,%.02f,%f,%f",
					awake, cellVoltage, stateOfCharge, gps.location.lat(), gps.location.lng());

			Particle.publish(eventName, data, 60, PRIVATE);
			Serial.println(data);

			// Wait for the publish to go out
			stateTime = millis();
			state = SLEEP_WAIT_STATE;
		}
		else {
			// Haven't come online yet
			if (millis() - stateTime >= MAX_TIME_TO_PUBLISH_MS) {
				// Took too long to publish, just go to sleep
				state = SLEEP_STATE;
			}
		}
		break;

	case SLEEP_WAIT_STATE:
		if (millis() - stateTime >= TIME_AFTER_PUBLISH_MS) {
			state = SLEEP_STATE;
		}
		break;

	case BOOT_WAIT_STATE:
		if (millis() - stateTime >= TIME_AFTER_BOOT_MS) {
			// To publish the battery stats after boot, set state to PUBLISH_STATE
			// To go to sleep immediately, set state to SLEEP_STATE
			state = GPS_WAIT_STATE;
			stateTime = millis();
		}
		break;

	case SLEEP_STATE:
		// It's a good idea to reset the accelerometer here. It shouldn't be necessary, but 
		// sometimes if you don't do this, the Electron will never wake up again. This is
		// oddly correlated with powering down the GPS
		{
			LIS3DHConfig config;
			config.setLowPowerWakeMode(16);
	
			accel.setup(config);
		}	
		state = CALIBRATE_STATE;
		stateTime = millis();
		break;

	case CALIBRATE_STATE:
		// Wait for Electron to stop moving for 2 seconds so we can recalibrate the accelerometer
		// Wait for 5 seconds before looping again
		if (accel.calibrateFilter(2000, 5000)) {
			// We've stopped moving and the accelerometer is calibrated

			// Uncomment this line to power down the GPS. It saves power but will increase the amount
			// of time to get a fix.
			// digitalWrite(D6, HIGH);

			Serial.println("going to sleep");
			delay(500);

			// If you use SLEEP_MODE_DEEP it's very important to make sure WKP is LOW before going to
			// sleep. If you go into SLEEP_MODE_DEEP with WKP high you will likely never wake up again
			// (until reset).
			if (digitalRead(WKP)) {
				// Try to calibrate again
				break;
			}

			// Sleep
			System.sleep(WKP, RISING, TIME_PUBLISH_BATTERY_SEC, SLEEP_NETWORK_STANDBY);

			// This delay should not be necessary, but sometimes things don't seem to work right
			// immediately coming out of sleep.
			delay(500);

			awake = ((accel.clearInterrupt() & LIS3DH::INT1_SRC_IA) != 0);

			Serial.printlnf("awake=%d", awake);

			// Restart the GPS
			digitalWrite(D6, LOW);
			startFix = millis();
			gettingFix = true;

			state = GPS_WAIT_STATE;
			stateTime = millis();
		}
		else {
			Serial.printlnf("still moving after %u sec", (millis() - stateTime) / 1000);
		}
		break;

	}

}



void displayInfo()
{
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (gps.location.isValid()) {
			snprintf(buf, sizeof(buf), "%f,%f", gps.location.lat(), gps.location.lng());
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
		}
		Serial.println(buf);
	}
}
