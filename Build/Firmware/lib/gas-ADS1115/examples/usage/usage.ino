// Example usage for CS_gasads1115 library by Adafruit.

#include "Adafruit_ADS1015.h"
#include "CS_core.h"

// Initialize objects from the lib
Adafruit_ADS1115 ads; 

void setup() {
    // Call functions on initialized library objects that require hardware
    Serial.begin();
    CS_core_handler.begin();
    ads.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    int16_t adc0, adc1, adc2, adc3;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  Serial.print("SN1 Working: "); Serial.println(adc0);
  Serial.print("SN1 Reference: "); Serial.println(adc1);
  Serial.print("SN2 Working: "); Serial.println(adc2);
  Serial.print("SN2 Reference: "); Serial.println(adc3);
  Serial.println(" ");
  
  delay(1000);
}
