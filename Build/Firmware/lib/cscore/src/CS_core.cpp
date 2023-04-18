
#include "CS_core.h"

#define PCA9554_REG_INP 0
#define PCA9554_REG_OUT 1
#define PCA9554_REG_POL 2
#define PCA9554_REG_CTRL 3
#define NORMAL 0
#define INVERTED 1
#define GPIO_A 0x20 //GPIO Extender A
#define GPIO_B 0x21 //GPIO Extender B

uint8_t pinNum2bitNum[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

CS_core *CS_core::_instance = nullptr;

/**
 * Constructor.
 */
CS_core::CS_core()
{
  // be sure not to call anything that requires hardware be initialized here, put those in begin()
}

/***************************************************************************
 *
 *  Writes 8-bits to the specified destination register
 *
 **************************************************************************/
static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

/***************************************************************************
 *
 * Reads 8-bits from the specified source register
 *
 **************************************************************************/
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg)
{
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)1);
  return Wire.read();
}

/***************************************************************************
 *
 * Begin method. This method must be called before using this library
 * either directly if the class is initializing the Wire library or by
 * calling this library's function begin(sda, scl) in which case that
 * function will call this one.
 *
 **************************************************************************/
void CS_core::begin(uint8_t hw_release)
{
  hw_version = hw_release;
  //pinMode(WKP, INPUT); //ACCEL PIN ON WKP
  switch (hw_version)
  {
  case V3:
  {
    Wire.begin();
    // Read out default values from the registers to the shadow variables.
    m_inp = readRegister(GPIO_A, PCA9554_REG_INP);
    m_out = readRegister(GPIO_A, PCA9554_REG_OUT);
    m_pol = readRegister(GPIO_A, PCA9554_REG_POL);
    m_ctrl = readRegister(GPIO_A, PCA9554_REG_CTRL);
    m_inp = readRegister(GPIO_B, PCA9554_REG_INP);
    m_out = readRegister(GPIO_B, PCA9554_REG_OUT);
    m_pol = readRegister(GPIO_B, PCA9554_REG_POL);
    m_ctrl = readRegister(GPIO_B, PCA9554_REG_CTRL);

    pinMode_ext(GPIO_A, 0, OUTPUT);   //Stat2 Overriden to avoid interrupt noise, they share logic line with interrupt WKP
    pinMode_ext(GPIO_A, 1, OUTPUT);   //Stat1 Overriden to avoid interrupt noise, they share logic line with interrupt WKP
    digitalWrite_ext(GPIO_A, 0, LOW); //Stat2
    digitalWrite_ext(GPIO_A, 1, LOW); //Stat1
    pinMode_ext(GPIO_A, 4, OUTPUT);   //EN_HEATER
    pinMode_ext(GPIO_A, 5, OUTPUT);   //EN_PW_OPC
    pinMode_ext(GPIO_B, 0, INPUT);  //Accelerometer interrupt 1 - mapped by the gpio ext to WKP
    pinMode_ext(GPIO_B, 1, INPUT);  //Accelerometer interrupt 2
    pinMode_ext(GPIO_B, 5, OUTPUT); //EN_3V
    pinMode_ext(GPIO_B, 6, OUTPUT); //EN_5V
    pinMode(A0, OUTPUT); //ONOFF_GPS
    pinMode(D5, OUTPUT); //EN_GPS
    pinMode(B0, INPUT);  //SYSTEM-ON_GPS
    pinMode(A7, INPUT);  //NOISE SENSOR
    delay(100);
    
    // Default configuration - everything OFF
    digitalWrite_ext(GPIO_B, 5, LOW);   //ENABLE3V OFF
    digitalWrite_ext(GPIO_B, 6, HIGH);  //ENABLE5V OFF
    digitalWrite_ext(GPIO_A, 4, LOW);   //EN_HEATER OFF
    digitalWrite_ext(GPIO_A, 5, LOW);   //EN_PW_OPC OFF
    digitalWrite(D6, LOW);              //EN_GPS OFF
    digitalWrite(A0, HIGH);             //ONOFF_GPS OFF
    break;
  }
  case V4:
  {
    pinMode(EN_3V, OUTPUT);
	  pinMode(EN_3V_GPS, OUTPUT);
    pinMode(EN_5V, OUTPUT);
    //pinMode(EN_OPC, OUTPUT);
    //pinMode(EN_HEATER, OUTPUT);

    //pinMode(INT_GPS, INPUT);

    //pinMode(MB_RST, OUTPUT);
    //pinMode(MB_INT, INPUT);
    pinMode(MB_AN, INPUT);
    pinMode(MB_CS, OUTPUT);
   // pinMode(MB_PWM, OUTPUT);

    pinMode(INT_ACC, INPUT);

    //pinMode(STAT1, INPUT_PULLUP);
    //pinMode(STAT2, INPUT_PULLUP);

    //default configuration - everything OFF
    digitalWrite(EN_3V, HIGH);
	  digitalWrite(EN_3V_GPS, HIGH);
    digitalWrite(EN_5V, HIGH);
    //digitalWrite(EN_OPC, LOW);
    //digitalWrite(EN_HEATER, LOW);

    //digitalWrite(MB_RST, LOW);
    digitalWrite(MB_CS, LOW);
    //digitalWrite(MB_PWM, LOW);
  }
  default:
    break;
  }
}

/***************************************************************************
 *
 * Sets the desired pin mode
 *
 **************************************************************************/
boolean CS_core::pinMode_ext(uint8_t address, uint8_t pin, uint8_t mode)
{
  // Make sure the pin number is OK
  if (pin >= sizeof pinNum2bitNum)
  {
    return false;
  }

  // Calculate the new control register value
  if (mode == OUTPUT)
  {
    m_ctrl &= ~pinNum2bitNum[pin];
  }
  else if (mode == INPUT)
  {
    m_ctrl |= pinNum2bitNum[pin];
  }
  else
  {
    return false;
  }

  writeRegister(address, PCA9554_REG_CTRL, m_ctrl);

  return true;
}

/***************************************************************************
 *
 * Write digital value to pin
 *
 **************************************************************************/
boolean CS_core::digitalWrite_ext(uint8_t address, uint8_t pin, boolean val)
{
  // Make sure pin number is OK
  if (pin >= sizeof pinNum2bitNum)
  {
    return false;
  }

  if (val == HIGH)
  {
    m_out |= pinNum2bitNum[pin];
  }
  else
  {
    m_out &= ~pinNum2bitNum[pin];
  }

  writeRegister(address, PCA9554_REG_OUT, m_out);
  return true;
}

/***************************************************************************
 *
 * Read digital value from pin.
 * Note, so far this function will fail silently if the pin parameter is
 * incorrectly specified.
 *
 **************************************************************************/
boolean CS_core::digitalRead_ext(uint8_t address, uint8_t pin)
{
  //Serial.print("Debug: "); Serial.println(readRegister(address, PCA9554_REG_INP));
  //Serial.print("Pin numbrt"); Serial.println(pinNum2bitNum[pin]);
  return (readRegister(address, PCA9554_REG_INP) & (pinNum2bitNum[pin]));
}

/***************************************************************************
 *
 * Sets the desired pin polarity. This can be used to invert inverse
 * hardware logic.
 *
 **************************************************************************/
boolean CS_core::pinPolarity(uint8_t address, uint8_t pin, uint8_t polarity)
{
  // Make sure pin number is OK
  if (pin >= sizeof pinNum2bitNum)
  {
    return false;
  }

  if (polarity == INVERTED)
  {
    m_pol |= pinNum2bitNum[pin];
  }
  else if (polarity == NORMAL)
  {
    m_pol &= ~pinNum2bitNum[pin];
  }
  else
  {
    return false;
  }

  writeRegister(address, PCA9554_REG_POL, m_pol);

  return true;
}

void CS_core::enableHEATER(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      digitalWrite_ext(GPIO_A, 4, HIGH);
      Log.info("Heater Enabled");
    }
    else
    {
      digitalWrite_ext(GPIO_A, 4, LOW);
      Log.info("Heater Disabled");
    }
    break;
  }
  case V4:
  {
    if (command)
    {
      //digitalWrite(EN_HEATER, HIGH);
      Log.info("Heater Enabled");
    }
    else
    {
      //digitalWrite(EN_HEATER, LOW);
      Log.info("Heater Disabled");
    }
    break;
  }
  default:
    break;
  }
}

void CS_core::enable3V3(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      digitalWrite_ext(GPIO_B, 5, HIGH);
      Log.info("3V3 Enabled");
    }
    else
    {
      digitalWrite_ext(GPIO_B, 5, LOW);
      Log.info("3V3 Disabled");
    }
    break;
  }
  case V4:
  {
    if (command)
    {
      digitalWrite(EN_3V, HIGH);
	    //digitalWrite(EN_3V_GPS, HIGH);
      Log.info("3V3 Enabled");
    }
    else
    {
      digitalWrite(EN_3V, LOW);
	    //digitalWrite(EN_3V_GPS, LOW);
      Log.info("3V3 Disabled");
    }
    break;
  }
  default:
    break;
  }
}

void CS_core::enable5V(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      digitalWrite_ext(GPIO_B, 6, LOW);
      Log.info("5V Enabled");
    }
    else
    {
      digitalWrite_ext(GPIO_B, 6, HIGH);
      Log.info("5V Disabled");
    }
    break;
  }
  case V4:
  {
    if (command)
    {
      digitalWrite(EN_5V, HIGH);
      Log.info("5V Enabled");
    }
    else
    {
      digitalWrite(EN_5V, LOW);
      Log.info("5V Disabled");
    }
    break;
  }
  default:
    break;
  }
}

void CS_core::enableOPC(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      digitalWrite_ext(GPIO_A, 5, HIGH);
      Log.info("OPC Enabled");
    }
    else
    {
      digitalWrite_ext(GPIO_A, 5, LOW);
      Log.info("OPC Disabled");
    }
    break;
  }
  case V4:
  {
    if (command)
    {
      //digitalWrite(EN_OPC, HIGH);
      Log.info("OPC Enabled");
    }
    else
    {
      //digitalWrite(EN_OPC, LOW);
      Log.info("OPC Disabled");
    }
    break;
  }
  default:
    break;
  }
}

void CS_core::enableGPS(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      //digitalWrite(C4, HIGH);
      Log.info("GPS Enabled");
    }
    else
    {
      //digitalWrite(C4, LOW);
      Log.info("GPS Disabled");
    }
  }
  case V4:
    break;
  default:
    break;
  }
}

void CS_core::activateGPS(bool command)
{
  switch (hw_version)
  {
  case V3:
    if (command)
    {
      //digitalWrite(A0, LOW);
      Log.info("GPS Activated");
    }
    else
    {
      //digitalWrite(A0, HIGH);
      Log.info("GPS Deactivated");
    }
    break;
  case V4:
    break;
  default:
    break;
  }
}

uint8_t CS_core::getGPSstatus()
{
  switch (hw_version)
  {
  case V4:
    //return digitalRead(INT_GPS);
	return 1;	// Check code for use
    break;
  
  default:
    return 250;
    break;
  }
}

uint8_t CS_core::isCharging()
{
  switch (hw_version)
  {
  case V4:
    //return !digitalRead(STAT1);
	return 1;
    break;
  default:
    return 250;
    break;
  }
}

uint8_t CS_core::isCharged()
{
  switch (hw_version)
  {
  case V4:
    //return !digitalRead(STAT2);
	return 1;
    break;  
  default:
    return 250;
    break;
  }
}

void CS_core::enableALL(bool command)
{
  switch (hw_version)
  {
  case V3:
  {
    if (command)
    {
      digitalWrite_ext(GPIO_B, 5, HIGH); //ENABLE3V ON
      delay(500);
      digitalWrite_ext(GPIO_B, 6, LOW);  //ENABLE5V ON
      delay(500);
      digitalWrite_ext(GPIO_A, 4, HIGH); //EN_HEATER ON
      delay(500);
      digitalWrite_ext(GPIO_A, 5, HIGH); //EN_PW_OPC ON
      delay(500);
      //digitalWrite(C4, HIGH);            //EN_GPS ON
      delay(500);
      digitalWrite(A0, LOW);             //ONOFF_GPS ON
      delay(500);
      Log.info("Enable ALL");
    }
    else
    {
      digitalWrite_ext(GPIO_B, 5, LOW);  //ENABLE3V OFF
      digitalWrite_ext(GPIO_B, 6, HIGH); //ENABLE5V OFF
      digitalWrite_ext(GPIO_A, 4, LOW);  //EN_HEATER OFF
      digitalWrite_ext(GPIO_A, 5, LOW);  //EN_PW_OPC OFF
      //digitalWrite(C4, LOW);             //EN_GPS OFF
      digitalWrite(A0, HIGH);            //ONOFF_GPS OFF
      Log.info("Deactivate ALL");
    }
    break;
  }
  case V4:
    if (command)
    {
      digitalWrite(EN_3V, HIGH);
	    delay(500);
	    digitalWrite(EN_3V_GPS, HIGH);
      delay(500);
      digitalWrite(EN_5V, HIGH);
      delay(500);
      //digitalWrite(EN_OPC, HIGH);
      //delay(500);
      //digitalWrite(EN_HEATER, HIGH);
      //delay(500); 
      Log.info("Enable ALL");
    }
    else
    {
      digitalWrite(EN_3V, LOW);
      digitalWrite(EN_3V_GPS, LOW);
      digitalWrite(EN_5V, LOW);
      //digitalWrite(EN_OPC, LOW);
      //digitalWrite(EN_HEATER, LOW);
      Log.info("Deactivate ALL");
    }
    break;
  default:
    break;
  }
}


