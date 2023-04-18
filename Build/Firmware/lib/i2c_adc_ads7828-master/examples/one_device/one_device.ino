/*

  one_device.ino - example using i2c_adc_ads7828 library

  Library:: i2c_adc_ads7828
  Author:: Doc Walker <4-20ma@wvfans.net>

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/


#include <i2c_adc_ads7828.h>


// device 0
// Address: A1=0, A0=0
// Command: SD=1, PD1=1, PD0=1
ADS7828 device(0, SINGLE_ENDED | REFERENCE_ON | ADC_ON, 0x0F);
ADS7828* adc = &device;
ADS7828Channel* ambientTemp = adc->channel(0);
ADS7828Channel* waterTemp = adc->channel(1);
ADS7828Channel* filterPressure = adc->channel(2);
ADS7828Channel* waterLevel = adc->channel(3);


void setup()
{
  // enable serial monitor
  Serial.begin(19200);

  // enable I2C communication
  ADS7828::begin();

  // adjust scaling on an individual channel basis
  ambientTemp->minScale = 0;
  ambientTemp->maxScale = 150;

  waterTemp->minScale = 0;
  waterTemp->maxScale = 100;

  filterPressure->minScale = 0;
  filterPressure->maxScale = 30;

  waterLevel->minScale = 0;
  waterLevel->maxScale = 100;
}


void loop()
{
  // update all registered ADS7828 devices/unmasked channels
  ADS7828::updateAll();

  // output moving average values to console
  Serial.print("\n Ambient: ");
  Serial.print(ambientTemp->value(), DEC);
  Serial.print("\n Water temp: ");
  Serial.print(waterTemp->value(), DEC);
  Serial.print("\n Filter pressure: ");
  Serial.print(filterPressure->value(), DEC);
  Serial.print("\n Water level: ");
  Serial.print(waterLevel->value(), DEC);
  Serial.print("\n- - - - - - - - - - - - - - - - - - - - \n");

  // delay
  delay(1000);
}
