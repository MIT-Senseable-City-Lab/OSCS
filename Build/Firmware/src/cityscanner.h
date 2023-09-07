#pragma once
#include "cityscanner_config.h"
#include "cityscanner_store.h"
#include "cityscanner_sense.h"
#include "cityscanner_vitals.h"
#include "cityscanner_sleep.h"
#include "CS_core.h"
#include "location_service.h"
#include "motion_service.h"
#include "cityscanner_cellular.h"


class Cityscanner {
    public:
        static Cityscanner &instance() {
            if(!_instance) {
                _instance = new Cityscanner();
            }
            return *_instance;
        }

        CS_core &core;
        CitySense &sense;
        CityStore &store;
        CityVitals &vitals;
        LocationService &locationService;
        MotionService &motionService;
        CityCellular &cellular;
        
        enum Modes{
            IDLE,
            REALTIME,
            LOGGING,
            PWRSAVE,
            TEST
        };

        String data_payload = "na";
        String vitals_payload = "na";
        
        static void startup();
        int init();
        void loop();
        void startShippingMode();
        void checkbattery();
        void sendWarning(String);
        int counter = 0;
        bool debug_mode = DEBUG;
    
    private:
        Cityscanner();
        static Cityscanner* _instance;
        void printDebug();
};