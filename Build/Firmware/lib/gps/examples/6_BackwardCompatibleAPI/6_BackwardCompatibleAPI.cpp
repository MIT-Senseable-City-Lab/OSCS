/* -----------------------------------------------------------
This example shows a lot of different features. As configured here
it'll check for a good GPS fix every 10 minutes and publish that data
if there is one. If not, it'll save you data by staying quiet. It also
registers 3 Particle.functions for changing whether it publishes,
reading the battery level, and manually requesting a GPS reading.
---------------------------------------------------------------*/

#include "Particle.h"

// Getting the library
#include "AssetTrackerRK.h"

int transmitMode(String command);
int gpsPublish(String command);
int batteryStatus(String command);

// Set whether you want the device to publish data to the internet by default here.
// 1 will Particle.publish AND Serial.print, 0 will just Serial.print
// Extremely useful for saving data while developing close enough to have a cable plugged in.
// You can also change this remotely using the Particle.function "tmode" defined in setup()
int transmittingData = 1;

// Used to keep track of the last time we published data
long lastPublish = 0;

// How many minutes between publishes? 10+ recommended for long-time continuous publishing!
int delayMinutes = 1;

// Creating an AssetTracker named 't' for us to reference
AssetTracker t = AssetTracker();

// A FuelGauge named 'fuel' for checking on the battery state
FuelGauge fuel;

// setup() and loop() are both required. setup() runs once when the device starts
// and is used for registering functions and variables and initializing things
void setup() {
    // Opens up a Serial port so you can listen over USB
    Serial.begin(9600);

    // These three functions are useful for remote diagnostics. Read more below.
    Particle.function("tmode", transmitMode);
    Particle.function("batt", batteryStatus);
    Particle.function("gps", gpsPublish);

    // Sets up all the necessary AssetTracker bits
    t.begin();
    
    // If using an external antenna, uncomment this line:
    // t.antennaExternal();

    // Enable the GPS module. Defaults to off to save power. 
    // Takes 1.5s or so because of delays.
    t.gpsOn();
}

// loop() runs continuously
void loop() {
    // You'll need to run this every loop to capture the GPS output
    t.updateGPS();

    // if the current time - the last time we published is greater than your set delay...
    if(millis()-lastPublish > delayMinutes*60*1000){
        // Remember when we published
        lastPublish = millis();
        
        //String pubAccel = String::format("%d,%d,%d",t.readX(),t.readY(),t.readZ());
        //Serial.println(pubAccel);
        //Particle.publish("A", pubAccel, 60, PRIVATE);
        
        // Dumps the full NMEA sentence to serial in case you're curious
        // Serial.println(t.preNMEA());
        
        // GPS requires a "fix" on the satellites to give good data,
        // so we should only publish data if there's a fix
        if(t.gpsFix()){
            // Only publish if we're in transmittingData mode 1;
            if(transmittingData){
                // Short publish names save data!
                Particle.publish("G", t.readLatLon(), 60, PRIVATE);
            }
            // but always report the data over serial for local development
            Serial.println(t.readLatLon());
        }
        else {
        	Serial.println("no fix");
        }
    }
}

// Allows you to remotely change whether a device is publishing to the cloud
// or is only reporting data over Serial. Saves data when using only Serial!
// Change the default at the top of the code.
int transmitMode(String command){
    transmittingData = atoi(command);
    return 1;
}

// Actively ask for a GPS reading if you're impatient. Only publishes if there's
// a GPS fix, otherwise returns '0'
int gpsPublish(String command){
    if(t.gpsFix()){ 
        Particle.publish("G", t.readLatLon(), 60, PRIVATE);
        
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    }
    else { return 0; }
}

// Lets you remotely check the battery status by calling the function "batt"
// Triggers a publish with the info (so subscribe or watch the dashboard)
// and also returns a '1' if there's >10% battery left and a '0' if below
int batteryStatus(String command){
    // Publish the battery voltage and percentage of battery remaining
    // if you want to be really efficient, just report one of these
    // the String::format("%f.2") part gives us a string to publish,
    // but with only 2 decimal points to save space
    Particle.publish("B", 
          "v:" + String::format("%.2f",fuel.getVCell()) + 
          ",c:" + String::format("%.2f",fuel.getSoC()),
          60, PRIVATE
    );
    // if there's more than 10% of the battery left, then return 1
    if(fuel.getSoC()>10){ return 1;} 
    // if you're running out of battery, return 0
    else { return 0;}
}
