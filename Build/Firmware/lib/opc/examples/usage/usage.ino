// Example usage for CS_opcn3 library by flat.

#include "OPCN3.h"

// Initialize objects from the lib
OPCN3 myOPCN3(D5);

void setup() {

  Serial.begin(9600);

  // turn on laser, fan and set high gain
  myOPCN3.initialize();
  delay(1000);

}

void loop() {

  delay(10000);
  HistogramData hist = myOPCN3.readHistogramData();

  // Get Temperature
  Serial.print("Temperature: ");
  Serial.println(hist.getTempC());

  // Get PM values
  Serial.print("PM 10: ");
  Serial.println(hist.pm10);

}