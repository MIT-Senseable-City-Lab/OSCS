/*
  Use a callback function to trace the program execution (and maybe find problems!)
  By: Owen Lyke
  SparkFun Electronics
  Date: August 24 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Example1_BasicReadings
  To connect the sensor to an Arduino:
  This library supports the sensor using the I2C protocol
  On Qwiic enabled boards simply connnect the sensor with a Qwiic cable and it is set to go
  On non-qwiic boards you will need to connect 4 wires between the sensor and the host board
  (Arduino pin) = (Display pin)
  SCL = SCL on display carrier
  SDA = SDA
  GND = GND
  3.3V = 3.3V
*/

#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3

#define WIRE_PORT   Wire    // This allows you to choose another Wire port (as long as your board supports more than one) 
#define WIRE_SPEED  800000  // 800 kHz is the fastest speed that worked on the Uno, but the sensor is rated for up to 1 MHz

SHTC3 mySHTC3;              // Declare an instance of the SHTC3 class

void errorDecoder(SHTC3_Status_TypeDef message); // Declaration of errorDecoder() so that we can use it in the callback

/*

  What is a callback? It is a way that a library can provide a place for the user to do something.
  What is the SHTC3 callback? The library was written with a function that is called (nearly) every time a function is exited.
  How is that useful? You can overwrite the function with your own definition that allows you to do things like debug or watch the program execute

  How do you do it?
  Make a function with this signature:

  void SHTC3_exitOp_Callback(SHTC3_Status_TypeDef status, bool inProcess, char * file, uint16_t line)

  Then write in it some code that might help you. 

  The example below helps us watch the code execute by displaying the line number, file, and status every time that a function finishes.
  This is useful for debugging, but can add a lot of overhead. For the fastest code execution just don't declare this function and it 
  should be optimized out by the compiler.
 */
#define DEBUG 1 // Change to 0 to (hopefully) optimize out the callback function
#if DEBUG
void SHTC3_exitOp_Callback(SHTC3_Status_TypeDef status, bool inProcess, char * file, uint16_t line)
{
    Serial.print("Exited with status '"); 
    errorDecoder(status);
    Serial.print("' in line ");
    Serial.print(line);
    Serial.print(" of file '");
    Serial.print(file);
    Serial.print("'. In process: ");
    Serial.print(inProcess);
    Serial.println();
}
#endif /* DEBUG */



void setup() {
  Serial.begin(115200);                                  // Begin Serial 
  while(Serial == false){};                              // Wait for the serial connection to start up
  Serial.println("SHTC3 Example 4 - Using Callback");  // Title
  Serial.println();

  Serial.print("Beginning sensor. Result = ");           // Most SHTC3 functions return a variable of the type "SHTC3_Status_TypeDef" to indicate the status of their execution 
  errorDecoder(mySHTC3.begin(WIRE_PORT));                     // Calling "begin()" with port and speed arguments allows you to reassign the interface to the sensor
  #if(WIRE_SPEED <= SHTC3_MAX_CLOCK_FREQ)                     // You can boost the speed of the I2C interface, but keep in mind that other I2C sensors may not support as high a speed!
    WIRE_PORT.setClock(WIRE_SPEED);                             
  #else
     WIRE_PORT.setClock(SHTC3_MAX_CLOCK_FREQ);
  #endif
  Serial.println();

  if(mySHTC3.passIDcrc)                                       // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
  {                                                           // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
    Serial.print("ID Passed Checksum. ");
    Serial.print("Device ID: 0b"); 
    Serial.print(mySHTC3.ID, BIN);                       // The 16-bit device ID can be accessed as a member variable of the object
  }
  else
  {
    Serial.print("ID Checksum Failed. ");
  }
  Serial.println("\n\n");
  
  Serial.println("Waiting for 5 seconds so you can read this info ^^^ \n");

  delay(5000);                                                // Give time to read the welcome message and device ID. 
}

void loop() {
  SHTC3_Status_TypeDef result = mySHTC3.update();             // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object
  printInfo();                                                // This function is used to print a nice little line of info to the serial port
  delay(1990);                                                 // Delay for the data rate you want - note that measurements take ~10 ms so the fastest data rate is 100 Hz (when no delay is used)
}



///////////////////////
// Utility Functions //
///////////////////////
void printInfo()
{
  if(mySHTC3.lastStatus == SHTC3_Status_Nominal)              // You can also assess the status of the last command by checking the ".lastStatus" member of the object
  {
    Serial.print("RH = "); 
    Serial.print(mySHTC3.toPercent());                   // "toPercent" returns the percent humidity as a floating point number
    Serial.print("% (checksum: "); 
    if(mySHTC3.passRHcrc)                                     // Like "passIDcrc" this is true when the RH value is valid from the sensor (but not necessarily up-to-date in terms of time)
    {
      Serial.print("pass");
    }
    else
    {
      Serial.print("fail");
    }
    Serial.print("), T = "); 
    Serial.print(mySHTC3.toDegF());                        // "toDegF" and "toDegC" return the temperature as a flaoting point number in deg F and deg C respectively 
    Serial.print(" deg F (checksum: "); 
    if(mySHTC3.passTcrc)                                        // Like "passIDcrc" this is true when the T value is valid from the sensor (but not necessarily up-to-date in terms of time)
    {
      Serial.print("pass");
    }
    else
    {
      Serial.print("fail");
    }
    Serial.println(")");
  }
  else
  {
    Serial.print("Update failed, error: "); 
    errorDecoder(mySHTC3.lastStatus);
    Serial.println();
  }
}

void errorDecoder(SHTC3_Status_TypeDef message)                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way
{
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.print("Nominal"); break;
    case SHTC3_Status_Error : Serial.print("Error"); break;
    case SHTC3_Status_CRC_Fail : Serial.print("CRC Fail"); break;
    default : Serial.print("Unknown return code"); break;
  }
}
