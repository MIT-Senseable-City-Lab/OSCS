/*
TinyGPS++ - a small GPS library for Arduino providing universal NMEA parsing
Based on work by and "distanceBetween" and "courseTo" courtesy of Maarten Lamers.
Suggestion to add satellites, courseTo(), and cardinal() by Matt Monson.
Location precision improvements suggested by Wayne Holder.
Copyright (C) 2008-2013 Mikal Hart
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __TinyGPSPlus_h
#define __TinyGPSPlus_h

#include "Particle.h"
#include <limits.h>
#include <math.h>

#define _GPS_VERSION "0.92" // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945 // miles per hour
#define _GPS_MPS_PER_KNOT 0.51444444 // meters per second
#define _GPS_KMPH_PER_KNOT 1.852 // kilometers per hour
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15

// Stuff included in Ardiuno but not Particle:
#ifndef SPARK_WIRING_ARDUINO_CONSTANTS_H
double radians(double deg);
double degrees(double radians);
double sq(double value);
const double TWO_PI = M_PI * 2;
#endif
// End

/**
 * @brief Class to hold raw degree values as integer values instead of floating point
 */
struct RawDegrees
{
	uint16_t deg; 			//!< Degree value (0 <= deg < 365)
	uint32_t billionths; 	//!< Billionths of a segree
	bool negative; 			//!< true if negative (south latitude or east longitude)
public:
	RawDegrees() : deg(0), billionths(0), negative(false)
	{}
};

/**
 * @brief Class to hold a location value
 */
struct TinyGPSLocation
{
	friend class TinyGPSPlus;
public:
	/**
	 * @brief Returns true if the data is valid
	 *
	 * The valid flag will be false if the GPS loses its fix, but will not be invalidated if the GPS
	 * stops returning data. If that is a possibility, you should also check the age.
	 */
	bool isValid() const    { return valid; }

	/**
	 * @brief Returns true if the value has been updated since last read
	 *
	 * The updated flag is set to false after reading data (lag, lng, rawLat, rawLng) and is set to
	 * true when a valid location is received from the GPS.
	 */
	bool isUpdated() const  { return updated; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

	/**
	 * @brief Returns the raw latitude.
	 *
	 * The raw latitude separates out the whole part (in degrees), billionths of a degree, and
	 * sign, instead of using a double floating point number.
	 *
	 * This method is not const because it clears the updated flag.
	 */
	const RawDegrees &rawLat()     { updated = false; return rawLatData; }

	/**
	 * @brief Returns the raw longitude.
	 *
	 * The raw latitude separates out the whole part (in degrees), billionths of a degree, and
	 * sign, instead of using a double floating point number.
	 *
	 * This method is not const because it clears the updated flag.
	 */
	const RawDegrees &rawLng()     { updated = false; return rawLngData; }

	/**
	 * @brief Returns the latitude in degrees as a signed double floating point value.
	 *
	 * Positive values are for north latitude. Negative values are for south latitude.
	 * Valid values are 0 <= lat() < 360.0
	 *
	 * This method is not const because it clears the updated flag.
	 */
	double lat();

	/**
	 * @brief Returns the longitude in degrees as a signed double floating point value.
	 *
	 * Positive values are for west longitude. Negative values are for east longitude.
	 * Valid values are 0 <= lng() < 360.0
	 *
	 * This method is not const because it clears the updated flag.
	 */
	double lng();

	/**
	 * @brief Sets the valid flag to false (marks data as invalid)
	 *
	 * This is used internally when we get a valid GPS sentence that does not have a fix.
	 * It can be checked by using the isValid() method.
	 */
	void invalidate() { valid = false; }

	/**
	 * @brief Constructor
	 */
	TinyGPSLocation() : valid(false), updated(false), lastCommitTime(0)
	{}

private:
	bool valid, updated;
	RawDegrees rawLatData, rawLngData, rawNewLatData, rawNewLngData;
	uint32_t lastCommitTime;
	void commit();
	void setLatitude(const char *term);
	void setLongitude(const char *term);
};

/**
 * @brief Class to hold a date (year, month, day) value.
 */

struct TinyGPSDate
{
	friend class TinyGPSPlus;
public:
	/**
	 * @brief Returns true if the data is valid
	 *
	 * The valid flag will be false if the GPS loses its fix, but will not be invalidated if the GPS
	 * stops returning data. If that is a possibility, you should also check the age.
	 */
	bool isValid() const       { return valid; }

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isUpdated() const     { return updated; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const       { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

	/**
	 * @brief Sets the valid flag to false (marks data as invalid)
	 *
	 * This is used internally when we get a valid GPS sentence that does not have a fix.
	 * It can be checked by using the isValid() method.
	 */
	void invalidate() { valid = false; }

	/**
	 * @brief Returns the current value and clears the updated flag
	 */
	uint32_t value()           { updated = false; return date; }

	/**
	 * @brief Gets the year (4-digit, like 2019)
	 */
	uint16_t year();

	/**
	 * @brief Gets the month 1 <= month <= 12
	 */
	uint8_t month();

	/**
	 * @brief Gets the day of month 1 <= day <= 31
	 */
	uint8_t day();

	/**
	 * @brief Constructor
	 */
	TinyGPSDate() : valid(false), updated(false), date(0), newDate(0), lastCommitTime(0)
	{}

private:
	bool valid;
	bool updated;
	uint32_t date;
	uint32_t newDate;
	uint32_t lastCommitTime;
	void commit();
	void setDate(const char *term);
};

/**
 * @brief Class to hold a time of day value
 */
struct TinyGPSTime
{
	friend class TinyGPSPlus;
public:
	/**
	 * @brief Returns true if the data is valid
	 *
	 * The valid flag will be false if the GPS loses its fix, but will not be invalidated if the GPS
	 * stops returning data. If that is a possibility, you should also check the age.
	 */
	bool isValid() const       { return valid; }

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isUpdated() const     { return updated; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const       { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }


	/**
	 * @brief Returns the current value and clears the updated flag
	 */
	uint32_t value()           { updated = false; return time; }

	/**
	 * @brief Sets the valid flag to false (marks data as invalid)
	 *
	 * This is used internally when we get a valid GPS sentence that does not have a fix.
	 * It can be checked by using the isValid() method.
	 */
	void invalidate() { valid = false; }

	/**
	 * @brief Gets the hour value 0 <= hour < 24
	 */
	uint8_t hour();

	/**
	 * @brief Gets the minute value 0 <= minute < 60
	 */
	uint8_t minute();

	/**
	 * @brief Gets the second value 0 <= second < 60
	 */
	uint8_t second();

	/**
	 * @brief Gets centiseconds 0 <= centisecond < 100
	 */
	uint8_t centisecond();

	/**
	 * @brief Constructor
	 */
	TinyGPSTime() : valid(false), updated(false), time(0), newTime(0), lastCommitTime(0)
	{}

private:
	bool valid, updated;
	uint32_t time, newTime;
	uint32_t lastCommitTime;
	void commit();
	void setTime(const char *term);
};

/**
 * @brief Class to hold an arbitrary decimal value. Typically subclassed.
 */
struct TinyGPSDecimal
{
	friend class TinyGPSPlus;
public:
	/**
	 * @brief Returns true if the data is valid
	 *
	 * The valid flag will be false if the GPS loses its fix, but will not be invalidated if the GPS
	 * stops returning data. If that is a possibility, you should also check the age.
	 */
	bool isValid() const    { return valid; }

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isUpdated() const  { return updated; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

	/**
	 * @brief Returns the current value and clears the updated flag
	 */
	int32_t value()         { updated = false; return val; }

	/**
	 * @brief Sets the valid flag to false (marks data as invalid)
	 *
	 * This is used internally when we get a valid GPS sentence that does not have a fix.
	 * It can be checked by using the isValid() method.
	 */
	void invalidate() { valid = false; }

	/**
	 * @brief Constructor
	 */
	TinyGPSDecimal() : valid(false), updated(false), lastCommitTime(0), val(0), newval(0)
	{}

private:
	bool valid, updated;
	uint32_t lastCommitTime;
	int32_t val, newval;
	void commit();
	void set(const char *term);
};

/**
 * @brief Class to parse and maintain integer data
 */
struct TinyGPSInteger
{
	friend class TinyGPSPlus;
public:
	/**
	 * @brief Returns true if the data is valid
	 *
	 * The valid flag will be false if the GPS loses its fix, but will not be invalidated if the GPS
	 * stops returning data. If that is a possibility, you should also check the age.
	 */
	bool isValid() const    { return valid; }

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isUpdated() const  { return updated; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

	/**
	 * @brief Returns the current value and clears the updated flag
	 */
	uint32_t value()        { updated = false; return val; }

	/**
	 * @brief Sets the valid flag to false (marks data as invalid)
	 *
	 * This is used internally when we get a valid GPS sentence that does not have a fix.
	 * It can be checked by using the isValid() method.
	 */
	void invalidate() { valid = false; }

	/**
	 * @brief Constructor
	 */
	TinyGPSInteger() : valid(false), updated(false), lastCommitTime(0), val(0), newval(0)
	{}

private:
	bool valid, updated;
	uint32_t lastCommitTime;
	uint32_t val, newval;
	void commit();
	void set(const char *term);
};

/**
 * @brief Class to hold speed data and return the value in various units
 */
struct TinyGPSSpeed : TinyGPSDecimal
{
	/**
	 * @brief Returns the speed in knots
	 */
	double knots()    { return value() / 100.0; }

	/**
	 * @brief Returns the speed in miles per hour
	 */
	double mph()      { return _GPS_MPH_PER_KNOT * value() / 100.0; }

	/**
	 * @brief Returns the speed in meters per second
	 */
	double mps()      { return _GPS_MPS_PER_KNOT * value() / 100.0; }

	/**
	 * @brief Returns the speed in kilometers per hour
	 */
	double kmph()     { return _GPS_KMPH_PER_KNOT * value() / 100.0; }
};

/**
 * @brief Class to hold course data and return the direction in degrees 0 <= deg < 360.
 */
struct TinyGPSCourse : public TinyGPSDecimal
{
	/**
	 * @brief Returns the course (direction you are heading) in degrees
	 *
	 * 0 <= deg < 360
	 */
	double deg()      { return value() / 100.0; }
};

/**
 * @brief Class to hold altitude data and return the value in various units
 */
struct TinyGPSAltitude : TinyGPSDecimal
{
	/**
	 * @brief Returns the altitude in meters as a double floating point value.
	 */
	double meters()       { return value() / 100.0; }

	/**
	 * @brief Returns the altitude in miles as a double floating point value.
	 */
	double miles()        { return _GPS_MILES_PER_METER * value() / 100.0; }

	/**
	 * @brief Returns the altitude in kilometers as a double floating point value.
	 */
	double kilometers()   { return _GPS_KM_PER_METER * value() / 100.0; }

	/**
	 * @brief Returns the altitude in feet as a double floating point value.
	 */
	double feet()         { return _GPS_FEET_PER_METER * value() / 100.0; }
};

class TinyGPSPlus; // Forward declaration

/**
 * @brief Class parse and hold an arbitrary value. Typically subclassed.
 */
class TinyGPSCustom
{
public:
	/**
	 * @brief Constructor
	 */
	TinyGPSCustom() : lastCommitTime(0), valid(false), updated(false), sentenceName(0), termNumber(0), next(0) {};

	/**
	 * @brief Constructor
	 */
	TinyGPSCustom(TinyGPSPlus &gps, const char *sentenceName, int termNumber);

	/**
	 * @brief Called when this sentence is beginning
	 */
	void begin(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber);

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isUpdated() const  { return updated; }

	/**
	 * @brief Returns true if the value has been updated.
	 *
	 * Getting the value clears the updated flag, and commiting a change sets it.
	 */
	bool isValid() const    { return valid; }

	/**
	 * @brief Returns the age of the value in milliseconds
	 *
	 * If the value is not valid, then ULONG_MAX is returned.
	 *
	 * You might check to see if age is < 10000 to make sure the value has been retrieved in the
	 * last 10 seconds, for example.
	 */
	uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

	/**
	 * @brief Returns the current value and clears the updated flag
	 */
	const char *value()     { updated = false; return buffer; }

private:
	void commit();
	void set(const char *term);

	char stagingBuffer[_GPS_MAX_FIELD_SIZE + 1];
	char buffer[_GPS_MAX_FIELD_SIZE + 1];
	unsigned long lastCommitTime;
	bool valid, updated;
	const char *sentenceName;
	int termNumber;
	friend class TinyGPSPlus;
	TinyGPSCustom *next;
};

/**
 * @brief Container to hold the data values.
 *
 * The TinyGPSPlus object has this as a public superclass so you can still access the location field,
 * for example. However, it also means you can copy all of the values to a local variable if you
 * want to.
 *
 * Also, it's used internally so a temporary copy can be updated while parsing the GPS data. If the
 * checksum is valid, then the data will be copied into the TinyGPSPlus superclass TinyGPSData. This
 * is necessary to avoid reading half-written values or blocking for long periods of time when
 * supporting multi-threaded mode.
 *
 * For best thread safety, you should avoid using fields like location directly and instead should
 * use methods like getLocation() to make a copy of the location data. This will assure that your
 * data is valid and does not change while you are reading it.
 */
class TinyGPSData {
public:
	/**
	 * @brief Location (latitude and longitude)
	 *
	 * While this field is public, you should instead use getLocation(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSLocation location;

	/**
	 * @brief Get the date (year, month, day of month)
	 *
	 * While this field is public, you should instead use getDate(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSDate date;

	/**
	 * @brief Get the time (hour, minute, second, centisecond)
	 *
	 * While this field is public, you should instead use getTime(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSTime time;

	/**
	 * @brief Get the speed
	 *
	 * While this field is public, you should instead use getSpeed(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSSpeed speed;

	/**
	 * @brief Get the course (direction of travel)
	 *
	 * While this field is public, you should instead use getCourse(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSCourse course;

	/**
	 * @brief
	 *
	 * While this field is public, you should instead use getAltitude(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSAltitude altitude;

	/**
	 * @brief Get the Geo ID separation
	 *
	 * While this field is public, you should instead use getGeoidSeparation(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSAltitude geoidSeparation;

	/**
	 * @brief Get the number of satellites
	 *
	 * While this field is public, you should instead use getSatellites(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSInteger satellites;

	/**
	 * @brief Get the HDOP
	 *
	 * While this field is public, you should instead use getHDOP(). In multi-threaded mode,
	 * accessing this field directly is not safe, but for backward compatibility this field
	 * remains public.
	 */
	TinyGPSDecimal hdop;

	/**
	 * @brief Get the location (latitude and longitude)
	 */
	TinyGPSLocation getLocation() const {
	    SINGLE_THREADED_BLOCK() {
	    	return location;
	    }
		return location;
	}

	/**
	 * @brief Get the date (year, month, day of month)
	 */
	TinyGPSDate getDate() const {
	    SINGLE_THREADED_BLOCK() {
	    	//return date;
	    }
		return date;
	}

	/**
	 * @brief Get the time (hour, minute, second, centisecond)
	 */
	TinyGPSTime getTime() const {
	    SINGLE_THREADED_BLOCK() {
	    	//return time;
	    }
		return time;
	}

	/**
	 * @brief Get the speed
	 */
	TinyGPSSpeed getSpeed() const {
	    SINGLE_THREADED_BLOCK() {
	    	return speed;
	    }
	}

	/**
	 * @brief Get the course
	 */
	TinyGPSCourse getCourse() const {
	    SINGLE_THREADED_BLOCK() {
	    	return course;
	    }
	}

	/**
	 * @brief Get the altitude
	 */
	TinyGPSAltitude getAltitude() const {
	    SINGLE_THREADED_BLOCK() {
	    	return altitude;
	    }
	}

	/**
	 * @brief Get the Geo ID separation
	 *
	 * Geoid separation is difference between ellipsoid and mean sea level (in meters)
	 */
	TinyGPSAltitude getGeoidSeparation() const {
	    SINGLE_THREADED_BLOCK() {
	    	return geoidSeparation;
	    }
	}

	/**
	 * @brief Get the number of satellites
	 */
	TinyGPSInteger getSatellites() const {
	    SINGLE_THREADED_BLOCK() {
	    	return satellites;
	    }
	}

	/**
	 * @brief Get the HDOP
	 *
     * HDOP: Acronym for horizontal dilution of precision. A measure of the geometric quality of a GPS satellite
     * configuration in the sky. HDOP is a factor in determining the relative accuracy of a horizontal position.
     * The smaller the DOP number, the better the geometry.
	 */
	TinyGPSDecimal getHDOP() const {
	    SINGLE_THREADED_BLOCK() {
	    	return hdop;
	    }
	}

	/**
	 * @brief Copy the data in this object to another object, atomically
	 *
	 * This is different than operator= because it uses a SINGLE_THREADED_BLOCK.
	 */
	void copyDataTo(TinyGPSData &other) const{
	    SINGLE_THREADED_BLOCK() {
	    	other.operator=(*this);
	    }
	}

};

/**
 * @public Class to parse GPS data and maintain the current state
 *
 * You typically instantiate one of these per application as a global variable.
 * There is one embedded in the AssetTracker class
 */
class TinyGPSPlus : public TinyGPSData
{
public:
	/**
	 * @brief Constructor
	 */
	TinyGPSPlus();

	/**
	 * @brief Encode one character of data from the GPS
	 *
	 * Typically you read from serial and call the encode() method. Note that you should empty
	 * the serial buffer, not just parse one character per loop!
	 *
	 * Returns true if a full sentence was just parsed
	 */
	bool encode(char c);

	/**
	 * @brief operator<< can be used instead of encode
	 */
	TinyGPSPlus &operator << (char c) {encode(c); return *this;}

	/**
	 * @brief Returns the version of the TinyGPS++ library
	 */
	static const char *libraryVersion() { return _GPS_VERSION; }

	/**
	 * @brief Utility to calculate the distance between two locations
	 */
	static double distanceBetween(double lat1, double long1, double lat2, double long2);

	/**
	 * @brief Utility to calculate the course angle between two locations
	 */
	static double courseTo(double lat1, double long1, double lat2, double long2);

	/**
	 * @brief Given a course in degrees, returns a cardinal (N, NNE, NE, etc.)
	 */
	static const char *cardinal(double course);

	/**
	 * @brief Used internally to parse a GPS decimal value
	 */
	static int32_t parseDecimal(const char *term);

	/**
	 * @brief Used internally to parse a GPS degress value
	 */
	static void parseDegrees(const char *term, RawDegrees &deg);

	/**
	 * @brief Return the number of characters processed
	 */
	uint32_t charsProcessed()   const { return encodedCharCount; }

	/**
	 * @brief Return the number of GPS sentences parsed with a GPS fix
	 */
	uint32_t sentencesWithFix() const { return sentencesWithFixCount; }

	/**
	 * @brief Return the number of sentences that had a failed checksum
	 */
	uint32_t failedChecksum()   const { return failedChecksumCount; }

	/**
	 * @brief Return the number of sentences with a valid checksum
	 */
	uint32_t passedChecksum()   const { return passedChecksumCount; }

private:
	enum {GPS_SENTENCE_GPGGA, GPS_SENTENCE_GPRMC, GPS_SENTENCE_OTHER};

	// temporary data until we get a valid checksum. This is also used to avoid
	// corrupted data in multithreaded mode
	TinyGPSData tempData;

	// parsing state variables
	uint8_t parity;
	bool isChecksumTerm;
	char term[_GPS_MAX_FIELD_SIZE];
	uint8_t curSentenceType;
	uint8_t curTermNumber;
	uint8_t curTermOffset;
	bool sentenceHasFix;

	// custom element support
	friend class TinyGPSCustom;
	TinyGPSCustom *customElts;
	TinyGPSCustom *customCandidates;
	void insertCustom(TinyGPSCustom *pElt, const char *sentenceName, int index);

	// statistics
	uint32_t encodedCharCount;
	uint32_t sentencesWithFixCount;
	uint32_t failedChecksumCount;
	uint32_t passedChecksumCount;

	// internal utilities
	int fromHex(char a);
	bool endOfTermHandler();
};

#endif // def(__TinyGPSPlus_h)
