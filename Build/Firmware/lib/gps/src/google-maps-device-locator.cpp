#include "Particle.h"
#include "google-maps-device-locator.h"

#if Wiring_Cellular
# include "CellularHelper.h"
#endif

static char requestBuf[256];
static char *requestCur;
static int numAdded = 0;

GoogleMapsDeviceLocator::GoogleMapsDeviceLocator() : locatorMode(LOCATOR_MODE_MANUAL), periodMs(10000), eventName("deviceLocator"),
	stateTime(0), state(CONNECT_WAIT_STATE), callback(NULL), waitAfterConnect(8000) {

}

GoogleMapsDeviceLocator::~GoogleMapsDeviceLocator() {

}

GoogleMapsDeviceLocator &GoogleMapsDeviceLocator::withLocateOnce() {
	locatorMode = LOCATOR_MODE_ONCE;
	return *this;
}

GoogleMapsDeviceLocator &GoogleMapsDeviceLocator::withLocatePeriodic(unsigned long secondsPeriodic) {
	locatorMode = LOCATOR_MODE_PERIODIC;
	if (secondsPeriodic < 5) {
		secondsPeriodic = 5;
	}
	periodMs = secondsPeriodic * 1000;
	return *this;
}

GoogleMapsDeviceLocator &GoogleMapsDeviceLocator::withEventName(const char *name) {
	this->eventName = name;
	return *this;
}

GoogleMapsDeviceLocator &GoogleMapsDeviceLocator::withSubscribe(GoogleMapsDeviceLocatorSubscriptionCallback callback) {
	this->callback = callback;

	snprintf(requestBuf, sizeof(requestBuf), "hook-response/%s/%s", eventName.c_str(), System.deviceID().c_str());

	Particle.subscribe(requestBuf, &GoogleMapsDeviceLocator::subscriptionHandler, this, MY_DEVICES);

	return *this;
}

GoogleMapsDeviceLocator &GoogleMapsDeviceLocator::withOperator(const char *oper, int mcc, int mnc) {
	this->oper = oper;
	this->mcc = mcc;
	this->mnc = mnc;
	return *this;
}



void GoogleMapsDeviceLocator::loop() {
	switch(state) {
	case CONNECT_WAIT_STATE:
		if (Particle.connected()) {
			state = CONNECTED_WAIT_STATE;
			stateTime = millis();
		}
		break;

	case CONNECTED_WAIT_STATE:
		if (millis() - stateTime >= waitAfterConnect) {
			// Wait several seconds after connecting before doing the location
			if (locatorMode == LOCATOR_MODE_ONCE) {
				publishLocation();

				state = IDLE_STATE;
			}
			else
			if (locatorMode == LOCATOR_MODE_MANUAL) {
				state = IDLE_STATE;
			}
			else {
				state = CONNECTED_STATE;
				stateTime = millis() - periodMs;
			}
		}
		break;

	case CONNECTED_STATE:
		if (Particle.connected()) {
			if (millis() - stateTime >= periodMs) {
				stateTime = millis();
				publishLocation();
			}
		}
		else {
			// We have disconnected, rec
			state = CONNECT_WAIT_STATE;
		}
		break;


	case IDLE_STATE:
		// Just hang out here forever (entered only on LOCATOR_MODE_ONCE)
		break;
	}

}

const char *GoogleMapsDeviceLocator::scan() {
#if Wiring_WiFi
	return wifiScan();
#endif
#if Wiring_Cellular
	return cellularScan();
#endif
}


void GoogleMapsDeviceLocator::publishLocation() {

	Serial.println("publishLocation");

	const char *scanData = scan();

	Serial.printlnf("scanData=%s", scanData);

	if (scanData[0]) {

		if (Particle.connected()) {
			Particle.publish(eventName, scanData, PRIVATE);
		}
	}
}

void GoogleMapsDeviceLocator::subscriptionHandler(const char *event, const char *data) {
	// event: hook-response/deviceLocator/<deviceid>/0

	if (callback) {
		// float lat, float lon, float accuracy
		char *mutableCopy = strdup(data);
		char *part, *end;
		float lat, lon, accuracy;

		part = strtok_r(mutableCopy, ",", &end);
		if (part) {
			lat = atof(part);
			part = strtok_r(NULL, ",", &end);
			if (part) {
				lon = atof(part);
				part = strtok_r(NULL, ",", &end);
				if (part) {
					accuracy = atof(part);

					(*callback)(lat, lon, accuracy);
				}
			}
		}

		free(mutableCopy);
	}
}



#if Wiring_WiFi

static void wifiScanCallback(WiFiAccessPoint* wap, void* data) {
	// The - 3 factor here to leave room for the closing JSON array ] object }} and the trailing null
	size_t spaceLeft = &requestBuf[sizeof(requestBuf) - 3] - requestCur;

	size_t sizeNeeded = snprintf(requestCur, spaceLeft,
			"%s{\"m\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"s\":%d,\"c\":%d}",
			(requestCur[-1] == '[' ? "" : ","),
			wap->bssid[0], wap->bssid[1], wap->bssid[2], wap->bssid[3], wap->bssid[4], wap->bssid[5],
			wap->rssi, wap->channel);
	if (sizeNeeded <= spaceLeft) {
		// There is enough space to store the whole entry, so save it
		requestCur += sizeNeeded;
		numAdded++;
	}
}


const char *GoogleMapsDeviceLocator::wifiScan() {

	requestCur = requestBuf;
	numAdded = 0;

	requestCur += sprintf(requestCur, "{\"w\":{\"a\":");
	*requestCur++ = '[';

	WiFi.scan(wifiScanCallback);

	*requestCur++ = ']';
	*requestCur++ = '}';
	*requestCur++ = '}';
	*requestCur++ = 0;

	if (numAdded == 0) {
		requestBuf[0] = 0;
	}

	return requestBuf;
}

#endif /* Wiring_WiFi */


#if Wiring_Cellular

static void cellularAddTower(const CellularHelperEnvironmentCellData *cellData) {
	// The - 4 factor here to leave room for the closing JSON array ], object }}, and the trailing null
	size_t spaceLeft = &requestBuf[sizeof(requestBuf) - 4] - requestCur;

	size_t sizeNeeded = snprintf(requestCur, spaceLeft,
			"%s{\"i\":%d,\"l\":%u,\"c\":%d,\"n\":%d}",
			(requestCur[-1] == '[' ? "" : ","),
			cellData->ci, cellData->lac, cellData->mcc, cellData->mnc);

	if (sizeNeeded <= spaceLeft && cellData->lac != 0 && cellData->lac != 65535 && cellData->mcc != 65535 && cellData->mnc != 65535) {
		// There is enough space to store the whole entry, so save it
		requestCur += sizeNeeded;
		numAdded++;
	}

}

#if HAS_CELLULAR_GLOBAL_IDENTITY
const char *GoogleMapsDeviceLocator::cellularScanCGI() {

	*requestCur = 0;

	// getOperatorName (AT+UDOPN) is not supported on LTE (SARA-R410M-02-B) but the function
	// will return an empty string which is fine.
	String oper = CellularHelper.getOperatorName();

	CellularGlobalIdentity cgi = {0};
	cgi.size = sizeof(CellularGlobalIdentity);
	cgi.version = CGI_VERSION_LATEST;

	cellular_result_t res = cellular_global_identity(&cgi, NULL);
	if (res == SYSTEM_ERROR_NONE) {
		// We know these things fit, so just using sprintf instead of snprintf here
		requestCur += sprintf(requestCur, "{\"c\":{\"o\":\"%s\",", oper.c_str());

		requestCur += sprintf(requestCur, "\"a\":[");

		requestCur += sprintf(requestCur,
					"{\"i\":%d,\"l\":%u,\"c\":%d,\"n\":%d}",
					cgi.cell_id, cgi.location_area_code, cgi.mobile_country_code, cgi.mobile_network_code);

		numAdded++;

		*requestCur++ = ']';
		*requestCur++ = '}';
		*requestCur++ = '}';
		*requestCur++ = 0;
	}
	else {
		// Serial.printlnf("cellular_global_identity failed %d", res);
	}

	return requestBuf;
}
#endif /* HAS_CELLULAR_GLOBAL_IDENTITY */

// This is only useful on the Electron and E Series LTE before Device OS 1.2.1.
// It does not work on the Boron LTE. The cellular global identity (CGI) version
// is better, and this will eventually be deprecated.
const char *GoogleMapsDeviceLocator::cellularScanLTE() {

	CellularHelperCREGResponse resp;
	CellularHelper.getCREG(resp);

	// Serial.println(resp.toString().c_str());

	// We know these things fit, so just using sprintf instead of snprintf here
	requestCur += sprintf(requestCur, "{\"c\":{\"o\":\"%s\",", oper.c_str());

	requestCur += sprintf(requestCur, "\"a\":[");

	if (resp.valid) {
		requestCur += sprintf(requestCur,
					"{\"i\":%d,\"l\":%u,\"c\":%d,\"n\":%d}",
					resp.ci, resp.lac, mcc, mnc);

		numAdded++;
	}

	*requestCur++ = ']';
	*requestCur++ = '}';
	*requestCur++ = '}';
	*requestCur++ = 0;



	if (numAdded == 0) {
		requestBuf[0] = 0;
	}

	return requestBuf;
}


const char *GoogleMapsDeviceLocator::cellularScan() {

	requestCur = requestBuf;
	numAdded = 0;

#if HAS_CELLULAR_GLOBAL_IDENTITY
	{
		static bool modelChecked = false;
		static bool useCGI = false;

		if (!modelChecked) {
			modelChecked = true;

			// Use Cellular Global Identity (CGI) on Device OS 1.2.1 and later
			// if the modem is not a global 2G (G350). On the G350, AT+CGEG=5
			// works so a better multi-tower result can be returned.
			useCGI = !CellularHelper.getModel().startsWith("SARA-G350");
		}
		if (useCGI) {
			return cellularScanCGI();
		}
	}
#endif

	if (CellularHelper.isLTE()) {
		return cellularScanLTE();
	}

	// First try to get info on neighboring cells. This doesn't work for me using the U260
	CellularHelperEnvironmentResponseStatic<4> envResp;

	CellularHelper.getEnvironment(5, envResp);

	if (envResp.resp != RESP_OK) {
		// We couldn't get neighboring cells, so try just the receiving cell
		CellularHelper.getEnvironment(3, envResp);
	}
	// envResp.serialDebug();


	// We know these things fit, so just using sprintf instead of snprintf here
	requestCur += sprintf(requestCur, "{\"c\":{\"o\":\"%s\",",
			CellularHelper.getOperatorName().c_str());

	requestCur += sprintf(requestCur, "\"a\":[");

	cellularAddTower(&envResp.service);

	for(size_t ii = 0; ii < envResp.getNumNeighbors(); ii++) {
		cellularAddTower(&envResp.neighbors[ii]);
	}

	*requestCur++ = ']';
	*requestCur++ = '}';
	*requestCur++ = '}';
	*requestCur++ = 0;

	if (numAdded == 0) {
		requestBuf[0] = 0;
	}

	return requestBuf;
}


#endif /* Wiring_Cellular */





