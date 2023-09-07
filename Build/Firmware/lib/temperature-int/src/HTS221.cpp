/* HTS221 library by speirano
/*
* HTS221.cpp
*
* Created: 02/01/2015 20:50:20
*  Author: speirano
*/

#include "HTS221.h"

//#include "application.h"


#define HTS221_ADDRESS     0x5F 


//Define a few of the registers that we will be accessing on the HTS221
#define WHO_AM_I           0x0F
#define WHO_AM_I_RETURN    0xBC //This read-only register contains the device identifier, set to BCh

#define AVERAGE_REG        0x10	// To configure humidity/temperature average.
#define AVERAGE_DEFAULT    0x1B

/*
* [7] PD: power down control
* (0: power-down mode; 1: active mode)
*
* [6:3] Reserved
*
* [2] BDU: block data update
* (0: continuous update; 1: output registers not updated until MSB and LSB reading)
The BDU bit is used to inhibit the output register update between the reading of the upper
and lower register parts. In default mode (BDU = ?0?), the lower and upper register parts are
updated continuously. If it is not certain whether the read will be faster than output data rate,
it is recommended to set the BDU bit to ?1?. In this way, after the reading of the lower (upper)
register part, the content of that output register is not updated until the upper (lower) part is
read also.
*
* [1:0] ODR1, ODR0: output data rate selection (see table 17)
*/
#define CTRL_REG1          0x20
#define POWER_UP           0x80
#define BDU_SET            0x4
#define ODR0_SET           0x1   // setting sensor reading period 1Hz

#define CTRL_REG2          0x21
#define CTRL_REG3          0x22
#define REG_DEFAULT        0x00

#define STATUS_REG         0x27
#define TEMPERATURE_READY  0x1
#define HUMIDITY_READY     0x2

#define HUMIDITY_L_REG     0x28
#define HUMIDITY_H_REG     0x29
#define TEMP_L_REG         0x2A
#define TEMP_H_REG         0x2B
/*
* calibration registry should be read for temperature and humidity calculation.
* Before the first calculation of temperature and humidity,
* the master reads out the calibration coefficients.
* will do at init phase
*/
#define CALIB_START        0x30
#define CALIB_END	       0x3F
/*
#define CALIB_T0_DEGC_X8   0x32
#define CALIB_T1_DEGC_X8   0x33
#define CALIB_T1_T0_MSB    0x35
#define CALIB_T0_OUT_L     0x3C
#define CALIB_T0_OUT_H     0x3D
#define CALIB_T1_OUT_L     0x3E
#define CALIB_T1_OUT_H     0x3F
*/




static inline bool humidityReady(uint8_t data) {
   return (data & 0x02);
}
static inline bool temperatureReady(uint8_t data) {
   return (data & 0x01);
}


HTS221::HTS221(void)
{    _address=HTS221_ADDRESS;
   _temperature = 0.0;
   _humidity    = 0.0;
}


bool HTS221::begin(void)
{
   Wire.begin();
   uint8_t data;

   data = readRegister(_address, WHO_AM_I);
   if (data == WHO_AM_I_RETURN){
       if (activate()){
           storeCalibration();
           return true;
       }
   }

   return false;
}

bool HTS221::storeCalibration(void)
{
   uint8_t data;
   uint16_t tmp;

   for (int reg=CALIB_START; reg<=CALIB_END; reg++) {
       if ((reg!=CALIB_START+8) && (reg!=CALIB_START+9) && (reg!=CALIB_START+4)) {

           data = readRegister(HTS221_ADDRESS, reg);

           switch (reg) {
           case CALIB_START:
               _h0_rH = data;
               break;
           case CALIB_START+1:
           _h1_rH = data;
           break;
           case CALIB_START+2:
           _T0_degC = data;
           break;
           case CALIB_START+3:
           _T1_degC = data;
           break;

           case CALIB_START+5:
           tmp = _T0_degC;
           _T0_degC = (data&0x3)<<8;
           _T0_degC |= tmp;

           tmp = _T1_degC;
           _T1_degC = ((data&0xC)>>2)<<8;
           _T1_degC |= tmp;
           break;
           case CALIB_START+6:
           _H0_T0 = data;
           break;
           case CALIB_START+7:
           _H0_T0 |= data<<8;
           break;
           case CALIB_START+0xA:
           _H1_T0 = data;
           break;
           case CALIB_START+0xB:
           _H1_T0 |= data <<8;
           break;
           case CALIB_START+0xC:
           _T0_OUT = data;
           break;
           case CALIB_START+0xD:
           _T0_OUT |= data << 8;
           break;
           case CALIB_START+0xE:
           _T1_OUT = data;
           break;
           case CALIB_START+0xF:
           _T1_OUT |= data << 8;
           break;


           case CALIB_START+8:
           case CALIB_START+9:
           case CALIB_START+4:
           //DO NOTHING
           break;

           // to cover any possible error
           default:
               return false;
           } /* switch */
       } /* if */
   }  /* for */
   return true;
}


bool HTS221::activate(void)
{
   uint8_t data;

   data = readRegister(_address, CTRL_REG1);
   data |= POWER_UP;
   data |= ODR0_SET;
   writeRegister(_address, CTRL_REG1, data);

   return true;
}


bool HTS221::deactivate(void)
{
   uint8_t data;

   data = readRegister(_address, CTRL_REG1);
   data &= ~POWER_UP;
   writeRegister(_address, CTRL_REG1, data);
   return true;
}


bool HTS221::bduActivate(void)
{
   uint8_t data;

   data = readRegister(_address, CTRL_REG1);
   data |= BDU_SET;
   writeRegister(_address, CTRL_REG1, data);

   return true;
}


bool HTS221::bduDeactivate(void)
{
   uint8_t data;

   data = readRegister(_address, CTRL_REG1);
   data &= ~BDU_SET;
   writeRegister(_address, CTRL_REG1, data);
   return true;
}


const double HTS221::readHumidity(void)
{
   uint8_t data   = 0;
   uint16_t h_out = 0;
   double h_temp  = 0.0;
   double hum     = 0.0;

   data = readRegister(_address, STATUS_REG);

   if (data & HUMIDITY_READY) {
       data = readRegister(_address, HUMIDITY_H_REG);
       h_out = data << 8;  // MSB
       data = readRegister(_address, HUMIDITY_L_REG);
       h_out |= data;      // LSB

       // Decode Humidity
       hum = ((int16_t)(_h1_rH) - (int16_t)(_h0_rH))/2.0;  // remove x2 multiple

       // Calculate humidity in decimal of grade centigrades i.e. 15.0 = 150.
       h_temp = (double)(((int16_t)h_out - (int16_t)_H0_T0) * hum) /
          (double)((int16_t)_H1_T0 - (int16_t)_H0_T0);
       hum    = (double)((int16_t)_h0_rH) / 2.0; // remove x2 multiple
       _humidity = (hum + h_temp); // provide signed % measurement unit
   }
   return _humidity;
}



const double HTS221::readTemperature(void)
{
   uint8_t data   = 0;
   uint16_t t_out = 0;
   double t_temp  = 0.0;
   double deg     = 0.0;

   data = readRegister(_address, STATUS_REG);

   if (data & TEMPERATURE_READY) {

       data= readRegister(_address, TEMP_H_REG);
       t_out = data  << 8; // MSB
       data = readRegister(_address, TEMP_L_REG);
       t_out |= data;      // LSB

       // Decode Temperature
       deg    = (double)((int16_t)(_T1_degC) - (int16_t)(_T0_degC))/8.0; // remove x8 multiple

       // Calculate Temperature in decimal of grade centigrades i.e. 15.0 = 150.
       t_temp = (double)(((int16_t)t_out - (int16_t)_T0_OUT) * deg) /
          (double)((int16_t)_T1_OUT - (int16_t)_T0_OUT);
       deg    = (double)((int16_t)_T0_degC) / 8.0;     // remove x8 multiple
       _temperature = deg + t_temp;   // provide signed celsius measurement unit
   }

   return _temperature;
}



// Read a single byte from addressToRead and return it as a byte
byte HTS221::readRegister(byte slaveAddress, byte regToRead)
{
   Wire.beginTransmission(slaveAddress);
   Wire.write(regToRead);
   Wire.endTransmission(false); //endTransmission but keep the connection active

   Wire.requestFrom(slaveAddress, 1); //Ask for 1 byte, once done, bus is released by default

   while(!Wire.available()) ; //Wait for the data to come back
   return Wire.read(); //Return this one byte
}

// Writes a single byte (dataToWrite) into regToWrite
bool HTS221::writeRegister(byte slaveAddress, byte regToWrite, byte dataToWrite)
{
   Wire.beginTransmission(slaveAddress);

   if (!Wire.write(regToWrite)) {
       return false;
   }
   if (!Wire.write(dataToWrite)) {
       return false;
   }

   uint8_t errorNo = Wire.endTransmission(); //Stop transmitting
   return (errorNo == 0);
}
