#include "location_service.h"
#include "TinyGPS++.h"
#include "AssetTrackerRK.h"
#include "CS_core.h"
#include "TimeLib.h"
#include "LegacyAdapter.h"
#include <Wire.h>

LocationService *LocationService::_instance = nullptr;
AssetTracker gps;

tmElements_t gpstime;
LocationService::LocationService() {
}

int LocationService::start()
{
    if(!location_started){

    CS_core::instance().enableGPS(1);
    CS_core::instance().activateGPS(1);
    gps.withI2C();
    gps.startThreadedMode();
    location_started = true;
    }
    return 1;
}

int LocationService::stop()
{
    if(location_started)
    {
    CS_core::instance().activateGPS(0);
    CS_core::instance().enableGPS(0);
    }
    return 1;
}

String LocationService::getGPSdata()
{
    if(location_started)
    {
    String gpsdata;
    gpsdata = String(gps.readLatDeg()) + "," + String(gps.readLonDeg());
    return gpsdata;
    }
    else 
    return "na,na";
}
String LocationService::getGPStime()
{
	if (location_started){

		String gps_time =  String(gps.getHour()) +":" + String(gps.getMinute()) +":" + String(gps.getSeconds());
		String gps_date = String(gps.getDay()) +"/" + String(gps.getMonth()) +"/" + String(gps.getYear());
		return gps_date +" "+ gps_time;
	}
	else
	{
		return "00/00/0000";
	}
  
}

String LocationService::getEpochTime(void)
{
    tmElements_t gpsTime;
    char sEpochTime[20];
    time_t EpochTime;

    if (location_started) 
    { 
        gpsTime.Year = y2kYearToTm(gps.getYear());
        gpsTime.Month = gps.getMonth();
        gpsTime.Day = gps.getDay();
        gpsTime.Hour = gps.getHour();
        gpsTime.Minute = gps.getMinute();
        gpsTime.Second = gps.getSeconds();

        EpochTime = makeTime(gpsTime);
        itoa(EpochTime, sEpochTime, 10);

        return sEpochTime;
    } 
    else
    {
        gpsTime.Year = gpsTime.Year = y2kYearToTm(0);
        gpsTime.Month = 0;
        gpsTime.Day = 0;
        gpsTime.Hour = 0;
        gpsTime.Minute = 0;
        gpsTime.Second = 0;

        EpochTime = makeTime(gpsTime);
        itoa(EpochTime, sEpochTime, 10);

        return sEpochTime;
    }
}