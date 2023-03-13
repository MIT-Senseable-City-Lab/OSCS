#include "DFRobot_SHT20.h"

// Initialize objects from the lib
DFRobot_SHT20 sht20;

void setup() {
  // Call functions on initialized library objects that require hardware  
  sht20.initSHT20();        // External temp monitor
  delay(100);
  sht20.checkSHT20();       // Check SHT20 Sensor
}

void loop() {
    // Use the library's initialized objects and functions
    
    
float temperature_ext = sht20.readTemperature();
float humidity_ext = sht20.readHumidity();

}
