/******************************************************************************
KXTJ3-1057.cpp
KXTJ3-1057 Arduino
Leonardo Bispo
May, 2020
https://github.com/ldab/KXTJ3-1057
Resources:
Uses Wire.h for i2c operation

Distributed as-is; no warranty is given.
******************************************************************************/

#include "kxtj3-1057.h"
#include "stdint.h"

#include "Wire.h"

//****************************************************************************//
//
//  Default construction is I2C mode, address 0x0E.
//
//****************************************************************************//
KXTJ3::KXTJ3( uint8_t inputArg = 0x0E )
{
  I2CAddress = inputArg;
}

kxtj3_status_t KXTJ3::begin( float SampleRate, uint8_t accRange )
{

	_DEBBUG("Configuring IMU");

	kxtj3_status_t returnError = IMU_SUCCESS;

  Wire.begin();
  
	// Start-up time, Figure 1: Typical StartUp Time - DataSheet
	#ifdef HIGH_RESOLUTION
		if			( SampleRate < 1)		delay(1300);
		else if ( SampleRate < 3)		delay(650);
		else if ( SampleRate < 6)		delay(350);
		else if ( SampleRate < 25)	delay(180);
		else												delay(45);
	#else
		delay(2);
	#endif

	//Check the ID register to determine if the operation was a success.
	uint8_t _whoAmI;
	
	readRegister(&_whoAmI, KXTJ3_WHO_AM_I);

	if( _whoAmI != 0x35 )
	{
		returnError = IMU_HW_ERROR;
	}

	accelSampleRate = SampleRate;
	accelRange 			= accRange;

	_DEBBUG("Apply settings");
	applySettings();

	return returnError;
}

//****************************************************************************//
//  ReadRegisterRegion
//
//  Parameters:
//    *outputPointer -- Pass &variable (base address of) to save read data to
//    offset -- register to read
//    length -- number of bytes to read
//****************************************************************************//
kxtj3_status_t KXTJ3::readRegisterRegion(uint8_t *outputPointer , uint8_t offset, uint8_t length)
{
	kxtj3_status_t returnError = IMU_SUCCESS;

	//define pointer that will point to the external space
	uint8_t i = 0;
	uint8_t c = 0;

  Wire.beginTransmission(I2CAddress);
  offset |= 0x80; //turn auto-increment bit on, bit 7 for I2C
  Wire.write(offset);
  if( Wire.endTransmission() != 0 )
  {
    returnError = IMU_HW_ERROR;
  }
  else  //OK, all worked, keep going
  {
    // request 6 bytes from slave device
    Wire.requestFrom(I2CAddress, length);
    while ( (Wire.available()) && (i < length))  // slave may send less than requested
    {
      c = Wire.read(); // receive a byte as character
      *outputPointer = c;
      outputPointer++;
      i++;
    }
  }

	return returnError;
}

//****************************************************************************//
//  ReadRegister
//
//  Parameters:
//    *outputPointer -- Pass &variable (address of) to save read data to
//    offset -- register to read
//****************************************************************************//
kxtj3_status_t KXTJ3::readRegister(uint8_t* outputPointer, uint8_t offset) {
	//Return value
	uint8_t result = 0;
	uint8_t numBytes = 1;
	kxtj3_status_t returnError = IMU_SUCCESS;

	Wire.beginTransmission(I2CAddress);
	Wire.write(offset);
	
	if( Wire.endTransmission() != 0 )
	{
		returnError = IMU_HW_ERROR;
	}

	Wire.requestFrom(I2CAddress, numBytes);

	while ( Wire.available() ) // slave may send less than requested
	{
		result = Wire.read(); 	 // receive a byte as a proper uint8_t
	}

	_DEBBUG("Read register 0x", offset, " = ", result);

	*outputPointer = result;
	return returnError;
}

//****************************************************************************//
//  readRegisterInt16
//
//  Parameters:
//    *outputPointer -- Pass &variable (base address of) to save read data to
//    offset -- register to read
//****************************************************************************//
kxtj3_status_t KXTJ3::readRegisterInt16( int16_t* outputPointer, uint8_t offset )
{
	//offset |= 0x80; //turn auto-increment bit on
	uint8_t myBuffer[2];
	kxtj3_status_t returnError = readRegisterRegion(myBuffer, offset, 2);  //Does memory transfer
	int16_t output = (int16_t)myBuffer[0] | int16_t(myBuffer[1] << 8);

	_DEBBUG("12 bit from 0x", offset, " = ", output);
	*outputPointer = output;
	return returnError;
}

//****************************************************************************//
//  writeRegister
//
//  Parameters:
//    offset -- register to write
//    dataToWrite -- 8 bit data to write to register
//****************************************************************************//
kxtj3_status_t KXTJ3::writeRegister(uint8_t offset, uint8_t dataToWrite)
{
	kxtj3_status_t returnError = IMU_SUCCESS;

  //Write the byte
  Wire.beginTransmission(I2CAddress);
  Wire.write(offset);
  Wire.write(dataToWrite);
  if( Wire.endTransmission() != 0 )
  {
    returnError = IMU_HW_ERROR;
  }

	return returnError;
}

// Read axis acceleration as Float
float KXTJ3::axisAccel( axis_t _axis)
{
	int16_t outRAW;
	uint8_t regToRead = 0;
	switch (_axis)
	{
		case 0:
			// X axis
			regToRead = KXTJ3_OUT_X_L;
			break;
		case 1:
			// Y axis
			regToRead = KXTJ3_OUT_Y_L;
			break;
		case 2:
			// Z axis
			regToRead = KXTJ3_OUT_Z_L;
			break;
	
		default:
			// Not valid axis return NAN
			return NAN;
			break;
	}

	readRegisterInt16( &outRAW, regToRead );

	float outFloat;

	switch( accelRange )
	{
		case 2:
		outFloat = (float)outRAW / 15987;
		break;
		case 4:
		outFloat = (float)outRAW / 7840;
		break;
		case 8:
		outFloat = (float)outRAW / 3883;
		break;
		case 16:
		outFloat = (float)outRAW / 1280;
		break;
		default:
		outFloat = 0;
		break;
	}

	return outFloat;

}

kxtj3_status_t	KXTJ3::standby( bool _en )
{
	uint8_t _ctrl;

	// "Backup" KXTJ3_CTRL_REG1
	readRegister(&_ctrl, KXTJ3_CTRL_REG1);
	
	if( _en )
		_ctrl &= 0x7E;
	else
		_ctrl |= (0x01 << 7);	// disable standby-mode -> Bit7 = 1 = operating mode

	return writeRegister(KXTJ3_CTRL_REG1, _ctrl);
}

//****************************************************************************//
//
//  Apply settings passed to .begin();
//
//****************************************************************************//
void KXTJ3::applySettings( void )
{
	uint8_t dataToWrite = 0;  //Temporary variable

	standby( true );
	
	//Build DATA_CTRL_REG

	//  Convert ODR
	if(accelSampleRate < 1)					dataToWrite |= 0x08;	// 0.781Hz
	else if(accelSampleRate < 2)		dataToWrite |= 0x09;	// 1.563Hz
	else if(accelSampleRate < 4)		dataToWrite |= 0x0A;	// 3.125Hz
	else if(accelSampleRate < 8)		dataToWrite |= 0x0B;	// 6.25Hz
	else if(accelSampleRate < 16)		dataToWrite |= 0x00;	// 12.5Hz
	else if(accelSampleRate < 30)		dataToWrite |= 0x01;	// 25Hz
	else if(accelSampleRate < 60)		dataToWrite |= 0x02;	// 50Hz
	else if(accelSampleRate < 150)	dataToWrite |= 0x03;	// 100Hz
	else if(accelSampleRate < 250)	dataToWrite |= 0x04;	// 200Hz
	else if(accelSampleRate < 450)	dataToWrite |= 0x05;	// 400Hz
	else if(accelSampleRate < 850)	dataToWrite |= 0x06;	// 800Hz
	else														dataToWrite	|= 0x07;	// 1600Hz

	//Now, write the patched together data
	_DEBBUG ("KXTJ3_DATA_CTRL_REG: 0x", dataToWrite);
	writeRegister(KXTJ3_DATA_CTRL_REG, dataToWrite);

	//Build CTRL_REG1
	
	// LOW power, 8-bit mode
	dataToWrite = 0x80;

	#ifdef HIGH_RESOLUTION
		dataToWrite = 0xC0;
	#endif

	//  Convert scaling
	switch(accelRange)
	{	
		default:
		case 2:
		dataToWrite |= (0x00 << 2);
		break;
		case 4:
		dataToWrite |= (0x02 << 2);
		break;
		case 8:
		dataToWrite |= (0x04 << 2);
		break;
		case 16:
		dataToWrite |= (0x01 << 2);
		break;
	}

	//Now, write the patched together data
	_DEBBUG ("KXTJ3_CTRL_REG1: 0x", dataToWrite);
	writeRegister(KXTJ3_CTRL_REG1, dataToWrite);

}

//****************************************************************************//
//  Configure interrupt, stop or move, threshold and duration
//	Durationsteps and maximum values depend on the ODR chosen.
//****************************************************************************//
kxtj3_status_t KXTJ3::intConf(uint16_t threshold, uint8_t moveDur, uint8_t naDur, bool polarity )
{
	// Note that to properly change the value of this register, the PC1 bit in CTRL_REG1must first be set to “0”.
	standby( true );

	kxtj3_status_t returnError = IMU_SUCCESS;

	// Build INT_CTRL_REG1

	uint8_t dataToWrite = 0x22;  		// Interrupt enabled, active LOW, non-latched
	
	if( polarity == HIGH )
		dataToWrite |= (0x01 << 5);		// Active HIGH

	_DEBBUG ("KXTJ3_INT_CTRL_REG1: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_INT_CTRL_REG1, dataToWrite);
	
	// WUFE – enables the Wake-Up (motion detect) function.

	uint8_t _reg1;

	returnError = readRegister(&_reg1, KXTJ3_CTRL_REG1);
	
	_reg1 |= (0x01 << 1);

	returnError = writeRegister(KXTJ3_CTRL_REG1, _reg1);

	// Build INT_CTRL_REG2

	dataToWrite = 0xBF;  // enable interrupt on all axis any direction - Unlatched

	_DEBBUG ("KXTJ3_INT_CTRL_REG1: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_INT_CTRL_REG2, dataToWrite);

	// Set WAKE-UP (motion detect) Threshold

	dataToWrite = (uint8_t)(threshold >> 4);

	_DEBBUG ("KXTJ3_WAKEUP_THRD_H: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_WAKEUP_THRD_H, dataToWrite);

	dataToWrite = (uint8_t)(threshold << 4);

	_DEBBUG ("KXTJ3_WAKEUP_THRD_L: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_WAKEUP_THRD_L, dataToWrite);

	// WAKEUP_COUNTER -> Sets the time motion must be present before a wake-up interrupt is set
	// WAKEUP_COUNTER (counts) = Wake-Up Delay Time (sec) x Wake-Up Function ODR(Hz)

	dataToWrite = moveDur;

	_DEBBUG ("KXTJ3_WAKEUP_COUNTER: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_WAKEUP_COUNTER, dataToWrite);

	// Non-Activity register sets the non-activity time required before another wake-up interrupt will be reported.
	// NA_COUNTER (counts) = Non-ActivityTime (sec) x Wake-Up Function ODR(Hz)

	dataToWrite = naDur;

	_DEBBUG ("KXTJ3_NA_COUNTER: 0x", dataToWrite);
	returnError = writeRegister(KXTJ3_NA_COUNTER, dataToWrite);

	// Set IMU to Operational mode
	returnError = standby( false );

	return returnError;
}
