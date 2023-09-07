/******************************************************************************
kXTJ3-1057 Arduino
Leonardo Bispo
May, 2020
https://github.com/ldab/kxtj3-1057
Resources:
Uses Wire.h for i2c operation

Distributed as-is; no warranty is given.
******************************************************************************/

// Accelerometer provides different Power modes by changing output bit resolution
#define LOW_POWER
//#define HIGH_RESOLUTION

// Enable Serial debbug on Serial UART to see registers wrote
#define KXTJ3_DEBUG Serial

#include "kxtj3-1057.h"
#include "Wire.h"

float   sampleRate = 6.25;  // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
uint8_t accelRange = 2;     // Accelerometer range = 2, 4, 8, 16g

KXTJ3 myIMU(0x0E); // Address can be 0x0E or 0x0F

void setup()
{
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  delay(1000); //wait until serial is open...
  
  if( myIMU.begin(sampleRate, accelRange) != 0 )
  {
    Serial.print("Failed to initialize IMU.\n");
  }
  else
  {
    Serial.print("IMU initialized.\n");
  }
  
  // Detection threshold, movement duration and polarity
  myIMU.intConf(123, 1, 10, HIGH);

  uint8_t readData = 0;

  // Get the ID:
  myIMU.readRegister(&readData, KXTJ3_WHO_AM_I);
  Serial.print("Who am I? 0x");
  Serial.println(readData, HEX);

}


void loop()
{

  myIMU.standby( false );

  int16_t dataHighres = 0;

  if( myIMU.readRegisterInt16( &dataHighres, KXTJ3_OUT_X_L ) != 0 )

  Serial.print(" Acceleration X RAW = ");
  Serial.println(dataHighres);

  if( myIMU.readRegisterInt16( &dataHighres, KXTJ3_OUT_Z_L ) != 0 )

  Serial.print(" Acceleration Z RAW = ");
  Serial.println(dataHighres);

  // Read accelerometer data in mg as Float
  Serial.print(" Acceleration X float = ");
  Serial.println( myIMU.axisAccel( X ), 4);

  // Read accelerometer data in mg as Float
  Serial.print(" Acceleration Y float = ");
  Serial.println( myIMU.axisAccel( Y ), 4);

  // Read accelerometer data in mg as Float
  Serial.print(" Acceleration Z float = ");
  Serial.println( myIMU.axisAccel( Z ), 4);

  myIMU.standby( true );

  delay(1000);

}