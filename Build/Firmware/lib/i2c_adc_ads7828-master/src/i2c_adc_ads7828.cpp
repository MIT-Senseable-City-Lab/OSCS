/*

  i2c_adc_ads7828.cpp - Arduino library for TI i2c_adc_ads7828 I2C A/D converter

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


// __________________________________________________________ PROJECT INCLUDES
#include "i2c_adc_ads7828.h"


// ___________________________________________________ PUBLIC MEMBER FUNCTIONS
/// \remark Invoked by ADS7828 constructor;
///   this function will not normally be called by end user.
ADS7828Channel::ADS7828Channel(ADS7828* const device, uint8_t id,
  uint8_t options, uint16_t min, uint16_t max)
{
  this->device_ = device;
  this->commandByte_ = (bitRead(options, 7) << 7) | (bitRead(id, 0) << 6) |
    (bitRead(id, 2) << 5) | (bitRead(id, 1) << 4);
  this->minScale = min;
  this->maxScale = max;
  reset();
}


/// Return command byte for channel object.
/// \optional This function is for testing and troubleshooting.
/// \return command byte (0x00..0xFC)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint8_t command = temperature->commandByte();
/// ...
/// \endcode
uint8_t ADS7828Channel::commandByte()
{
  return commandByte_ | device_->commandByte();
}


/// Return pointer to parent device object.
/// \return pointer to parent ADS7828 object
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// ADS7828* parentDevice = temperature->device();
/// ...
/// \endcode
ADS7828* ADS7828Channel::device()
{
  return device_;
}


/// Return ID number of channel object (+IN connection).
/// Single-ended inputs use COM as -IN; Differential inputs are as follows:
/// \arg 0 indicates CH0 as +IN, CH1 as -IN
/// \arg 1 indicates CH1 as +IN, CH0 as -IN
/// \arg 2 indicates CH2 as +IN, CH3 as -IN
/// \arg ...
/// \arg 7 indicates CH7 as +IN, CH6 as -IN
/// 
/// \return id (0..7)
/// \retval 0 command byte C2 C1 C0 = 000
/// \retval 1 command byte C2 C1 C0 = 100
/// \retval 2 command byte C2 C1 C0 = 001
/// \retval 3 command byte C2 C1 C0 = 101
/// \retval 4 command byte C2 C1 C0 = 010
/// \retval 5 command byte C2 C1 C0 = 110
/// \retval 6 command byte C2 C1 C0 = 011
/// \retval 7 command byte C2 C1 C0 = 111
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint8_t channelId = temperature->id();
/// ...
/// \endcode
uint8_t ADS7828Channel::id()
{
  return ((bitRead(commandByte_, 5) << 2) | (bitRead(commandByte_, 4) << 1) |
    (bitRead(commandByte_, 6)));
}


/// Return index position within moving average array.
/// \optional This function is for testing and troubleshooting.
/// \return index (0..2<sup>\ref MOVING_AVERAGE_BITS_</sup> - 1)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint8_t channelIndex = temperature->index();
/// ...
/// \endcode
uint8_t ADS7828Channel::index()
{
  return index_;
}


/// Add (unscaled) sample value to moving average array, update totalizer.
/// \param sample sample value (0x0000..0xFFFF)
/// \remark Invoked by ADS7828::update() / ADS7828::updateAll() functions;
///   this function will not normally be called by end user.
void ADS7828Channel::newSample(uint16_t sample)
{
  this->index_++;
  if (index_ >= (1 << MOVING_AVERAGE_BITS_)) this->index_ = 0;
  this->total_ -= samples_[index_];
  this->samples_[index_] = sample;
  this->total_ += samples_[index_];
}


/// Reset moving average array, index, totalizer to zero.
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// temperature->reset();
/// ...
/// \endcode
void ADS7828Channel::reset()
{
  this->index_ = this->total_ = 0;
  for (uint8_t k = 0; k < (1 << MOVING_AVERAGE_BITS_); k++)
  {
    this->samples_[k] = 0;
  }
}


/// Return most-recent (unscaled) sample value from moving average array.
/// \optional This function is for testing and troubleshooting.
/// \return sample value (0x0000..0xFFFF)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint16_t sampleValue = temperature->sample();
/// ...
/// \endcode
uint16_t ADS7828Channel::sample()
{
  return samples_[index_];
}


/// Initiate A/D conversion for channel object.
/// \optional This function is for testing and troubleshooting.
/// \todo Determine whether this function is needed.
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint8_t status = temperature->start();
/// ...
/// \endcode
uint8_t ADS7828Channel::start()
{
  return device_->start(id());
}


/// Return (unscaled) totalizer value for channel object.
/// \optional This function is for testing and troubleshooting.
/// \return totalizer value (0x0000..0xFFFF)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint16_t totalValue = temperature->total();
/// ...
/// \endcode
uint16_t ADS7828Channel::total()
{
  return total_;
}


/// Initiate A/D conversion, read data, update moving average for channel object.
/// \optional This function is for testing and troubleshooting.
/// \todo Determine whether this function is needed.
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint8_t status = temperature->update();
/// ...
/// \endcode
uint8_t ADS7828Channel::update()
{
  device_->update(id());
return 1;
}


/// Return moving average value for channel object.
/// \required This is the most commonly-used channel function.
/// \return scaled value (0x0000..0xFFFF)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// uint16_t ambient = temperature->value();
/// ...
/// \endcode
uint16_t ADS7828Channel::value()
{
  uint16_t r = (total_ >> MOVING_AVERAGE_BITS_);
  return map(r, DEFAULT_MIN_SCALE, DEFAULT_MAX_SCALE, minScale, maxScale);
}


// ____________________________________________ STATIC PUBLIC MEMBER FUNCTIONS


// __________________________________________________ PRIVATE MEMBER FUNCTIONS


// ___________________________________________ STATIC PRIVATE MEMBER FUNCTIONS


// ___________________________________________________ PUBLIC MEMBER FUNCTIONS
/// Constructor with the following defaults:
/// 
/// \arg differential inputs (SD=0)
/// \arg internal reference OFF between conversions (PD1=0)
/// \arg A/D converter OFF between conversions (PD0=0)
/// \arg min scale=0
/// \arg max scale=4095
/// 
/// \param address device address (0..3)
/// \par Usage:
/// \code
/// ...
/// // construct device with address 2
/// ADS7828 adc(2);
/// ...
/// \endcode
/// \sa ADS7828::address()
ADS7828::ADS7828(uint8_t address)
{
  init(address, (DIFFERENTIAL | REFERENCE_OFF | ADC_OFF),
    DEFAULT_CHANNEL_MASK, DEFAULT_MIN_SCALE, DEFAULT_MAX_SCALE);
}


/// \overload ADS7828::ADS7828(uint8_t address, uint8_t options)
/// \param options command byte bits SD, PD1, PD0
/// \par Usage:
/// \code
/// ...
/// // device address 0, differential inputs, ref/ADC ON between conversions
/// ADS7828 adc0(0, DIFFERENTIAL | REFERENCE_ON | ADC_ON);
/// 
/// // device address 1, single-ended inputs, ref/ADC OFF between conversions
/// ADS7828 adc1(1, SINGLE_ENDED | REFERENCE_OFF | ADC_OFF);
/// 
/// // device address 2, single-ended inputs, ref/ADC ON between conversions
/// ADS7828 adc2(2, SINGLE_ENDED | REFERENCE_ON | ADC_ON);
/// ...
/// \endcode
/// \sa ADS7828Channel::commandByte()
ADS7828::ADS7828(uint8_t address, uint8_t options)
{
  init(address, options, DEFAULT_CHANNEL_MASK, DEFAULT_MIN_SCALE,
    DEFAULT_MAX_SCALE);
}


/// \overload ADS7828::ADS7828(uint8_t address, uint8_t options, uint8_t channelMask)
/// \param channelMask bit positions containing a 1 represent channels that
///   are to be read via update() / updateAll()
/// \par Usage:
/// \code
/// ...
/// // device address 0, update all channels via updateAll() (bits 7..0 are set)
/// ADS7828 adc0(0, 0, 0xFF);
/// 
/// // device address 1, update channels 0..3 via updateAll() (bits 3..0 are set)
/// ADS7828 adc1(1, 0, 0b00001111);
/// 
/// // device address 2, update channels 0, 1, 2, 7 via updateAll() (bits 7, 2, 1, 0 are set)
/// ADS7828 adc2(2, 0, 0b10000111);
/// ...
/// \endcode
/// \sa ADS7828::channelMask
ADS7828::ADS7828(uint8_t address, uint8_t options, uint8_t channelMask)
{
  init(address, options, channelMask, DEFAULT_MIN_SCALE, DEFAULT_MAX_SCALE);
}


/// \overload ADS7828::ADS7828(uint8_t address, uint8_t options, uint8_t channelMask, uint16_t min, uint16_t max)
/// \param min minimum scaling value applied to value()
/// \param max maximum scaling value applied to value()
/// \par Usage:
/// \code
/// ...
/// // device address 2, channel default minScale 0, maxScale 100
/// ADS7828 adc(2, 0, DEFAULT_CHANNEL_MASK, 0, 100);
/// ...
/// \endcode
/// \sa ADS7828Channel::minScale, ADS7828Channel::maxScale
ADS7828::ADS7828(uint8_t address, uint8_t options, uint8_t channelMask,
  uint16_t min, uint16_t max)
{
  init(address, options, channelMask, min, max);
}


/// Device address as defined by pins A1, A0
/// \retval 0x00 A1=0, A0=0
/// \retval 0x01 A1=0, A0=1
/// \retval 0x02 A1=1, A0=0
/// \retval 0x03 A1=1, A0=1
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(3);
/// uint8_t deviceAddress = adc.address();
/// ...
/// \endcode
uint8_t ADS7828::address()
{
  return address_;
}


/// Return pointer to channel object.
/// \param ch channel number (0..7)
/// \return pointer to ADS7828Channel object
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ADS7828Channel* temperature = adc.channel(0);
/// ...
/// \endcode
ADS7828Channel* ADS7828::channel(uint8_t ch)
{
  return &channels_[ch & 0x07];
}


/// Return command byte for device object (PD1 PD0 bits only).
/// \optional This function is for testing and troubleshooting.
/// \retval 0x00 Power Down Between A/D Converter Conversions
/// \retval 0x04 Internal Reference OFF and A/D Converter ON
/// \retval 0x08 Internal Reference ON and A/D Converter OFF
/// \retval 0x0C Internal Reference ON and A/D Converter ON
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// uint8_t command = adc.commandByte();
/// ...
/// \endcode
uint8_t ADS7828::commandByte()
{
  return commandByte_;
}


/// Initiate communication with device.
/// \optional This function is for testing and troubleshooting and
///   can be used to determine whether a device is available (similar to
///   the TCP/IP \c ping \c command).
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(3);
/// // test whether device is available
/// uint8_t status = adc.start();
/// ...
/// \endcode
uint8_t ADS7828::start()
{
  return start(0);
}


/// \overload ADS7828::start(uint8_t ch)
/// \optional This function is for testing and troubleshooting.
/// \param ch channel number (0..7)
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// // test whether device is available (channel 3 A/D conversion started)
/// uint8_t status = adc.start(3);
/// ...
/// \endcode
uint8_t ADS7828::start(uint8_t ch)
{
  return start(address_, commandByte_ | channel(ch)->commandByte());
}


/// Update all unmasked channels on device.
/// \required Call this or one of the update() / updateAll() functions
///   from within \c loop() in order to read data from device(s).
/// \return quantity of channels updated (0..8)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ...
/// void loop()
/// {
///   ...
///   // update device 0, all unmasked channels
///   uint8_t quantity = adc.update();
///   ...
/// }
/// ...
/// \endcode
uint8_t ADS7828::update()
{
  return update(this);
}


/// \overload uint8_t ADS7828::update(uint8_t ch)
/// \required Call this or one of the update() / updateAll() functions
///   from within \c loop() in order to read data from device(s).
/// \param ch channel number (0..7)
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
/// \par Usage:
/// \code
/// ...
/// ADS7828 adc(0);
/// ...
/// void loop()
/// {
///   ...
///   // update device 0, channel 3
///   uint8_t status = adc.update(3);
///   ...
/// }
/// ...
/// \endcode
uint8_t ADS7828::update(uint8_t ch)
{
  return update(this, ch);
}


// ____________________________________________ STATIC PUBLIC MEMBER FUNCTIONS
/// Enable I2C communication.
/// \required Call from within \c setup()\c to enable I2C communication.
/// \par Usage:
/// \code
/// ...
/// void setup()
/// {
///   // enable I2C communication
///   ADS7828::begin();
/// }
/// ...
/// \endcode
void ADS7828::begin()
{
  Wire.begin();
}


/// Return pointer to device object.
/// \param address device address (0..3)
/// \return pointer to ADS7828 object
/// \par Usage:
/// \code
/// ...
/// // device 2 pointer
/// ADS7828* device2 = ADS7828::device(2);
/// ...
/// \endcode
ADS7828* ADS7828::device(uint8_t address)
{
  return devices_[address & 0x03];
}


/// Update all unmasked channels on all registered devices.
/// \required Call this or one of the update() functions
///   from within \c loop() in order to read data from device(s).
///   This is the most commonly-used device update function.
/// \return quantity of channels updated (0..32)
/// \par Usage:
/// \code
/// ...
/// void loop()
/// {
///   ...
///   // update all registered ADS7828 devices/unmasked channels
///   uint8_t quantity = ADS7828::updateAll();
///   ...
/// }
/// ...
/// \endcode
uint8_t ADS7828::updateAll()
{
  uint8_t a, ch, count = 0;
  for (a = 0; a < 4; a++)
  {
    if (0 != devices_[a]) count += update(devices_[a]);
  }
  return count;
}


// __________________________________________________ PRIVATE MEMBER FUNCTIONS
/// Common code for constructors.
/// \param address device address (0..3)
/// \param options command byte bits SD, PD1, PD0
/// \param channelMask bit positions containing a 1 represent channels that
///   are to be read via update() / updateAll()
/// \param min minimum scaling value applied to value()
/// \param max maximum scaling value applied to value()
void ADS7828::init(uint8_t address, uint8_t options,
  uint8_t channelMask, uint16_t min, uint16_t max)
{
  this->address_ = address & 0x03;     // A1 A0 bits
  this->commandByte_ = options & 0x0C; // PD1 PD0 bits
  this->channelMask = channelMask;
  for (uint8_t ch = 0; ch < 8; ch++)
  {
    channels_[ch] = ADS7828Channel(this, ch, options, min, max);
  }
  this->devices_[address_] = this;
}


/// Request and receive data from most-recent A/D conversion from device.
/// \return 16-bit zero-padded word (12 data bits D11..D0)
uint16_t ADS7828::read()
{
  return read(address_);
}


// ___________________________________________ STATIC PRIVATE MEMBER FUNCTIONS
/// Request and receive data from most-recent A/D conversion from device.
/// \param address device address (0..3)
/// \return 16-bit zero-padded word (12 data bits D11..D0)
uint16_t ADS7828::read(uint8_t address)
{
  Wire.requestFrom(BASE_ADDRESS_ | (address & 0x03), 2);
  return word(Wire.read(), Wire.read());
}


/// Initiate communication with device.
/// \param address device address (0..3)
/// \param command command byte (0x00..0xFC)
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
uint8_t ADS7828::start(uint8_t address, uint8_t command)
{
  Wire.beginTransmission(BASE_ADDRESS_ | (address & 0x03));
  Wire.write((uint8_t) command);
  return Wire.endTransmission();
}


/// Initiate communication with device.
/// \param device pointer to device object
/// \return quantity of channels updated (0..8)
uint8_t ADS7828::update(ADS7828* device)
{
  if (0 == device) device = devices_[0];
  uint8_t ch, count = 0;
  for (ch = 0; ch < 8; ch++)
  {
    if (bitRead(device->channelMask, ch))
    {
      if (0 == update(device, ch)) count++;
    }
  }
  return count;
}


/// Initiate communication with device.
/// \param device pointer to device object
/// \param ch channel number (0..7)
/// \retval 0 success
/// \retval 1 length too long for buffer
/// \retval 2 address send, NACK received <b>(device not on bus)</b>
/// \retval 3 data send, NACK received
/// \retval 4 other twi error (lost bus arbitration, bus error, ...)
uint8_t ADS7828::update(ADS7828* device, uint8_t ch)
{
  if (0 == device) device = devices_[0];
  uint8_t status = device->start(ch);
  if (0 == status) device->channel(ch)->newSample(device->read());
  return status;
}


// _________________________________________________ STATIC PRIVATE ATTRIBTUES
ADS7828* ADS7828::devices_[] = {};
