#ifndef __LEGACYADAPTER_H
#define __LEGACYADAPTER_H

#include "TinyGPS++.h"

/**
 * @brief This class adapts the output from TinyGPS++ to match the results from the Adafruit parser and official Particle AssetTracker library
 */
class LegacyAdapter {
public:
	/**
	 * @brief Constructs the LegacyAdapter object.
	 *
	 * @param gpsData The TinyGPSPlus object containing the actual data
	 *
	 * You will likely never need to do this as AssetTracker is a superclass of this object. This class
	 * just separates out the legacy translation features so they can be kept together and also to
	 * be tested by the unit test framework.
	 */
	LegacyAdapter(TinyGPSPlus &gpsData);

	/**
	 * @brief Destructor
	 */
	virtual ~LegacyAdapter();

	/**
	 * @brief Converts a degree value (as a double) into the weird GPS DDMM.MMMMM format (degrees * 100 + minutes).
	 *
	 * This is the format for the Adafruit methods that don't have "Deg" in them, like readLat() and readLon().
	 * Because of rounding it may not be the same as the actual value returned by the GPS, but it should be close.
	 */
	float convertToDegreesMinutes(double deg) const;

	/**
	 * @brief Return the latitude in a GPS-style value DDMM.MMMMM format (degrees * 100 + minutes).
	 *
	 * The result is always positive! To find out if the latitude is south, check location.rawLat().negative
	 * or check the sign of readLatDeg().
	 */
	float readLat(void) const {
		return convertToDegreesMinutes(gpsData.getLocation().lat());
	}

	/**
	 * @brief Return the longitude in a GPS-style value DDMM.MMMMM format (degrees * 100 + minutes).
	 *
	 * The result is always positive! To find out if the latitude is east, check location.rawLng().negative
	 * or check the sign of readLonDeg().
	 */
	float readLon(void) const {
		return convertToDegreesMinutes(gpsData.getLocation().lng());
	}

	/**
	 * @brief Returns the latitude in degrees as a float. May be positive or negative.
	 *
	 * Negative values are used for south latitude.
	 */
	float readLatDeg(void) const {
		return (float) gpsData.getLocation().lat();
	}

	/**
	 * @brief Returns the longitude in degrees as a float. May be positive or negative.
 	 *
	 * Negative values are used for east longitude.
	 */
	float readLonDeg(void) const {
		return (float) gpsData.getLocation().lng();
	}

	/**
	 * @brief Gets the speed in knots
	 *
	 * If you want to get more common units, you can use:
	 *
	 * Miles per hour: getTinyGPSPlus()->getSpeed().mph();
	 * Meters per second: getTinyGPSPlus()->getSpeed().mps();
	 * Kilometers per hour: getTinyGPSPlus()->getSpeed().kmph();
	 *
	 */
	float getSpeed() const {
		// The Adafruit library does not check the validity and always returns the last speed
		return (float) gpsData.getSpeed().knots();
	}

	/**
	 * @brief Get the course angle in degrees 0 <= deg < 360
	 */
	float getAngle() const {
		return (float) gpsData.getCourse().deg();
	}

	/**
	 * @brief Get the current hour (in UTC)
	 */
	uint8_t getHour() const {
		return (uint8_t) gpsData.getTime().hour();
	}

	/**
	 * @brief Get the current minute (in UTC)
	 */
	uint8_t getMinute() const {
		return (uint8_t) gpsData.getTime().minute();
	}

	/**
	 * @brief Get the current second (in UTC)
	 */
	uint8_t getSeconds() const {
		return (uint8_t) gpsData.getTime().second();
	}

	/**
	 * @brief Get the current millisecond (in UTC)
	 *
	 * Note that this is the time of the last timestamp received by GPS, and does not automatically increment,
	 * so it's not useful for measuring the actual time to the nearest millisecond!
	 */
	uint16_t getMilliseconds() const {
		return (uint16_t)gpsData.getTime().centisecond() * 10;
	}

	/**
	 * @brief Get the current year (in UTC)
	 *
	 * Because the AssetTracker V1 only returned a 2-digit year, that is returned here as well.
	 *
	 * The 4-digit year can be found with: getTinyGPSPlus()->getDate().year()
	 */
	uint8_t getYear() const {
		return (uint8_t) (gpsData.getDate().year() % 100);
	}

	/**
	 * @brief Get the current month (1-12) (at UTC)
	 */
	uint8_t getMonth() const {
		return (uint8_t) gpsData.getDate().month();
	}

	/**
	 * @brief Get the current day of month (1-31) (at UTC)
	 */
	uint8_t getDay() const {
		return (uint8_t) gpsData.getDate().day();
	}

	/**
	 * @brief Returns the number of milliseconds since the last GPS reading
	 */
	uint32_t getGpsTimestamp() const {
		TinyGPSTime time = gpsData.getTime();

		// Return timestamp in milliseconds, from last GPS reading
		// 0 if no reading has been done
		// (This returns the milliseconds of current day)
		return time.hour() * 60 * 60 * 1000 + time.minute() * 60 * 1000 + time.second() * 1000 + time.centisecond() * 10;
	}

	/**
	 * @brief Returns true (1) if there is a GPS fix or false (0) if not
	 */
    uint8_t getFixQuality() const {
    	return gpsData.getLocation().isValid();
    }

    /**
     * @brief Gets the HDOP value
     *
     * HDOP: Acronym for horizontal dilution of precision. A measure of the geometric quality of a GPS satellite
     * configuration in the sky. HDOP is a factor in determining the relative accuracy of a horizontal position.
     * The smaller the DOP number, the better the geometry.
     */
	float readHDOP(void) const {
		return (float) gpsData.getHDOP().value() / 100.0;
	}

    /**
     * @brief Gets the GPS accuracy
     */
	float getGpsAccuracy() const {
		// 1.8 taken from specs at https://learn.adafruit.com/adafruit-ultimate-gps/
		return 1.8 * readHDOP();
	}

	/**
	 * @brief Get the altitude in meters
	 */
	float getAltitude() const {
		return gpsData.getAltitude().meters();
	}

	/**
	 * @brief Get the GeoID separation in meters
	 *
	 * Geoid separation is difference between ellipsoid and mean sea level.
	 */
	float getGeoIdHeight() const {
		return gpsData.getGeoidSeparation().meters();
	}

	/**
	 * @brief Gets the number of satellites found
	 */
	uint8_t getSatellites() const {
		return (uint8_t) gpsData.getSatellites().value();
	}

	/**
	 * @brief Returns true if there is a GPS fix
	 *
	 * Note: It may take 10 seconds for for this to go to false after losing GPS signal.
	 */
	bool gpsFix(void) const {
		TinyGPSLocation location = gpsData.getLocation();

		return location.isValid() && location.age() < MAX_GPS_AGE_MS;
	}

	/**
	 * @brief Returns a string version of latitude and longitude
	 *
	 * The values are in signed degrees in lat,lon format.
	 */
	String readLatLon(void) const {
		TinyGPSLocation location = gpsData.getLocation();

	    String latLon = String::format("%lf,%lf", location.lat(), location.lng());
	    return latLon;
	}

	/**
	 * @brief GPS location must be newer than this to be considered valid
	 */
	static const unsigned long MAX_GPS_AGE_MS = 10000;

private:
	TinyGPSPlus &gpsData;
};

#endif /* __LEGACYADAPTER_H */
