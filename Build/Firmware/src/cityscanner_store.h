#pragma once
#include "cityscanner_CONFIG.h"
#include "SD.h"
#include "SPI.h"
#include "location_service.h"
#define ALL_FILES -1

#define BROADCAST_NONE 0
#define BROADCAST_IMMEDIATE 1
#define BROADCAST_DELAYED 2

enum payloadType {
  Data,
  Vitals,
  Warning
};

class CityStore {
    public:
        static CityStore &instance() {
            if(!_instance) {
                _instance = new CityStore();
            }
            return *_instance;
        }
        /**
         * @brief Initialize device for application setup()
         *
         * @retval SYSTEM_ERROR_NONE
         */
        int init();
        int stop();
        void reInit();
        unsigned int records = 20;
        const uint8_t chipSelect = SS; 
        int switch_logfile();
        void logData(int broadcastType, int payloadType, String data);
        void writeData(String data);
        bool dumpData(int files_to_dump);
        int countFilesInQueue();
        String deviceID = "na";
    
    private:
        CityStore();
        static CityStore* _instance;
        File activeFile;
        unsigned int cnt = 1;
        TCPClient client;
        const char* s3endpoint = "0";
        
        bool deleteAll(bool removeDirs);
        void delFiles(const char *folder_name);
       
};