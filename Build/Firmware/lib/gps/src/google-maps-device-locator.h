#ifndef __GOOGLEMAPSDEVICELOCATOR_H
#define __GOOGLEMAPSDEVICELOCATOR_H


#include "Particle.h"

#if defined(SYSTEM_VERSION_v121) && defined(Wiring_Cellular)
#define HAS_CELLULAR_GLOBAL_IDENTITY 1
#else
#define HAS_CELLULAR_GLOBAL_IDENTITY 0
#endif

/**
 * This is the callback function prototype for the callback if you use the
 * withSubscribe() method to be notified of your own location.
 *
 * lat is the latitude in degrees
 * lon is the longitude in degrees
 * accuracy is the accuracy radius in meters
 */
typedef void (*GoogleMapsDeviceLocatorSubscriptionCallback)(float lat, float lon, float accuracy);

class GoogleMapsDeviceLocator {
public:
	GoogleMapsDeviceLocator();
	virtual ~GoogleMapsDeviceLocator();

	/**
	 * If you use withLocateOnce() then the location will be updated once after you've
	 * connected to the cloud.
	 */
	GoogleMapsDeviceLocator &withLocateOnce();

	/**
	 * If you use withLocatePeriod() then the location will be updated this often when
	 * connected to the cloud.
	 *
	 * If you use neither withLocateOnce() nor withLocatePeriodic() you can check manually
	 * using publishLocation() instead
	 */
	GoogleMapsDeviceLocator &withLocatePeriodic(unsigned long secondsPeriodic);

	/**
	 * The default event name is "deviceLocator". Use this method to change it. Note that this
	 * name is also configured in your Google location integration and must be changed in both
	 * locations or location detection will not work
	 */
	GoogleMapsDeviceLocator &withEventName(const char *name);

	/**
	 * Use this method to register a callback to be notified when your location has been found.
	 *
	 * The prototype for the function is:
	 *
	 * void locationCallback(float lat, float lon, float accuracy)
	 */
	GoogleMapsDeviceLocator &withSubscribe(GoogleMapsDeviceLocatorSubscriptionCallback callback);

	/**
	 * Deprecated. No longer needed in Device OS 1.2.1 and later.
	 *
	 * Use this method to set the operator, mcc, and mnc when using LTE in Device OS prior to 1.2.1.
	 * This does not work on the Boron LTE.
	 *
	 * The SARA-R410M-02B cannot get this information using AT+UCGED, so you need to pass it in
	 * manually. Fortunately, in the United States with the Particle SIM it's always
	 * "AT&T", 310, 410.
	 */
	GoogleMapsDeviceLocator &withOperator(const char *oper, int mcc, int mnc);

	/**
	 * You should call this from loop() to give the code time to process things in the background.
	 * This is really only needed if you use withLocateOnce() or withLocatePeriodic() but it doesn't
	 * hurt to call it always from loop. It executes quickly.
	 */
	void loop();

	/**
	 * You can use this to manually publish your location. It finds the Wi-Fi or cellular location
	 * using scan() and then publishes it as an event
	 */
	void publishLocation();

	/**
	 * Queries the location information (Wi-Fi or cellular) and returns a JSON block of data to
	 * put in the event data. This returns a static buffer pointer and is not reentrant.
	 */
	const char *scan();

protected:
	void subscriptionHandler(const char *event, const char *data);

#if Wiring_WiFi
	const char *wifiScan();
#endif

#if HAS_CELLULAR_GLOBAL_IDENTITY
	const char *cellularScanCGI();
#endif

#if Wiring_Cellular
	const char *cellularScanLTE();
	const char *cellularScan();
#endif

	static const int CONNECT_WAIT_STATE = 0;
	static const int CONNECTED_WAIT_STATE = 2;
	static const int CONNECTED_STATE = 3;
	static const int IDLE_STATE = 4;

	static const int LOCATOR_MODE_MANUAL = 0;
	static const int LOCATOR_MODE_ONCE = 1;
	static const int LOCATOR_MODE_PERIODIC = 2;

	int locatorMode;
	unsigned long periodMs;
	String eventName;
	unsigned long stateTime;
	int state;
	GoogleMapsDeviceLocatorSubscriptionCallback callback;
	unsigned long waitAfterConnect;
	String oper = "AT&T"; // Used for LTE (SARA-R410M-02B only)
	int mcc = 310; // LTE
	int mnc = 410; // LTE
};

#endif /* __GOOGLEMAPSDEVICELOCATOR_H */

