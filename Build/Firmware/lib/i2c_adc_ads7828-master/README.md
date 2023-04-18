# i2c_adc_ads7828
[![GitHub release](https://img.shields.io/github/release/4-20ma/i2c_adc_ads7828.svg?maxAge=3600)][GitHub release]
[![Travis](https://img.shields.io/travis/4-20ma/i2c_adc_ads7828.svg?maxAge=3600)][Travis]
[![license](https://img.shields.io/github/license/4-20ma/i2c_adc_ads7828.svg?maxAge=3600)][license]
[![code of conduct](https://img.shields.io/badge/%E2%9D%A4-code%20of%20conduct-blue.svg?maxAge=3600)][code of conduct]

[GitHub release]:   https://github.com/4-20ma/i2c_adc_ads7828
[Travis]:           https://travis-ci.org/4-20ma/i2c_adc_ads7828
[license]:          LICENSE
[code of conduct]:  CODE_OF_CONDUCT.md


## Overview
This is an Arduino library for the Texas Instruments ADS7828 12-bit, 8-channel I<sup>2</sup>C A/D converter.


## Features
The following features are available:

  - Up to (4) A/D converters can be used on the same I<sup>2</sup>C bus (hardware-addressable via pins A0, A1 and software-addressable via ID 0..3; address 0x48..0x4C)
  - A/D conversions may be initiated on a bus-, device-, or channel-specific level
  - Retrieve values as 16-period moving average or last sample
  - Built-in scaling function to return values in user-defined engineering units


## Installation

#### Library Manager
Install the library into your Arduino IDE using the Library Manager (available from IDE version 1.6.2). Open the IDE and click Sketch > Include Library > Manage Libraries&hellip;

Scroll or search for `i2c_adc_ads7828`, then select the version of the library you want to install. Quit/re-launch the IDE to refresh the list; new versions are automatically added to the list, once released on GitHub.

Refer to Arduino Tutorials > Libraries [Using the Library Manager](https://www.arduino.cc/en/Guide/Libraries#toc3).

#### Zip Library
Refer to Arduino Tutorials > Libraries [Importing a .zip Library](https://www.arduino.cc/en/Guide/Libraries#toc4).

#### Manual
Refer to Arduino Tutorials > Libraries [Manual Installation](https://www.arduino.cc/en/Guide/Libraries#toc5).


## Schematic
This library has been tested with an Arduino [Duemilanove](http://www.arduino.cc/en/Main/ArduinoBoardDuemilanove) and a Texas Instruments [ADS7828](http://focus.ti.com/docs/prod/folders/print/ads7828.html) A/D converter.

Below is a simplified schematic diagram. Refer to the datasheet for specific requirements.

![Figure 1 - Schematic Diagram](SCHEMATIC.svg)

## Example
The library contains sketches that demonstrates use of the `i2c_adc_ads7828` library. You can find these in the [examples](/examples/) folder.

``` cpp
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
```


## Caveats
Conforms to Arduino IDE 1.5 Library Specification v2.1 which requires Arduino IDE >= 1.5.


## Support
Please [submit an issue](https://github.com/4-20ma/i2c_adc_ads7828/issues) for all questions, bug reports, and feature requests. Email requests will be politely redirected to the issue tracker so others may contribute to the discussion and requestors get a more timely response.


## License & Authors

- Author:: Doc Walker ([4-20ma@wvfans.net](mailto:4-20ma@wvfans.net))

```
Copyright:: 2009-2019 Doc Walker

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
