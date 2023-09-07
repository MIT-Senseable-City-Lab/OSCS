#pragma once
#include "Particle.h"
#include "cityscanner_config.h"

class LocationService
{

public:
    /**
     * @brief Return instance of the motion service
     *
     * @retval LocationService&
     */

    static LocationService &instance()
    {
        if (!_instance)
        {
            _instance = new LocationService();
        }
        return *_instance;
    }
    int start();
    int stop();
    //int Year; //Year.
    //byte month, Day, Hour, Minute, Second;
    bool location_started = false;
    String getGPSdata(void);
    String getGPStime(void);
    String getEpochTime(void);

private:
    LocationService();
    static LocationService *_instance;
};