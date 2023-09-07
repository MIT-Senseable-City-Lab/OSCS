// Master configuration
//#define MODE LOGGING
#define MODE PWRSAVE

//#define PILOT_CITY NYC                              //To set Particle's product ID
//#define PILOT_CITY_VERSION NYC_LATEST_VERSION       //To set Particle's product version

#define AUTOSLEEP FALSE             //Autosleep when inactive
#define INACTIVITY_TIME 320         //Seconds, triggers light sleep
#define DEBUG TRUE
 
#define HW_VERSION V4

#define OPC_ENABLED FALSE           //For Stockholm (Turn off) 
#define IR_ENABLED TRUE
#define CELLULAR_ON_STARTUP FALSE    // TRUE or FALSE
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

//#define TCP_ENDPOINT "54.224.77.206" //MIT BACKEND
#define TCP_ENDPOINT "3.236.127.62" //MIT BACKEND

#define NYC 11951
#define STOCKHOLM 11375
#define NYC_LATEST_VERSION 3
#define STOCKHOLM_LATEST_VERSION 1

// Device types - Thermal and OPC
#define THERMAL_DEVICE  0
#define OPC_DEVICE      1

#define DEVICE_TYPE THERMAL_DEVICE

//#define MODEM_JUMPER_DISCONNECTED 1       // Modem jumper is not present on the board (disconnected)
//#define MODEM_JUMPER_CONNECTED 0          // Modem jumper is present on the board (connected)

