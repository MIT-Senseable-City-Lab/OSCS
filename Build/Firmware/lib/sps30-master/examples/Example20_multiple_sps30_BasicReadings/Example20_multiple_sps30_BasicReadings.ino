/************************************************************************************
 *  Copyright (c) August 2020, version 1.0     Paul van Haastrecht
 *
 *  Version 1.0 Paul van Haastrecht August 2020
 *  - initial version
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch is be able to read multiple SPS30 at the same time. They can be
 *  connected to Seria11, Serial2, Serial3, Wire Wire1 etc..
 *
 *  You can now connect up to 5 (!) different SPS30 devices to a single Arduino DUE or
 *  4 on a singleMega2560 or ESP32. It has been tested with 3 SPS30 running at the same time.
 *  WHY 3... I "only" have 3 SPS30 to test :-)
 *
 *  For now I have only tested on a MEGA2560,DUE and ESP32, but it could / should work on any
 *  other board with enough Serial or Wire lines.
 *
 *  =========================  Hardware connections =================================
 *
 *  //////////////////////////////////////////////////////////////////////////////////
 *  ## I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C ##
 *  //////////////////////////////////////////////////////////////////////////////////
 *  NOTE 1:
 *  Depending on the Wire / I2C buffer size we might not be able to read all the values.
 *  The buffer size needed is at least 60 while on many boards this is set to 32. The driver
 *  will determine the buffer size and if less than 64 only the MASS values are returned.
 *  You can manually edit the Wire.h of your board to increase (if you memory is larg enough)
 *  One can check the expected number of bytes with the I2C_expect() call as in this example
 *  see detail document.
 *
 *  NOTE 2:
 *  As documented in the datasheet, make sure to use external 10K pull-up resistor on
 *  both the SDA and SCL lines. Otherwise the communication with the sensor will fail random.
 *  ..........................................................
 *  Successfully tested on ESP32
 *
 *  SPS30 pin     ESP32
 *  1 VCC -------- VUSB
 *  2 SDA -------- SDA (pin 21)
 *  3 SCL -------- SCL (pin 22)
 *  4 Select ----- GND (select I2c)
 *  5 GND -------- GND
 *
 *  The pull-up resistors should be to 3V3
 *  The usage of Wire1 is possible and explained in the included SPS30.odt
 *  ..........................................................
 *  Successfully tested on ATMEGA2560, Due
 *
 *  SPS30 pin     ATMEGA / DUE
 *  1 VCC -------- 5V
 *  2 SDA -------- SDA  either SDA0 (wire) or on a DUE SDA1 (Wire1) *1
 *  3 SCL -------- SCL  either SCL0 (wire) or on a DUE SCL1 (Wire1) *1
 *  4 Select ----- GND  (select I2c)
 *  5 GND -------- GND
 *
 *  *1:Just be aware the standard Wire on the DUE is a challenge due to the on-board pull-up
 *  resistors of 1K5. (look it up on Internet... it is NOT good)
 *  It worked well with external 5K6 (or 10K) pull-up resistors on Wire1. See included
 *  SPS30.odt chapter 9.
 *
 *  /////////////////////////////////////////////////////////////////////////////////
 *  ## UART UART UART UART UART UART UART UART UART UART UART UART UART UART UART  ##
 *  /////////////////////////////////////////////////////////////////////////////////
 *
 *  Sucessfully test has been performed on an ESP32:
 *
 *  Using Serial1, setting the RX-pin(25) and TX-pin(26)
 *
 *  SPS30 pin     ESP32
 *  1 VCC -------- VUSB
 *  2 RX  -------- TX  pin 26 !! NEEDS A CHANGE : see LINE 147
 *  3 TX  -------- RX  pin 25 !! NEEDS A CHANGE : see LINE 147
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 *
 *  Also successfully tested on Serial2 (default pins TX:17, RX: 16)
 *  NO level shifter is needed as the SPS30 is TTL 5V and LVTTL 3.3V compatible
 *  ..........................................................
 *  Successfully tested on ATMEGA2560 and DUE
 *  Used Serial2. No need to set/change RX or TX pin
 *  SPS30 pin     ATMEGA
 *  1 VCC -------- 5V
 *  2 RX  -------- TX2  pin 16
 *  3 TX  -------- RX2  pin 17
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 *
 *  Also tested on Serial1 and Serial3 successfully
 *
 *  ================================= PARAMETERS =====================================
 *
 *  From line 116 there are configuration parameters for the program
 *
 *  ================================== SOFTWARE ======================================
 *  Sparkfun ESP32
 *
 *    Make sure :
 *      - To select the Sparkfun ESP32 thing board before compiling
 *      - The serial monitor is NOT active (will cause upload errors)
 *      - Press GPIO 0 switch during connecting after compile to start upload to the board
 *
 *  ================================ Disclaimer ======================================
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  ===================================================================================
 *
 *  NO support, delivered as is, have fun, good luck (BUT maybe if you ask... maybe some advice) !!
 */

#include "sps30.h"

/*//////////////////////////////////////////////////////////
// define communication channel to use for SPS30          */
// !!!  comment SPS30_COMMSx line out if not in use !!!
/*
// SPS30_COMMS1, SPS30_COMMS2, SPS30_COMMS3
// You can select any Serial (e.g. Serial1, Serial2 or Serial3)
//
// SPS30_COMMS4 SPS30_COMMS5
// You can select I2C-line (e.g. Wire, Wire1 or Wire2)
///////////////////////////////////////////////////////////*/
#define SPS30_COMMS1 Serial1   // serial only
#define SPS30_COMMS2 Serial2   // serial only
#define SPS30_COMMS3 Serial3   // serial only
#define SPS30_COMMS4 Wire      // I2C only
#define SPS30_COMMS5 Wire1     // I2C only

/////////////////////////////////////////////////////////////
/* define driver debug for the communication channel
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
 //////////////////////////////////////////////////////////////
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0

///////////////////////////////////////////////////////////////
///// NORMALLY NO CHANGES BEYOND THIS POINT NEEDED  ///////////
///////////////////////////////////////////////////////////////
// BUT IF YOU NEED TO SET DIFFERENT PINS FOR SERIAL PORT     //
// AS IS THE CASE FOR ESP32 SERIAL1                          //
// CHANGE LINE : SPS30_COMMSx.begin(115200);                 //
//                                                           //
// FOR ESP32 SERIAL1 TO : E.G. RX-PIN 25, TX-PIN 26          //
// SPS30_COMMSx.begin(115200, SERIAL_8N1, 25, 26);           //
//                                                           //
///////////////////////////////////////////////////////////////

// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
void serialTrigger(char * mess);
void ErrtoMess(uint8_t d, char * mess, uint8_t r);
void ErrorStop();
void Errorloop(uint8_t d, char * mess, uint8_t r);
void GetDeviceInfo(uint8_t d, bool i2c);
bool read_all(uint8_t d);
void setupSPS30(uint8_t d);

// create constructors
// we create them all (even if not used) to keep the code simple and readable)
SPS30 sps301;
SPS30 sps302;
SPS30 sps303;
SPS30 sps304;
SPS30 sps305;

// save serial numbers
char serial[5][32];

void setup() {

  Serial.begin(115200);

  serialTrigger((char *)"SPS30-Example20: Basic reading with multiple SPS30. press <enter> to start");

  Serial.println(F("Trying to connect."));

//*************** setup SPS30 - 1 Serial **********************************
#ifdef SPS30_COMMS1
  setupSPS30(1);
#endif
//*************** setup SPS30 - 2 Serial **********************************
#ifdef SPS30_COMMS2
  setupSPS30(2);
#endif
//*************** setup SPS30 - 3 Serial **********************************
#ifdef SPS30_COMMS3
  setupSPS30(3);
#endif
//*************** setup SPS30 - 4  WIRE ***********************************
#ifdef SPS30_COMMS4
  setupSPS30(4);
#endif
//*************** setup SPS30 - 5  WIRE ***********************************
#ifdef SPS30_COMMS5
  setupSPS30(5);
#endif

  serialTrigger((char *) "Hit <enter> to continue reading.");
}

void loop() {

  struct sps_values val;
  uint8_t ret;

#ifdef SPS30_COMMS1
  read_all(1);
#endif

#ifdef SPS30_COMMS2
  read_all(2);
#endif

#ifdef SPS30_COMMS3
  read_all(3);
#endif

#ifdef SPS30_COMMS4
 read_all(4);
#endif

#ifdef SPS30_COMMS5
  read_all(5);
#endif

  Serial.println();

  delay(3000);
}

/**
 * @brief : read and display all values
 * @param dev : constructor
 * @param d : device number
 */
bool read_all(uint8_t d)
{

  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  do {
  if (d == 1) ret = sps301.GetValues(&val);
  else if (d == 2) ret = sps302.GetValues(&val);
  else if (d == 3) ret = sps303.GetValues(&val);
  else if (d == 4) ret = sps304.GetValues(&val);
  else if (d == 5) ret = sps305.GetValues(&val);
  else {
    Serial.println("Invalid device to read");
    return false;
  }

    // data might not have been ready
    if (ret == SPS30_ERR_DATALENGTH){

        if (error_cnt++ > 3) {
          ErrtoMess(d,(char *) "Error during reading values: ",ret);
        }
        delay(1000);
    }

    // if other error
    else if(ret != SPS30_ERR_OK) {
      ErrtoMess(d,(char *) "Error during reading values: ",ret);
    }

  } while (ret != SPS30_ERR_OK);

  // only print header first time
  if (header) {
    Serial.println(F("SPS30 -------------Mass -----------    ------------- Number --------------   -Average-   --Serialnumber--"));
    Serial.println(F("Dev       Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
    Serial.println(F("      P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPartSize\n"));
    header = false;
  }

  Serial.print(d);
  Serial.print(F("    "));

  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(val.NumPM0);
  Serial.print(F("\t"));
  Serial.print(val.NumPM1);
  Serial.print(F("\t"));
  Serial.print(val.NumPM2);
  Serial.print(F("\t"));
  Serial.print(val.NumPM4);
  Serial.print(F("\t"));
  Serial.print(val.NumPM10);
  Serial.print(F("\t"));
  Serial.print(val.PartSize);
  Serial.print(F("\t"));
  Serial.print(serial[d - 1]);
  Serial.print(F("\n"));

  return(true);
}

/*
 * Initialize SPS30
 * @param d   : device number
 */
void setupSPS30(uint8_t d)
{
  bool ret;

#ifdef SPS30_COMMS1
  if (d == 1)
  {
    // set driver debug level
    sps301.EnableDebugging(DEBUG1);

    // start channel
    SPS30_COMMS1.begin(115200);

    //SPS30_COMMS1.begin(115200, SERIAL_8N1, 25, 26);    // ESP32 in case of Serial1

    // Initialize SPS30 library
    if (!sps301.begin(&SPS30_COMMS1)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 1"));
      ErrorStop();
    }

    // check for SPS30 connection
    if (! sps301.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 1"));
      ErrorStop();
    }

    Serial.println(F("\nDetected SPS30 - 1"));

    // reset SPS30 connection
    if (! sps301.reset()){
      Serial.println(F("could not reset."));
      ErrorStop();
    }

    // read device info
    GetDeviceInfo(d, false);

    if (! sps301.start()) {
      Serial.println(F("\nCould NOT start measurement SPS30"));
      ErrorStop();
    }

    Serial.println(F("Measurement started SPS30 - 1"));
  }
#endif

#ifdef SPS30_COMMS2
  if (d == 2)
  {
    // set driver debug level
    sps302.EnableDebugging(DEBUG2);

    // start channel
    SPS30_COMMS2.begin(115200);

    // Initialize SPS30 library
    if (!sps302.begin(&SPS30_COMMS2)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 2"));
      ErrorStop();
    }

    // check for SPS30 connection
    if (! sps302.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 2"));
      ErrorStop();
    }

    Serial.println(F("\nDetected SPS30 - 2"));

    // reset SPS30 connection
    if (! sps302.reset()){
      Serial.println(F("could not reset."));
      ErrorStop();
    }

    // read device info
    GetDeviceInfo(d, false);

    if (! sps302.start()) {
      Serial.println(F("\nCould NOT start measurement SPS30"));
      ErrorStop();
    }

    Serial.println(F("Measurement started SPS30 - 2"));
  }
#endif

#ifdef SPS30_COMMS3
  if (d == 3)
  {
    // set driver debug level
    sps303.EnableDebugging(DEBUG3);

    // start channel
    SPS30_COMMS3.begin(115200);

    // Initialize SPS30 library
    if (!sps303.begin(&SPS30_COMMS3)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 3"));
      ErrorStop();
    }

    // check for SPS30 connection
    if (! sps303.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 3"));
      ErrorStop();
    }

    Serial.println(F("\nDetected SPS30 - 3"));

    // reset SPS30 connection
    if (! sps303.reset()){
      Serial.println(F("could not reset."));
      ErrorStop();
    }

    // read device info
    GetDeviceInfo(d, false);

    if (! sps303.start()) {
      Serial.println(F("\nCould NOT start measurement SPS30"));
      ErrorStop();
    }

    Serial.println(F("Measurement started SPS30 - 3"));
  }
#endif

#ifdef SPS30_COMMS4
  if (d == 4)
  {
    // set driver debug level
    sps304.EnableDebugging(DEBUG4);

    // start channel
    SPS30_COMMS4.begin();

    // Initialize SPS30 library
    if (!sps304.begin(&SPS30_COMMS4)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 4"));
      ErrorStop();
    }

    // check for SPS30 connection
    if (! sps304.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 4"));
      ErrorStop();
    }

    Serial.println(F("\nDetected SPS30 - 4"));

    // reset SPS30 connection
    if (! sps304.reset()){
      Serial.println(F("could not reset."));
      ErrorStop();
    }

    // read device info
    GetDeviceInfo(d, true);

    if (! sps304.start()) {
      Serial.println(F("\nCould NOT start measurement SPS30"));
      ErrorStop();
    }

    Serial.println(F("Measurement started SPS30 - 4"));

    if (sps304.I2C_expect() == 4)
        Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
#endif

#ifdef SPS30_COMMS5
  if (d == 5)
  {
    // set driver debug level
    sps305.EnableDebugging(DEBUG5);

    // start channel
    SPS30_COMMS5.begin();

    //SPS30_COMMS5.begin(23,18,100000); // SDA pin 23, SCL pin 18, 100kHz frequency // Wire1 on ESP32

    // Initialize SPS30 library
    if (!sps305.begin(&SPS30_COMMS5)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 5"));
      ErrorStop();
    }

    // check for SPS30 connection
    if (! sps305.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 5"));
      ErrorStop();
    }

    Serial.println(F("\nDetected SPS30 - 5"));

    // reset SPS30 connection
    if (! sps305.reset()){
      Serial.println(F("could not reset."));
      ErrorStop();
    }

    // read device info
    GetDeviceInfo(d, true);

    if (! sps305.start()) {
      Serial.println(F("\nCould NOT start measurement SPS30"));
      ErrorStop();
    }

    Serial.println(F("Measurement started SPS30 - 5"));

    if (sps305.I2C_expect() == 4)
        Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
#endif

  if (d <1 || d > 5) {
      Serial.print(F("Invalid device number for setup :"));
      Serial.println(d);
      ErrorStop();
  }
}

/**
 * @brief : read and display device info
 * @param d : device number
 * @param i2c : true if I2C / wire channel
 */
void GetDeviceInfo(uint8_t d, bool i2c)
{
  char buf[32];
  uint8_t ret;
  SPS30_version v;

  if (d == 1) ret = sps301.GetSerialNumber(buf, 32);
  else if (d == 2) ret = sps302.GetSerialNumber(buf, 32);
  else if (d == 3) ret = sps303.GetSerialNumber(buf, 32);
  else if (d == 4) ret = sps304.GetSerialNumber(buf, 32);
  else if (d == 5) ret = sps305.GetSerialNumber(buf, 32);
  else {
    Serial.println("Invalid device");
    return;
  }
  //try to read serial number
  if (ret == SPS30_ERR_OK) {
    Serial.print(F("Serial number : "));
    if(strlen(buf) > 0) {
      Serial.print(buf);
      strcpy(&serial[d -1][0], buf); // save serial number
    }
    else {
      Serial.print(F("not available"));
      serial[d-1][0] = 0;
    }
  }
  else
    ErrtoMess(d,(char *) "could not get serial number", ret);

  if (d == 1) ret = sps301.GetProductName(buf, 32);
  else if (d == 2) ret = sps302.GetProductName(buf, 32);
  else if (d == 3) ret = sps303.GetProductName(buf, 32);
  else if (d == 4) ret = sps304.GetProductName(buf, 32);
  else if (d == 5) ret = sps305.GetProductName(buf, 32);
  // try to get product name

  if (ret == SPS30_ERR_OK)  {
    Serial.print(F(" Product name  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess(d,(char *)"could not get product name.", ret);

   // try to get version info
  if (d == 1) ret = sps301.GetVersion(&v);
  else if (d == 2) ret = sps302.GetVersion(&v);
  else if (d == 3) ret = sps303.GetVersion(&v);
  else if (d == 4) ret = sps304.GetVersion(&v);
  else if (d == 5) ret = sps305.GetVersion(&v);

  if (ret != SPS30_ERR_OK) {
    Serial.println(F("Can not read version info"));
    return;
  }

  Serial.print(F("Firmware level: "));  Serial.print(v.major);
  Serial.print(".");  Serial.print(v.minor);

  Serial.print(F(" Hardware level: "));  Serial.print(v.HW_version);

  // if Wire channel : no SHDLC info
  if (i2c){
    Serial.println();
    return;
  }

  Serial.print(F(" SHDLC protocol: ")); Serial.print(v.SHDLC_major);
  Serial.print(".");  Serial.print(v.SHDLC_minor);

  Serial.print(F(" Library level : "));  Serial.print(v.DRV_major);
  Serial.print(".");  Serial.println(v.DRV_minor);
}

/**
 *  @brief : continued loop after fatal error
 *  @param d: device number
 *  @param mess : message to display
 *  @param r : error code
 *
 *  if r is zero, it will only display the message
 */
void Errorloop(uint8_t d, char * mess, uint8_t r)
{
  if (r) ErrtoMess(d, mess, r);
  else Serial.println(mess);
  Serial.println(F("Program on hold"));
  for(;;) delay(100000);
}

// just stop
void ErrorStop()
{
  Errorloop(1,(char *)"",0);
}

/**
 *  @brief : display error message
 *  @param d: device number
 *  @param mess : message to display
 *  @param r : error code
 *
 */
void ErrtoMess(uint8_t d, char * mess, uint8_t r)
{
  char buf[80];

  Serial.print(mess);
  if (d == 1) sps301.GetErrDescription(r, buf, 80);
  else if (d == 2) sps302.GetErrDescription(r, buf, 80);
  else if (d == 3) sps303.GetErrDescription(r, buf, 80);
  else if (d == 4) sps304.GetErrDescription(r, buf, 80);
  else if (d == 5) sps305.GetErrDescription(r, buf, 80);
  else    strcpy(buf, "unknown device number");

  Serial.println(buf);
}

/**
 * serialTrigger prints repeated message, then waits for enter
 * to come in from the serial port.
 */
void serialTrigger(char * mess)
{
  Serial.println();

  while (!Serial.available()) {
    Serial.println(mess);
    delay(2000);
  }

  while (Serial.available())
    Serial.read();
}
