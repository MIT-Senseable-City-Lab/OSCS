// This #include statement was automatically added by the Particle IDE.

#include "HTS221.h"
//#include "application.h"
//#include "Particle.h"

/*
   humidity.cpp
   Example on SmartEverything humidity / temperature sensor reading
   Created: 4/27/2015 10:32:11 PM
    Author: speirano
*/

 HTS221 smeHumidity;
// the setup function runs once when you press reset or power the board
void setup() {
  //Initiate the Wire library and join the I2C bus
  Wire.begin();
  //pinMode(PIN_LED_13, OUTPUT);

  smeHumidity.begin();
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {

  double data = 0;

  data = smeHumidity.readHumidity();
  Serial.print("Humidity   : ");
  Serial.print(data);
  Serial.println(" %");

  data = smeHumidity.readTemperature();
  Serial.print("Temperature: ");
  Serial.print(data);
  Serial.println(" celsius");

  //digitalWrite(PIN_LED_13, LOW);
  //delay(300);

  //digitalWrite(PIN_LED_13, HIGH);       // turn the LED on
  delay(1200);              // wait for a second

}