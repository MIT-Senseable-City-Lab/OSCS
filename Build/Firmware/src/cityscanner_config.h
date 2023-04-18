// Master configuration
#define MODE LOGGING   


#define AUTOSLEEP TRUE             //Autosleep when inactive
#define INACTIVITY_TIME 320         //Seconds, triggers light sleep
#define DEBUG TRUE

#define HW_VERSION V4

#define OPC_ENABLED TRUE
#define IR_ENABLED FALSE
#define CELLULAR_ON_STARTUP TRUE
#define DTIME 100 

#define HARVARD_PILOT FALSE         //Is this an Harvard pilot device?
#define OLD_TEMPERATURE_SENSOR FALSE
#define BATT_ENABLED TRUE

#define OPC_DATA_VERSION EXTENDED       // BASE or EXTENDED for full BIN data
#define TCP_GHOSTWRITE FALSE         //For testing purpose, doesn't dump data over TCP but prints it over serial
#define SD_FORMAT_ONSTARTUP FALSE   //Erase SD Card on startup

// Data sampling
#define SAMPLE_RATE 5 //Seconds (for harvard 5s)
#define VITALS_RATE 30 //Seconds
#define ROUTINE_RATE 60 //seconds

// Data Storage and Broadcasting
#define RECORDS_PER_FILE 200 //standard is 200
#define LOW_BATTERY_THRESHOLD 3.80 //volt


#define TCP_ENDPOINT "127.0.0.1" //change the IP address to dump data over TCP

#define NYC 11951
#define STOCKHOLM 11375
#define NYC_LATEST_VERSION 3
#define STOCKHOLM_LATEST_VERSION 1


           
