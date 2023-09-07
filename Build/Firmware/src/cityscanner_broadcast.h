#pragma once
#include "cityscanner_config.h"


class CityBroadcast {
    public:
        static CityBroadcast &instance() {
            if(!_instance) {
                _instance = new CityBroadcast();
            }
            return *_instance;
        }

        /**
         * @brief Initialize device for application setup()
         *
         * @retval SYSTEM_ERROR_NONE
         */
        int init();
    
    private:
        CityBroadcast();
        static CityBroadcast* _instance;
};