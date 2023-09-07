
#include "AssetTrackerRK.h"

SYSTEM_THREAD(ENABLED);

SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler logHandler;

void checkGnss(); // forward declaration
void buttonHandler(system_event_t event, int param);

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long CHECK_PERIOD = 100;

AssetTrackerFeather6 t;
unsigned long lastCheck = 0;
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;
bool doSleep = false;

void setup()
{
	// When the MODE button is pressed, the device will go to sleep for 
	// 60 seconds.
	System.on(button_final_click, buttonHandler);

	// Optional: Enable this to wait for USB serial to connect so you can
	// see all of the log messages at startup.
	waitFor(Serial.isConnected, 10000);

	// Be sure to call Tracker setup from setup!
	t.setup();

    startFix = millis();
    gettingFix = true;

	Particle.connect();
}

void loop()
{
	// Be sure to call Tracker loop from loop. 
    t.loop();

	checkGnss();

	if (doSleep) {
		doSleep = false;

		Log.info("going to sleep");

		// Put the GPS to sleep
		t.gnssSleep();

		// Put the system to sleep
		SystemSleepConfiguration config;
		config.mode(SystemSleepMode::ULTRA_LOW_POWER)
			.duration(60s);

		System.sleep(config);

		waitFor(Serial.isConnected, 10000);

		Log.info("woke from sleep");

		// Restart the connecting to GNSS timer
		lastSerial = 0;

		// Wake the GPS again
		t.gnssWake();

	}
}

void checkGnss()
{
	if (millis() - lastCheck < CHECK_PERIOD) {
		return;
	}
	lastCheck = millis();

	// Check the GNSS 10 times per second (100 milliseconds)

	char buf[128];
	bool fixStatusChange = false;

	if (t.gpsFix()) {
		snprintf(buf, sizeof(buf), "location:%f,%f altitude:%f satellites:%d hdop:%d", t.readLatDeg(), t.readLonDeg(), t.getAltitude(), t.getSatellites(), t.getTinyGPSPlus()->getHDOP().value());
		if (gettingFix) {
			gettingFix = false;
			fixStatusChange = true;
			unsigned long elapsed = millis() - startFix;
			Log.info("%lu milliseconds to get GPS fix", elapsed);
		}
	}
	else {
		snprintf(buf, sizeof(buf), "no location satellites:%d", t.getSatellites());
		if (!gettingFix) {
			gettingFix = true;
			fixStatusChange = true;
			startFix = millis();
		}
	}

	if (millis() - lastSerial >= SERIAL_PERIOD || fixStatusChange) {
		lastSerial = millis();
		Log.info(buf);
	}

	// Publish to the cloud
	if (Particle.connected()) {
		if (millis() - lastPublish >= PUBLISH_PERIOD) {
			lastPublish = millis();
			Particle.publish("gps", buf, PRIVATE);
		}
	}

}


void buttonHandler(system_event_t event, int param) {
	// int clicks = system_button_clicks(param);

	doSleep = true;
}