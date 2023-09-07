/*

  two_devices.ino - example using i2c_adc_ads7828 library

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


// device 1
// Address: A1=0, A0=1
// Command: SD=1, PD1=1, PD0=1
ADS7828 device1(1, SINGLE_ENDED | REFERENCE_ON | ADC_ON, 0xFF);

// device 2
// Address: A1=1, A0=0
// Command: SD=1, PD1=1, PD0=1
// Scaling: min=0, max=1000
ADS7828 device2(2, SINGLE_ENDED | REFERENCE_ON | ADC_ON, 0xFF, 0, 1000);

void setup()
{
  // enable serial monitor
  Serial.begin(19200);

  // enable I2C communication
  ADS7828::begin();
}


void loop()
{
  uint8_t a, ch;

  // update all registered ADS7828 devices/unmasked channels
  ADS7828::updateAll();

  // iterate through device 1..2 channels 0..7
  for (a = 1; a <= 2; a++)
  {
    for (ch = 0; ch < 8; ch++)
    {
      serialPrint(ADS7828::device(a)->channel(ch));
    }
  }
  Serial.print("\n");

  // output moving average values to console
  Serial.print("\n- - - - - - - - - - - - - - - - - - - - \n");

  // delay
  delay(1000);
}


void serialPrint(ADS7828Channel* ch)
{
  // device address (0..3)
  Serial.print("\nAD:");
  Serial.print(ch->device()->address(), DEC);

  // channel ID (0..7)
  Serial.print(", CH:");
  Serial.print(ch->id(), DEC);

  // moving average value (scaled)
  Serial.print(", v:");
  Serial.print(ch->value(), DEC);

  // minimum scale applied to moving average value
  Serial.print(", mn:");
  Serial.print(ch->minScale, DEC);

  // maximum scale applied to moving average value
  Serial.print(", mx:");
  Serial.print(ch->maxScale, DEC);
}
