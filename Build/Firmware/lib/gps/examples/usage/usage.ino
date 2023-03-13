// Example usage for CS_gps library by flat.

#include "CS_gps.h"

// Initialize objects from the lib
CS_gps cS_gps;

void setup() {
    // Call functions on initialized library objects that require hardware
    cS_gps.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    cS_gps.process();
}
