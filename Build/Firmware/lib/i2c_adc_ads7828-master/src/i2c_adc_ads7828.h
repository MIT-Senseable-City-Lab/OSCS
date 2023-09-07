/// \file
/// Arduino library for TI ADS7828 I2C A/D converter.
/*

  i2c_adc_ads7828.h - Arduino library for TI ADS7828 I2C A/D converter

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


/// \mainpage Arduino library for TI ADS7828 I2C A/D converter.
/// \version \verbinclude VERSION
/// \date 27 Sep 2016
/// \par Source Code Repository:
///   https://github.com/4-20ma/i2c_adc_ads7828
/// \par Programming Style Guidelines:
///   http://geosoft.no/development/cppstyle.html
/// 
/// \par Features
/// The ADS7828 is a single-supply, low-power, 12-bit data acquisition 
/// device that features a serial I2C interface and an 8-channel 
/// multiplexer. The Analog-to-Digital (A/D) converter features a 
/// sample-and-hold amplifier and internal, asynchronous clock. The 
/// combination of an I2C serial, 2-wire interface and micropower 
/// consumption makes the ADS7828 ideal for applications requiring the A/D 
/// converter to be close to the input source in remote locations and for 
/// applications requiring isolation. The ADS7828 is available in a TSSOP-16 
/// package. 
/// \par Schematic
///   \verbinclude SCHEMATIC
/// \par Caveats
///   Conforms to Arduino IDE 1.5 Library Specification v2.1 which requires
///   Arduino IDE >= 1.5.
/// \par Support
/// Please [submit an issue](https://github.com/4-20ma/i2c_adc_ads7828/
/// issues) for all questions, bug reports, and feature requests. Email
/// requests will be politely redirected to the issue tracker so others may
/// contribute to the discussion and requestors get a more timely response.
/// \author Doc Walker ([4-20ma@wvfans.net](mailto:4-20ma@wvfans.net))
/// \copyright 2009-2016 Doc Walker
/// \par License
/// <pre>
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
/// <span>
///     http://www.apache.org/licenses/LICENSE-2.0
/// <span>
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
/// </pre>


#ifndef i2c_adc_ads7828_h
#define i2c_adc_ads7828_h

// _________________________________________________________ STANDARD INCLUDES
// include types & constants of Wiring core API
#include "Arduino.h"


// __________________________________________________________ PROJECT INCLUDES
// include twi/i2c library
#include <Wire.h>


// ____________________________________________________________ UTILITY MACROS


// _________________________________________________________________ CONSTANTS
/// Configure channels to use differential inputs (Command byte SD=0).
/// Use either \ref DIFFERENTIAL or \ref SINGLE_ENDED in ADS7828
///   constructor; default is \ref DIFFERENTIAL.
/// \par Usage:
/// \code
/// ...
/// // address 0, differential inputs, ref/ADC OFF between conversions
/// ADS7828 adc0(0, DIFFERENTIAL | REFERENCE_OFF | ADC_OFF);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t DIFFERENTIAL        = 0 << 7; // SD == 0


/// Configure channels to use single-ended inputs (Command byte SD=1).
/// Use either \ref DIFFERENTIAL or \ref SINGLE_ENDED in ADS7828
///   constructor; default is \ref DIFFERENTIAL.
/// \par Usage:
/// \code
/// ...
/// // address 1, single-ended inputs, ref/ADC OFF between conversions
/// ADS7828 adc1(1, SINGLE_ENDED | REFERENCE_OFF | ADC_OFF);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t SINGLE_ENDED         = 1 << 7; // SD == 1


/// Configure channels to turn internal reference OFF between conversions (Command byte PD1=0).
/// Use either \ref REFERENCE_OFF or \ref REFERENCE_ON in ADS7828
///   constructor; default is \ref REFERENCE_OFF.
/// \par Usage:
/// \code
/// ...
/// // address 0, differential inputs, ref/ADC OFF between conversions
/// ADS7828 adc0(0, DIFFERENTIAL | REFERENCE_OFF | ADC_OFF);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t REFERENCE_OFF        = 0 << 3; // PD1 == 0


/// Configure channels to turn internal reference ON between conversions (Command byte PD1=1).
/// Use either \ref REFERENCE_OFF or \ref REFERENCE_ON in ADS7828
///   constructor; default is \ref REFERENCE_OFF.
/// \par Usage:
/// \code
/// ...
/// // address 2, differential inputs, ref ON/ADC OFF between conversions
/// ADS7828 adc2(2, DIFFERENTIAL | REFERENCE_ON | ADC_OFF);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t REFERENCE_ON         = 1 << 3; // PD1 == 1


/// Configure channels to turn A/D converter OFF between conversions (Command byte PD0=0).
/// Use either \ref ADC_OFF or \ref ADC_ON in ADS7828
///   constructor; default is \ref ADC_OFF.
/// \par Usage:
/// \code
/// ...
/// // address 0, differential inputs, ref/ADC OFF between conversions
/// ADS7828 adc0(0, DIFFERENTIAL | REFERENCE_OFF | ADC_OFF);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t ADC_OFF              = 0 << 2; // PD0 == 0


/// Configure channels to turn A/D converter ON between conversions (Command byte PD0=1).
/// Use either \ref ADC_OFF or \ref ADC_ON in ADS7828
///   constructor; default is \ref ADC_OFF.
/// \par Usage:
/// \code
/// ...
/// // address 3 , differential inputs, ref OFF/ADC ON between conversions
/// ADS7828 adc3(3, DIFFERENTIAL | REFERENCE_OFF | ADC_ON);
/// ...
/// \endcode
/// \relates ADS7828
static const uint8_t ADC_ON               = 1 << 2; // PD0 == 1


/// Default channel mask used in ADS7828 constructor.
/// \relates ADS7828
static const uint8_t DEFAULT_CHANNEL_MASK  = 0xFF;


/// Default scaling minimum value used in ADS7828 constructor.
/// \relates ADS7828
static const uint16_t DEFAULT_MIN_SCALE    = 0;


/// Default scaling maximum value used in ADS7828 constructor.
/// \relates ADS7828
static const uint16_t DEFAULT_MAX_SCALE    = 0xFFF;


// _________________________________________________________ CLASS DEFINITIONS
class ADS7828;
class ADS7828Channel
{
  public:
    // ............................................... public member functions
    ADS7828Channel() {};
    ADS7828Channel(ADS7828* const, uint8_t, uint8_t, uint16_t, uint16_t);
    uint8_t commandByte();
    ADS7828* device();
    uint8_t id();
    uint8_t index();
    void newSample(uint16_t);
    void reset();
    uint16_t sample();
    uint8_t start();
    uint16_t total();
    uint8_t update();
    uint16_t value();

    // ........................................ static public member functions

    // ..................................................... public attributes
    /// Maximum value of moving average (defaults to 0x0FFF).
    /// \par Usage:
    /// \code
    /// ...
    /// ADS7828 device(0);
    /// ADS7828Channel* temperature = device.channel(0);
    /// uint16_t old = temperature->maxScale; // get current value and/or
    /// temperature->maxScale = 100;          // set new value
    /// ...
    /// \endcode
    uint16_t maxScale;

    /// Minimum value of moving average (defaults to 0x0000).
    /// \par Usage:
    /// \code
    /// ...
    /// ADS7828 device(0);
    /// ADS7828Channel* temperature = device.channel(0);
    /// uint16_t old = temperature->minScale; // get current value and/or
    /// temperature->minScale = 0;            // set new value
    /// ...
    /// \endcode
    uint16_t minScale;

    // .............................................. static public attributes

  private:
    // .............................................. private member functions

    // ....................................... static private member functions

    // .................................................... private attributes
    /// Command byte for channel object (SD C2 C1 C0 bits only).
    uint8_t commandByte_;

    /// Pointer to parent device object.
    ADS7828* device_;

    /// Index position within moving average array. 
    uint8_t index_;

    /// Array of (unscaled) sample values.
    /// \note Bit shift must match \ref MOVING_AVERAGE_BITS_.
    uint16_t samples_[1 << 4];

    /// (Unscaled) running total of moving average array elements.
    uint16_t total_;

    // ............................................. static private attributes
    /// Quantity of samples to be averaged =
    ///   2<sup>\ref MOVING_AVERAGE_BITS_</sup>.
    /// \note \ref MOVING_AVERAGE_BITS_ must match \ref samples_ bit shift.
    static const uint8_t MOVING_AVERAGE_BITS_ = 4;
};


class ADS7828
{
  public:
    // ............................................... public member functions
    ADS7828(uint8_t);
    ADS7828(uint8_t, uint8_t);
    ADS7828(uint8_t, uint8_t, uint8_t);
    ADS7828(uint8_t, uint8_t, uint8_t, uint16_t, uint16_t);
    uint8_t address();
    ADS7828Channel* channel(uint8_t);
    uint8_t commandByte();
    uint8_t start();
    uint8_t start(uint8_t);
    uint8_t update(); // single device, all unmasked channel
    uint8_t update(uint8_t); // single device, single channel

    // ........................................ static public member functions
    static void begin();
    static ADS7828* device(uint8_t);
    static uint8_t updateAll(); // all devices, all unmasked channels

    // ..................................................... public attributes
    /// Each bit position containing a 1 represents a channel that is to be
    /// read via update() / updateAll().
    uint8_t channelMask;                    // mask of active channels

    // .............................................. static public attributes

  private:
    // .............................................. private member functions
    void init(uint8_t, uint8_t, uint8_t, uint16_t, uint16_t);
    uint16_t read();

    // ....................................... static private member functions
    static uint16_t read(uint8_t);
    static uint8_t start(uint8_t, uint8_t);
    static uint8_t update(ADS7828*); // single device, all unmasked channels
    static uint8_t update(ADS7828*, uint8_t); // single device, single channel

    // .................................................... private attributes
    /// Device address as defined by pins A1, A0
    uint8_t address_;

    /// Array of channel objects.
    ADS7828Channel channels_[8];

    /// Command byte for device object (PD1 PD0 bits only).
    uint8_t commandByte_;

    // ............................................. static private attributes
    /// Array of pointers to registered device objects.
    static ADS7828* devices_[4];

    /// Factory pre-set slave address.
    static const uint8_t BASE_ADDRESS_ = 0x48;
};
#endif
/// \example examples/one_device/one_device.ino
/// \example examples/two_devices/two_devices.ino
