#include "location_service.h"
#include "TinyGPS++.h"
#include "AssetTrackerRK.h"
#include "CS_core.h"

LocationService *LocationService::_instance = nullptr;
AssetTracker gps;

LocationService::LocationService() {
}

int LocationService::start()
{
    if(!location_started){
    CS_core::instance().enableGPS(1);
    CS_core::instance().activateGPS(1);
    gps.gpsOn();
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