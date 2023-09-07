#pragma once
#include "cityscanner_config.h"
#include "cityscanner_store.h"
#include "cityscanner_sense.h"
#include "cityscanner_vitals.h"
#include "CS_core.h"
#include "location_service.h"
#include "motion_service.h"


#define SECONDS 0
#define MINUTES 1
#define HOURS 2

class CitySleep {
    public:
        static CitySleep &instance() {
            if(!_instance) {
                _instance = new CitySleep();
            }
            return *_instance;
        }

        CS_core &core;
        CitySense &sense;
        CityStore &store;
        CityVitals &vitals;
        LocationService &locationService;
        //MotionService &motionService;


      /**
         * @brief Initialize device for application setup()
         *
         * @retval SYSTEM_ERROR_NONE
         */
        int init();
        void stop();
        void hibernate(uint8_t duration, uint8_t type);
        

    private:
        CitySleep();
        static CitySleep* _instance;
};