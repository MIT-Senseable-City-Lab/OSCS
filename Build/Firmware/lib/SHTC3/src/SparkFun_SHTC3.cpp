/*

An Arduino library for the Sensirion SHTC3 humidity and temerature sensor

*/

#include "SparkFun_SHTC3.h"

SHTC3_Status_TypeDef SHTC3::sendCommand(SHTC3_Commands_TypeDef cmd)
{
	uint8_t res = 0;

	_wire->beginTransmission(SHTC3_ADDR_7BIT);
	_wire->write((((uint16_t)cmd) >> 8));
	_wire->write((((uint16_t)cmd) & 0x00FF));
	res = _wire->endTransmission();

	if (res)
	{
		return SHTC3_Status_Error;
	}

	return SHTC3_Status_Nominal;
}

SHTC3_Status_TypeDef SHTC3::sendCommand(SHTC3_MeasurementModes_TypeDef cmd)
{
	return sendCommand((SHTC3_Commands_TypeDef)cmd);
}

SHTC3_Status_TypeDef SHTC3::abortUpdate(SHTC3_Status_TypeDef status, const char *file, uint16_t line) // Sets the values to a known error state
{
	passRHcrc = false;
	passTcrc = false;
	passIDcrc = false;

	RH = 0x00;
	T = 0x00;
	ID = 0x00;

	return exitOp(status, file, line);
}

SHTC3_Status_TypeDef SHTC3::exitOp(SHTC3_Status_TypeDef status, const char *file, uint16_t line)
{
	lastStatus = status;
	SHTC3_exitOp_Callback(status, _inProcess, file, line);
	return status;
}

SHTC3_Status_TypeDef SHTC3::startProcess(void)
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	_inProcess = true;
	if (_isAsleep)
	{
		retval = wake();
	}
	if (retval == SHTC3_Status_Nominal)
	{
		_isAsleep = false;
	}
	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::endProcess(void)
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	_inProcess = false;
	if (_staySleeping)
	{
		retval = sleep();
	}
	if (retval == SHTC3_Status_Nominal)
	{
		_isAsleep = true;
	}
	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3::SHTC3()
{
	_mode = SHTC3_CMD_CSE_RHF_NPM; // My default pick

	_inProcess = false;	  // Definitely not doing anything when the object is intantiated
	_staySleeping = true; // Default to storing the sensor in low-power mode
	_isAsleep = true;	  // Assume the sensor is asleep to begin (there won't be any harm in waking it up if it is already awake)

	passRHcrc = false;
	passTcrc = false;
	passIDcrc = false;

	RH = 0x00;
	T = 0x00;
	ID = 0x00;
}

float SHTC3::toDegC()
{
	return SHTC3_raw2DegC(T);
}

float SHTC3::toDegF()
{
	return SHTC3_raw2DegF(T);
}

float SHTC3::toPercent()
{
	return SHTC3_raw2Percent(RH);
}

SHTC3_Status_TypeDef SHTC3::begin(TwoWire &wirePort)
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	;

	_wire = &wirePort; // Associate the I2C port
	// _clkFreq = speed;																	// Associate the desired speed

	// if(_clkFreq > SHTC3_MAX_CLOCK_FREQ)													// Throttle the clock speen
	// {
	// 	_clkFreq = SHTC3_MAX_CLOCK_FREQ;
	// }

	retval = startProcess(); // Multiple commands will go to the sensor before sleeping
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	retval = wake(); // Wake up the sensor
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	retval = checkID(); // Verify that the sensor is actually an SHTC3
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	retval = endProcess(); // We are about to return to user-land
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::softReset()
{
	return exitOp(sendCommand(SHTC3_CMD_SFT_RST), __FILE__, __LINE__);
	delayMicroseconds(500);
}

SHTC3_Status_TypeDef SHTC3::checkID()
{

	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	const uint8_t numBytesRequest = 3;
	uint8_t numBytesRx = 0x00;

	retval = startProcess(); // There will be several commands sent before returning control to the user
	if (retval != SHTC3_Status_Nominal)
	{
		return abortUpdate(retval, __FILE__, __LINE__);
	}

	retval = wake();
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	retval = sendCommand(SHTC3_CMD_READ_ID);
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	numBytesRx = _wire->requestFrom((uint8_t)SHTC3_ADDR_7BIT, numBytesRequest);
	if (numBytesRx != numBytesRequest)
	{
		exitOp(SHTC3_Status_Error, __FILE__, __LINE__);
		return SHTC3_Status_Error;
	}

	uint8_t IDhb = _wire->read();
	uint8_t IDlb = _wire->read();
	uint8_t IDcs = _wire->read();

	ID = (((uint16_t)IDhb << 8) | ((uint16_t)IDlb));

	passIDcrc = false;

	if (checkCRC(ID, IDcs) == SHTC3_Status_Nominal)
	{
		passIDcrc = true;
	}

	if ((ID & 0b0000100000111111) != 0b0000100000000111)		 // Checking the form of the ID
	{															 // Bits 11 and 5-0 must match
		return exitOp(SHTC3_Status_ID_Fail, __FILE__, __LINE__); // to identify an SHTC3
	}

	retval = endProcess(); // We are about to return to user-land
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::sleep(bool hold)
{
	_isAsleep = true; // It is fail-safe to always assume the sensor is asleep (whether or not the sleep command actually took effect)
	if (hold)
	{
		_staySleeping = true;
	}
	return exitOp(sendCommand(SHTC3_CMD_SLEEP), __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::wake(bool hold)
{
	SHTC3_Status_TypeDef retval = sendCommand(SHTC3_CMD_WAKE);
	if (retval == SHTC3_Status_Nominal)
	{
		_isAsleep = false; // Only when the wake command was sent successfully can you assume the sensor is awake
	}
	if (hold)
	{

		_staySleeping = false;
	}
	delayMicroseconds(240);
	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::setMode(SHTC3_MeasurementModes_TypeDef mode)
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	switch (mode) // Switch used to disallow unknown types
	{
	case SHTC3_CMD_CSE_RHF_NPM:
		_mode = SHTC3_CMD_CSE_RHF_NPM;
		break;
	case SHTC3_CMD_CSE_RHF_LPM:
		_mode = SHTC3_CMD_CSE_RHF_LPM;
		break;
	case SHTC3_CMD_CSE_TF_NPM:
		_mode = SHTC3_CMD_CSE_TF_NPM;
		break;
	case SHTC3_CMD_CSE_TF_LPM:
		_mode = SHTC3_CMD_CSE_TF_LPM;
		break;

	// case SHTC3_CMD_CSD_RHF_NPM : _mode = SHTC3_CMD_CSD_RHF_NPM; break;	// Currently clock stretching must be used because of the Arduino API's
	// case SHTC3_CMD_CSD_RHF_LPM : _mode = SHTC3_CMD_CSD_RHF_LPM; break;
	// case SHTC3_CMD_CSD_TF_NPM : _mode = SHTC3_CMD_CSD_TF_NPM; break;
	// case SHTC3_CMD_CSD_TF_LPM : _mode = SHTC3_CMD_CSD_TF_LPM; break;
	default:
		retval = SHTC3_Status_Error;
		break;
	}
	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_MeasurementModes_TypeDef SHTC3::getMode(void)
{
	return _mode;
}

SHTC3_Status_TypeDef SHTC3::update()
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;

	const uint8_t numBytesRequest = 6;
	uint8_t numBytesRx = 0;

	uint8_t RHhb = 0x00;
	uint8_t RHlb = 0x00;
	uint8_t RHcs = 0x00;

	uint8_t Thb = 0x00;
	uint8_t Tlb = 0x00;
	uint8_t Tcs = 0x00;

	retval = startProcess();
	if (retval != SHTC3_Status_Nominal)
	{
		return abortUpdate(retval, __FILE__, __LINE__);
	}

	// retval = wake();										// Always send wake() to be sure the sensor will respond
	// if(retval != SHTC3_Status_Nominal){ return abortUpdate(retval, __FILE__, __LINE__); }

	retval = sendCommand(_mode); // Send the appropriate command - Note: incorrect commands are excluded by the 'setMode' command and _mode is a protected variable
	if (retval != SHTC3_Status_Nominal)
	{
		return abortUpdate(retval, __FILE__, __LINE__);
	}

	switch (_mode) // Handle the two different ways of waiting for a measurement (polling or clock stretching)
	{
	case SHTC3_CMD_CSE_RHF_NPM:
	case SHTC3_CMD_CSE_RHF_LPM:
	case SHTC3_CMD_CSE_TF_NPM:
	case SHTC3_CMD_CSE_TF_LPM:	   // Address+read will yield an ACK and then clock stretching will occur
		numBytesRx = _wire->requestFrom((uint8_t)SHTC3_ADDR_7BIT, numBytesRequest);
		break;

	case SHTC3_CMD_CSD_RHF_NPM:
	case SHTC3_CMD_CSD_RHF_LPM:
	case SHTC3_CMD_CSD_TF_NPM:
	case SHTC3_CMD_CSD_TF_LPM: // These modes not yet supported (polling - need to figure out how to repeatedly send just address+read and look for ACK)
	default:
		return abortUpdate(SHTC3_Status_Error, __FILE__, __LINE__); // You really should never get to this code because setMode disallows non-approved values of _mode (type SHTC3_MeasurementModes_TypeDef)
		break;
	}

	// Now handle the received data
	if (numBytesRx != numBytesRequest)
	{
		return abortUpdate(SHTC3_Status_Error, __FILE__, __LINE__);
	} // Hopefully we got the right number of bytes

	switch (_mode) // Switch for the order of the returned results
	{
	case SHTC3_CMD_CSE_RHF_NPM:
	case SHTC3_CMD_CSE_RHF_LPM:
	case SHTC3_CMD_CSD_RHF_NPM:
	case SHTC3_CMD_CSD_RHF_LPM:
		// RH First
		RHhb = _wire->read();
		RHlb = _wire->read();
		RHcs = _wire->read();

		Thb = _wire->read();
		Tlb = _wire->read();
		Tcs = _wire->read();
		break;

	case SHTC3_CMD_CSE_TF_NPM:
	case SHTC3_CMD_CSE_TF_LPM:
	case SHTC3_CMD_CSD_TF_NPM:
	case SHTC3_CMD_CSD_TF_LPM:
		// T First
		Thb = _wire->read();
		Tlb = _wire->read();
		Tcs = _wire->read();

		RHhb = _wire->read();
		RHlb = _wire->read();
		RHcs = _wire->read();
		break;

	default:
		return abortUpdate(SHTC3_Status_Error, __FILE__, __LINE__); // Again, you should never experience this section of code
		break;
	}

	// Update values
	RH = ((uint16_t)RHhb << 8) | ((uint16_t)RHlb << 0);
	T = ((uint16_t)Thb << 8) | ((uint16_t)Tlb << 0);

	passRHcrc = false;
	passTcrc = false;

	if (checkCRC(RH, RHcs) == SHTC3_Status_Nominal)
	{
		passRHcrc = true;
	}
	if (checkCRC(T, Tcs) == SHTC3_Status_Nominal)
	{
		passTcrc = true;
	}

	// retval = sleep();
	// if(retval != SHTC3_Status_Nominal){ return exitOp(retval, __FILE__, __LINE__); }

	retval = endProcess(); // We are about to return to user-land
	if (retval != SHTC3_Status_Nominal)
	{
		return exitOp(retval, __FILE__, __LINE__);
	}

	return exitOp(retval, __FILE__, __LINE__);
}

SHTC3_Status_TypeDef SHTC3::checkCRC(uint16_t packet, uint8_t cs)
{
	uint8_t upper = packet >> 8;
	uint8_t lower = packet & 0x00FF;
	uint8_t data[2] = {upper, lower};
	uint8_t crc = 0xFF;
	uint8_t poly = 0x31;

	for (uint8_t indi = 0; indi < 2; indi++)
	{
		crc ^= data[indi];

		for (uint8_t indj = 0; indj < 8; indj++)
		{
			if (crc & 0x80)
			{
				crc = (uint8_t)((crc << 1) ^ poly);
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	if (cs ^ crc)
	{
		return exitOp(SHTC3_Status_CRC_Fail, __FILE__, __LINE__);
	}
	return exitOp(SHTC3_Status_Nominal, __FILE__, __LINE__);
}

float SHTC3_raw2DegC(uint16_t T)
{
	return -45 + 175 * ((float)T / 65535);
}

float SHTC3_raw2DegF(uint16_t T)
{
	return SHTC3_raw2DegC(T) * (9.0 / 5) + 32.0;
}

float SHTC3_raw2Percent(uint16_t RH)
{
	return 100 * ((float)RH / 65535);
}

void SHTC3_exitOp_Callback(SHTC3_Status_TypeDef status, bool inProcess, const char *file, uint16_t line) {} // Empty implementation. You can make your own implementation
