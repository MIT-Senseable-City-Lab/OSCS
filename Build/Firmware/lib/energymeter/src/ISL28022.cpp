#include "ISL28022.h"

static uint16_t	ISL_ConfRegValue;

ISL28022::ISL28022(){
}

void ISL28022::begin(uint8_t I2Caddr, bool type)
{
  if(type == PV_SENSING)    //solar panel
  {
  Addr = I2Caddr;
  current_lsb_res_ = (ISL_SHUNT_FS_PGA_80) / (1000 * PV_SHUNT_RES_VAL * ISL_ADC_RES_15B);  //verified to be 0.000061
  cal_reg_value_ =  (int)(0.04096/(current_lsb_res_* PV_SHUNT_RES_VAL));    // verified to be 16777
  vbus_lsb_res_ = 0.004;          // from data-sheet pg17
  vshunt_lsb_res = 0.00001;       //Vshunt_LSB = 10uV      
  Wire.begin();
  WriteReg(Addr,ISL_CONF_RST,ISL28022_CONF_REG);    // reset cofig reg
  delay(50);
  ISL_ConfRegValue = ((uint16_t)(ISL_CONF_BRNG_32) | (uint16_t)(ISL_CONF_PG_80) | (uint16_t)(ISL_CONF_BADC_15B) | (uint16_t)(ISL_CONF_SADC_15B) | (uint16_t)(ISL_CONF_MODE_SB_CON));
  WriteReg(Addr, ISL_ConfRegValue, ISL28022_CONF_REG);
  delay(50);
  WriteReg(Addr, cal_reg_value_ , ISL28022_CALIB_REG);
  delay(50);
  }
  if(type == BATT_SENSING)    //battery
  {
  Addr = I2Caddr;
  current_lsb_res_ = (ISL_SHUNT_FS_PGA_80) / (1000 * BAT_SHUNT_RES_VAL * ISL_ADC_RES_15B);  //verified to be 0.000326
  cal_reg_value_ =  (int)(0.04096/(current_lsb_res_*BAT_SHUNT_RES_VAL));    // verified to be 16777
  vbus_lsb_res_ = 0.004;          // from data-sheet pg17
  vshunt_lsb_res = 0.00001;       //Vshunt_LSB = 10uV

  Wire.begin();
  WriteReg(Addr,ISL_CONF_RST,ISL28022_CONF_REG);
  delay(50);
  // expected ConfRegValue = 0x299F --> 60V range, Gain 2 80mV, 15-bit BADC, 15-bit SADC, shunt and bus continuous mode
  ISL_ConfRegValue = ((uint16_t)(ISL_CONF_BRNG_32) | (uint16_t)(ISL_CONF_PG_80) | (uint16_t)(ISL_CONF_BADC_15B) | (uint16_t)(ISL_CONF_SADC_15B) | (uint16_t)(ISL_CONF_MODE_SB_CON));
  WriteReg(Addr, ISL_ConfRegValue, ISL28022_CONF_REG);
  delay(50);
  WriteReg(Addr, cal_reg_value_, ISL28022_CALIB_REG);
  delay(50);
  }
}

float ISL28022::getCurrent_mA(){
  uint16_t retValue = 0;
  float realCurrent = 0;
  /** - 1. Read Register */
  retValue = ReadReg(Addr, ISL28022_CURR_REG);
   /** - 2. Converion of the data to floating point */
   realCurrent = calcCurrent(retValue);
   realCurrent = realCurrent * 1000;
   return realCurrent;
}

float ISL28022::getBusVoltage_V(){
  uint16_t data = 0;
  data = ReadReg(Addr, ISL28022_BUS_REG);
  float realVoltage = 0;
 
  if ((ISL_ConfRegValue & ISL_CONF_BRNG_16) != 0)
	{
		/* Shift value and conversion to float */
		data = (data >> 3);
		data &= 0x0FFF;
	}
	else if ((ISL_ConfRegValue & ISL_CONF_BRNG_32) != 0)
	{
		/* Shift value and conversion to float */
		data = (data >> 3);
		data &= 0x1FFF;
	}
	else if ((ISL_ConfRegValue & ISL_CONF_BRNG_60) != 0)
	{
		/* Shift value and conversion to float */
		data = (data >> 2);
		data &= 0x3FFF;
	}

  realVoltage = (float)data * vbus_lsb_res_;
 //ealVoltage = realVoltage + (10 * getCurrent_mA());
  return realVoltage;
}

float ISL28022::getShuntVoltage_mV()
{
	/* Temporary Variables */
	uint16_t retValue = 0;
	float realVoltage = 0;
	/** - 1. Read Register */
	retValue = ReadReg(Addr, ISL28022_SH_V_REG);
	/** - 2. Converion of the data to floating point */
	realVoltage = Calc_ShuntVoltage(retValue);
	/** - 3. Returned value */
  return realVoltage;
}

uint16_t ISL28022::ReadReg(uint8_t islAddr, uint16_t regAddr)
{
	uint8_t regValue[2] = {0};
	uint16_t retValue = 0;
  Wire.beginTransmission(islAddr);
  Wire.write(regAddr);
  Wire.endTransmission();
  //delay(300);
  // Request 2 bytes of data
  Wire.requestFrom(islAddr, 2);
  // Read 2 bytes of data
  // raw_shunt msb, raw_shunt lsb
  if (Wire.available() == 2)
  {
    regValue[0] = Wire.read();
    regValue[1] = Wire.read();
  }
	retValue |= (uint16_t)(regValue[0] << 8);
	retValue |= (uint16_t)regValue[1];
  return retValue; 
}

uint8_t ISL28022::WriteReg(uint8_t islAddr, uint16_t data, uint16_t regAddr)
{
  /* Temp buffer */
	uint8_t regValue[2] = {0};
	/** - 1. Check maximum address value */
	if (regAddr > ISL28022_AUX_REG)
	{
		return FALSE;
	}
  /** - 2. Prepare buffer to write */
	regValue[0] = (uint8_t)((data & 0xFF00) >> 8);
	regValue[1] = (uint8_t)(data & 0x00FF);
	/** - 3. Call the function for write the registers */
  /** - 4. Return the operation success */
  // Start I2C transmission
  Wire.beginTransmission(islAddr);
  // Select configuration register
  Wire.write(regAddr);
  Wire.write(regValue[0]);
  Wire.write(regValue[1]);
  return Wire.endTransmission();
}


float ISL28022::calcCurrent(uint16_t data)
{	
	/* Temporary Variables */
	float realCurrent = 0;
	/* Conversion to float and remove sign bit */
	realCurrent = (float)(data & 0x7FFF);
	/* Check Sign on bit 15 */
	if (data & 0x8000)
	{
    realCurrent -= (float)(ISL_CUR_BIT_15); 
	}
	/* Multiplying the decimal value by the LSB weight */
	realCurrent *= current_lsb_res_;
	/* Return Value */
  return realCurrent;
}

float ISL28022:: Calc_ShuntVoltage(uint16_t data)
{
	/* Temporary Variables */
	float realVoltage = 0;
	/* Conversion to float and remove sign bit */
	realVoltage = (float)(data & 0x0FFF);
	/* Check Sign on bit 12 */
	if (data & 0x1000)
	{
		/* Negative value */
		realVoltage *= (-1.0);
	}
	/* Multiplying the decimal value by the LSB weight */
	realVoltage *= vshunt_lsb_res;
	/* Return Value */
  return realVoltage;
}