
#include "LegacyAdapter.h"


LegacyAdapter::LegacyAdapter(TinyGPSPlus &gpsData) : gpsData(gpsData) {

}

LegacyAdapter::~LegacyAdapter() {

}


float LegacyAdapter::convertToDegreesMinutes(double deg) const {
	// This more or less reconstitutes the weird data format from the GPS.
	// According to the NMEA Standard, Latitude and Longitude are output in the format Degrees, Minutes and (Decimal) Fractions of Minutes. To convert to Degrees and Fractions of Degrees, or Degrees, Minutes, Seconds and Fractions of seconds, the 'Minutes' and 'Fractional Minutes' parts need to be converted. In other words: If the GPS Receiver reports a Latitude of 4717.112671 North and Longitude of 00833.914843 East, this is
	// Latitude 47 Degrees, 17.112671 Minutes
	// Longitude 8 Degrees, 33.914843 Minutes
	// or
	// Latitude 47 Degrees, 17 Minutes, 6.76026 Seconds Longitude 8 Degrees, 33 Minutes, 54.89058 Seconds or
	// Latitude 47.28521118 Degrees Longitude 8.56524738 Degrees
	//
	// A minute is 1/60 degree, or there are 60 minutes in a degree.
	// minutes = deg * 60;
	// deg = min / 60;

	double posDeg = fabs(deg);

	double floorPosDeg = floor(posDeg);

	double minutes = (posDeg - floorPosDeg) * 60.0;

	// printf("floorPosDeg=%lf minutes=%lf\n", floorPosDeg, minutes);

	return (float)(floorPosDeg * 100.0 + minutes);
}



/*
 * Adafruit Algorithm:
 *
  	  Given GPS data  like: 4228.21303


  	  strncpy(degreebuff, p, 2);
      p += 2;
      degreebuff[2] = '\0';
      degree = atol(degreebuff) * 10000000;

      // That's the degree part, times 10,000,000
      // 42 -> 420000000

      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';

      // That converts 28.21303 to 2821303

      // Take the minutes part, including the part before the decimal
      // To make things really confusing, the variable minutes actually contains degrees * 1000
      // 50 * min / 3 = 16.667 * min = min * 1000 / 60
      minutes = 50 * atol(degreebuff) / 3;

      latitude_fixed = degree + minutes;

      // degree / 100000 = hundreds of degrees, like the GPS format: 42xx.xxxxx
      // since minutes actually contains degrees * 1000, this converts it back to minutes
      latitude = degree / 100000 + minutes * 0.000006F;


      latitudeDegrees = (latitude-100*int(latitude/100))/60.0;
      latitudeDegrees += int(latitude/100);
 */
