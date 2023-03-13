#include "Particle.h"
#include "motion_service.h"

LSM6DS3 myIMU(I2C_MODE,0x6A);

MotionService *MotionService::_instance = nullptr;
int MotionService::inactivity_counter = 0;


void MotionService::timer_fnc(){
   inactivity_counter++;
   Serial.print("Inactivity counter: ");Serial.println(inactivity_counter);
}


Timer inactivity_timer(1000, MotionService::timer_fnc);

MotionService::MotionService() {
}

int MotionService::start()
{
    Particle.variable("OVERRIDE_AS", OVVERRIDE_AUTOSLEEP);
    myIMU.begin();
    //Error accumulation variable
	uint8_t errorAccumulator = 0;
	uint8_t dataToWrite = 0;  //Temporary variable
	//Setup the accelerometer******************************
	dataToWrite = 0; //Start Fresh!
	dataToWrite |= LSM6DS3_ACC_GYRO_BW_XL_200Hz;
	dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
	dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_208Hz;
	// //Now, write the patched together data
	errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, dataToWrite);

    if (HW_VERSION == V4)
    {
    #define LSM6DSL_ACC_GYRO_TAP_CFG                0x58
    #define LSM6DSL_ACC_GYRO_TAP_THS_6D             0x59
    #define LSM6DSL_ACC_GYRO_INT_DUR2               0x5A
    #define LSM6DSL_ACC_GYRO_WAKE_UP_THS            0x5B
    #define LSM6DSL_ACC_GYRO_MD1_CFG                0x5E

    errorAccumulator += myIMU.writeRegister(LSM6DSL_ACC_GYRO_TAP_CFG, 0x8E);
    errorAccumulator += myIMU.writeRegister(LSM6DSL_ACC_GYRO_TAP_THS_6D, 0x03);
    errorAccumulator += myIMU.writeRegister(LSM6DSL_ACC_GYRO_INT_DUR2, 0x7F);
    errorAccumulator += myIMU.writeRegister(LSM6DSL_ACC_GYRO_WAKE_UP_THS, 0x80);
    errorAccumulator += myIMU.writeRegister(LSM6DSL_ACC_GYRO_MD1_CFG, 0x48);
    }
    else
    {      
    //Set the ODR bit
	errorAccumulator += myIMU.readRegister(&dataToWrite, LSM6DS3_ACC_GYRO_CTRL4_C);
	dataToWrite &= ~((uint8_t)LSM6DS3_ACC_GYRO_BW_SCAL_ODR_ENABLED);
    //Set the duration for Inactivity detection this field is set to 0010b, corresponding to 4.92 s (= 2 * 512 / ODR_XL). After this period of time has elapsed, the accelerometer ODR is internally set to 12.5 Hz.
    errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_WAKE_UP_DUR, 0x02 );
    //errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_WAKE_UP_DUR, 0x0F ); // sleep time to max, circa 38s
    //WAKE_UP_THS register is set to 000010b, therefore the Activity/Inactivity threshold is 62.5 mg (= 2 * FS_XL / 26).
    errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_WAKE_UP_THS, 0x42 );
    //Set INT1 to be triggered only by activity events
    errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_MD1_CFG, 0x20 ); /// INT configure for tap
    //errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_MD1_CFG, 0x80 ); /// INT1 configured for activity/inactivity
    }
	

    if(errorAccumulator)
	{
		Serial.println("Problem configuring the device.");
	}
	else
	{
		Serial.println("Device O.K.");
	}
    attachInterrupt(WKP, MotionService::resetInactivityCounter, FALLING);
    inactivity_timer.start();	
    attachInterrupt(WKP, resetInactivityCounter, RISING);
    Serial.println("timer started");
    motionservice_started = true;
    return true;
}

int MotionService::stop()
{
    inactivity_timer.dispose();	
    motionservice_started = false;
    return 1;
}

int MotionService::waitOnEvent()
{
    // check accelerometer for events and add it to event queque
    // if no motion events for x minutes go to sleep
}

void MotionService::loop()
{
    if(inactivity_counter > INACTIVITY_TIME){
        Serial.println("motion service loop inactivity timer");
        resetInactivityCounter();
        if(AUTOSLEEP && !OVVERRIDE_AUTOSLEEP){
        Serial.println("It's time to get some sleep");
        delay(100);
        CitySleep::instance().stop();
        }
    }
}

void MotionService::resetInactivityCounter(){
    inactivity_counter = 0;
}

int MotionService::getInactivityCounter()
{
    return inactivity_counter;
}

void MotionService::setOverrideAutosleep(bool override)
{
    if(override)
        OVVERRIDE_AUTOSLEEP = TRUE;
    else
        OVVERRIDE_AUTOSLEEP = FALSE;
}

void MotionService::testAccelerometer()
{
  Serial.print("\nAccelerometer:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatAccelX(), 4);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatAccelY(), 4);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatAccelZ(), 4);
  Serial.print("\nGyroscope:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatGyroX(), 4);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatGyroY(), 4);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatGyroZ(), 4);
  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees C = ");
  Serial.println(myIMU.readTempC(), 4);
  Serial.print(" Degrees F = ");
  Serial.println(myIMU.readTempF(), 4);
  delay(1000);
}