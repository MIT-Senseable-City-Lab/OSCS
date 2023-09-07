#pragma once
#include "cityscanner_config.h"
#include "Particle.h"
#include "SparkFun_SHTC3.h"


class CityVitals {
    public:
        static CityVitals &instance() {
            if(!_instance) {
                _instance = new CityVitals();
            }
            return *_instance;
        }

      /**
         * @brief Initialize device for application setup()
         *
         * @retval SYSTEM_ERROR_NONE
         */
        int init();
        int stop_all();
    
        bool startBattery(void);
        bool stopBattery(void);
        bool BATT_started = false;
        String getBatteryData(void);
        String getChargingStatus(void);
        bool isBatteryLow();
        float getBatteryVoltage();
        
        bool startSolar(void);
        bool stopSolar(void);
        bool SOLAR_started = false;
        String getSolarData(void);
        
        bool startTempInt(void);
        bool stopTempInt(void);
        bool TEMPint_started = false;
        String getTempIntData(void);
        void errorDecoder(SHTC3_Status_TypeDef message);                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way

        String getSignalStrenght();

    private:
        CityVitals();
        static CityVitals* _instance;
};