/************************************************************************************
 *  Copyright (c) February 2019, version 1.0     Paul van Haastrecht
 *
 *  Version 1.1.1 Paul van Haastrecht / March 2020
 *  - Fixed compile errors and warnings.
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch will connect to an SPS30, SCD30 and BME280
 *  for getting data and display the available data.
 *
 *  =========================  Hardware connections =================================
 *  /////////////////////////////////////////////////////////////////////////////////
 *  ## UART UART UART UART UART UART UART UART UART UART UART UART UART UART UART  ##
 *  /////////////////////////////////////////////////////////////////////////////////
 *
 *  Sucessfully test has been performed on an ESP32:
 *
 *  Using serial port1, setting the RX-pin(25) and TX-pin(26)
 *  Different setup can be configured in the sketch.
 *
 *  SPS30 pin     ESP32
 *  1 VCC -------- VUSB
 *  2 RX  -------- TX  pin 26
 *  3 TX  -------- RX  pin 25
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 *
 *  Also successfully tested on Serial2 (default pins TX:17, RX: 16)
 *  NO level shifter is needed as the SPS30 is TTL 5V and LVTTL 3.3V compatible
 *  ..........................................................
 *  Successfully tested on ATMEGA2560
 *  Used SerialPort2. No need to set/change RX or TX pin
 *  SPS30 pin     ATMEGA
 *  1 VCC -------- 5V
 *  2 RX  -------- TX2  pin 16
 *  3 TX  -------- RX2  pin 17
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 *
 *  Also tested on SerialPort1 and Serialport3 successfully
 *  .........................................................
 *  Failed testing on UNO
 *  Had to use softserial as there is not a separate serialport. But as the SPS30
 *  is only working on 115K the connection failed all the time with CRC errors.
 *
 *  Not tested ESP8266
 *  As the power is only 3V3 (the SPS30 needs 5V)and one has to use softserial,
 *  I have not tested this.
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
 *
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
 *  ..........................................................
 *  Successfully tested on ATMEGA2560
 *
 *  SPS30 pin     ATMEGA
 *  1 VCC -------- 5V
 *  2 SDA -------- SDA
 *  3 SCL -------- SCL
 *  4 Select ----- GND  (select I2c)
 *  5 GND -------- GND
 *
 *  ..........................................................
 *  Successfully tested on UNO R3
 *
 *  SPS30 pin     UNO
 *  1 VCC -------- 5V
 *  2 SDA -------- A4
 *  3 SCL -------- A5
 *  4 Select ----- GND  (select I2c)
 *  5 GND -------- GND
 *
 *  When UNO-board is detected the UART code is excluded as that does not work on
 *  UNO and will save memory. Also some buffers reduced and the call to
 *  GetErrDescription() is removed to allow enough memory.
 *  ..........................................................
 *  Successfully tested on ESP8266
 *
 *  SPS30 pin     External     ESP8266
 *  1 VCC -------- 5V
 *  2 SDA -----------------------SDA
 *  3 SCL -----------------------SCL
 *  4 Select ----- GND --------- GND  (select I2c)
 *  5 GND -------- GND --------- GND
 *
 *  The pull-up resistors should be to 3V3 from the ESP8266.
 *
 *  ===============  BME280 sensor =========================
 *  BME280
 *  VCC  ------ VCC  (3V3 or 5V depending on board)
 *  GND  ------ GND
 *  SCK  ------ SCL
 *  SDI  ------ SDA
 *
 *  ===============  SCD30 sensor =========================
 *
 *  SCD30
 *  1 VDD  --------- VCC ( 3V3 or 5V)
 *  2 GND  --------- GND
 *  3 TX/SCL ------- SCL
 *  4 RX/SDA ------- SDA
 *  5 RDY    ------- NOT CONNECTED
 *  6 PWM    ------- NOT CONNECTED
 *  7 SEL    ------- NOT CONNECTED
 *
 * !!!!! ONLY NEEDED for an ESP32.
 * Given that SCD30 is using clock stretching the driver has been modified to deal with that.
 * A SoftWire library is included in the SCD30 (February 2019) for the ESP32
 *
 * In case a sketch is to interact with both SPS30 and SCD30 on a ESP32 platform running BOTH over I2C,
 * you MUST use the SoftWire I2C that is part of SCD30.
 * For that you need to un-comment in sps30.h the line:  //#define SOFTI2C_ESP32 1
 *
 * In case you do not plan to use the I2C code for the SPS30 you could instead in sps30.h
 * comment out the line : #define INCLUDE_I2C   1. (also saves memory)
 *
 * In case of ESP32, given the SoftWire library, you also have to make a change in SparkfunBME280.h.
 *  Line 39 states:    #include <Wire.h>
 *  Comment that line out : //#include <Wire.h>
 *
 *  NOW include :
   #if defined(ARDUINO_ARCH_ESP32)
   #include <SoftWire/SoftWire.h>
   #else
   #include <Wire.h>
   #endif
 *  ================================= PARAMETERS =====================================
 *
 *  From line 180 there are configuration parameters for the program
 *
 *  ================================== SOFTWARE ======================================
 *  Sparkfun ESP32
 *
 *    Make sure :
 *      - To select the Sparkfun ESP32 thing board before compiling
 *      - The serial monitor is NOT active (will cause upload errors)
 *      - Press GPIO 0 switch during connecting after compile to start upload to the board
 *
 *  !!!! SCD30 library      : https://github.com/paulvha/scd30
 *  Sparkfun BME280 library : https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
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
 *  NO support, delivered as is, have fun, good luck !!
 */

#include "paulvha_SCD30.h"
#include "sps30.h"
#include "SparkFunBME280.h"

/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid options:
 *   I2C_COMMS              use I2C communication
 *   SOFTWARE_SERIAL        Arduino variants (NOTE)
 *   SERIALPORT             ONLY IF there is NO monitor attached
 *   SERIALPORT1            Arduino MEGA2560, Sparkfun ESP32 Thing : MUST define new pins as defaults are used for flash memory
 *   SERIALPORT2            Arduino MEGA2560 and ESP32
 *   SERIALPORT3            Arduino MEGA2560 only for now

 * NOTE: Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on any device.
 */
/////////////////////////////////////////////////////////////
#define SP30_COMMS SERIALPORT1

/////////////////////////////////////////////////////////////
/* define RX and TX pin for softserial and Serial1 on ESP32
 * can be set to zero if not applicable / needed           */
/////////////////////////////////////////////////////////////
#define TX_PIN 26
#define RX_PIN 25

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
//////////////////////////////////////////////////////////////
#define DEBUG 0

///////////////////////////////////////////////////////////////
//                          BME280                           //
///////////////////////////////////////////////////////////////
/* define the BME280 address.
 * Use if address jumper is closed (SDO - GND) : 0x76.*/
#define I2CADDR 0x77

/* Define reading in Fahrenheit or Celsius
 *  1 = Celsius
 *  0 = Fahrenheit */
#define TEMP_TYPE 1

/* define whether hight Meters or Foot
 *  1 = Meters
 *  0 = Foot */
#define BME_HIGHT 1

//////////////////////////////////////////////////////////////
////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
//////////////////////////////////////////////////////////////

// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
void serialTrigger(char * mess);
void ErrtoMess(char *mess, uint8_t r);
void Errorloop(char *mess, uint8_t r);
void GetDeviceInfo();
bool read_all();

// create constructors
SPS30 sps30;
SCD30 airSensor;
BME280 mySensor; //Global sensor object

// status
bool detect_BME280 = false;
bool SCD30_detected = false;

void setup() {
  char buf[30];

  Serial.begin(115200);

  serialTrigger((char *) "SPS30-Example8: Basic reading + SCD30 + BME280. press <enter> to start");

  Serial.println(F("Trying to connect"));

  // set driver debug level
  sps30.EnableDebugging(DEBUG);

  // set pins to use for softserial and Serial1 on ESP32
  if (TX_PIN != 0 && RX_PIN != 0) sps30.SetSerialPin(RX_PIN,TX_PIN);

  // Begin communication channel;
  if (! sps30.begin(SP30_COMMS))
    Errorloop((char *) "could not initialize communication channel.", 0);

  // check for SPS30 connection
  if (! sps30.probe()) Errorloop((char *) "could not probe / connect with SPS30.", 0);
  else  Serial.println(F("Detected SPS30."));

  // reset SPS30 connection
  if (! sps30.reset()) Errorloop((char *) "could not reset.", 0);

  // read device info
  GetDeviceInfo();

  // set SCD30
  airSensor.setDebug(DEBUG);

  // This will init the wire, but NOT start reading
  if (! airSensor.begin(Wire,false))
    Serial.println(F("cound not start SCD30"));
  else
  {
    Serial.println(F("Detected SCD30"));

    if (airSensor.getSerialNumber(buf))
    {
      Serial.print(F("\tSerial number : "));
      Serial.println(buf);
    }
    else
      Serial.println(F("could not read serial number"));
  }

  // set BME280 I2C address.
  mySensor.setI2CAddress(I2CADDR);

  if (mySensor.beginI2C() == false) // Begin communication over I2C
    Serial.println(F("The BME280 did not respond. Please check wiring."));
  else
  {
    detect_BME280 = true;
    Serial.println(F("Detected BME280"));
  }

  // This will cause readings to occur every two seconds
  if (airSensor.begin() == false)
    Serial.println(F("cound not start SCD30"));
  else
    SCD30_detected = true;

  // start measurement
  if (sps30.start())  Serial.println(F("Measurement started"));
  else Errorloop((char *) "Could NOT start measurement", 0);

  serialTrigger((char *) "Hit <enter> to continue reading");

  if (SP30_COMMS == I2C_COMMS) {
    if (sps30.I2C_expect() == 4)
      Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
}

void loop() {
  read_all();
  delay(3000);
}

/**
 * @brief : read and display device info
 */
void GetDeviceInfo()
{
  char buf[32];
  uint8_t ret;
  SPS30_version v;

  //try to read serial number
  ret = sps30.GetSerialNumber(buf, 32);
  if (ret == SPS30_ERR_OK) {
    Serial.print(F("\tSerial number : "));
    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess((char *) "could not get serial number", ret);

  // try to get product name
  ret = sps30.GetProductName(buf, 32);
  if (ret == SPS30_ERR_OK)  {
    Serial.print(F("\tProduct name  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess((char *) "could not get product name.", ret);

  // try to get version info
  ret = sps30.GetVersion(&v);
  if (ret != SPS30_ERR_OK) {
    Serial.println(F("Can not read version info"));
    return;
  }

  Serial.print(F("Firmware level: "));  Serial.print(v.major);
  Serial.print("."); Serial.println(v.minor);

  if (SP30_COMMS != I2C_COMMS) {
    Serial.print(F("Hardware level: ")); Serial.println(v.HW_version);

    Serial.print(F("SHDLC protocol: ")); Serial.print(v.SHDLC_major);
    Serial.print("."); Serial.println(v.SHDLC_minor);
  }

  Serial.print(F("Library level : "));  Serial.print(v.DRV_major);
  Serial.print(".");  Serial.println(v.DRV_minor);
}

/**
 * @brief : read and display all values
 */
bool read_all()
{
  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  do {
    ret = sps30.GetValues(&val);

    // data might not have been ready / retry max 3 times
    if (ret == SPS30_ERR_DATALENGTH){

        if (error_cnt++ > 3) {
          ErrtoMess((char *) "Error during reading values: ",ret);
          return(false);
        }
        delay(1000);
    }

    // if other error
    else if(ret != SPS30_ERR_OK) {
      ErrtoMess((char *) "Error during reading values: ",ret);
      return(false);
    }

  } while (ret != SPS30_ERR_OK);

  // only print header first time
  if (header) {

    Serial.print(F("==================================== SPS30 ====================================="));
    if(SCD30_detected) Serial.print(F(" ========= SCD30 =========="));

    if(detect_BME280) Serial.print(F(" =============== BME280 ==============="));
    Serial.print(F("\n-------------Mass -----------    ------------- Number --------------   -Average-"));
    if(SCD30_detected) Serial.print(F(" CO2   Humidity Temperature"));
    if(detect_BME280) Serial.print(F(" Pressure Humidity Altitude Temperature"));

    Serial.print(F("\n     Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
    if(SCD30_detected) {
      Serial.print(F("    [ppm]    [%]      "));

      if (TEMP_TYPE) Serial.print(F("[*C]"));
      else Serial.print(F("[*F]"));
    }

    if(detect_BME280) {
      Serial.print(F("     [hPa]    [%]      "));

      if (BME_HIGHT) Serial.print(F("Meter\t"));
      else Serial.print(F("Foot\t"));

      if (TEMP_TYPE) Serial.print(F("[*C]"));
      else Serial.print(F("[*F]"));
    }
    Serial.println(F("\nP1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPrtSize\n"));

    header = false;
  }

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

  if(SCD30_detected) {
      Serial.print(F("\t  "));
      Serial.print(airSensor.getCO2());
      Serial.print(F("\t "));
      Serial.print(airSensor.getHumidity(), 1);
      Serial.print(F("\t  "));

      if (TEMP_TYPE)  Serial.print(airSensor.getTemperature(), 2);
      else Serial.print(airSensor.getTemperatureF(), 2);
  }

  if(detect_BME280) {
      Serial.print(F("\t    "));
      Serial.print(mySensor.readFloatPressure()/100, 0);
      Serial.print(F("\t"));
      Serial.print(mySensor.readFloatHumidity(), 1);
      Serial.print(F("\t"));

      if (BME_HIGHT) Serial.print(mySensor.readFloatAltitudeMeters(), 1);
      else Serial.print(mySensor.readFloatAltitudeFeet(), 1);
      Serial.print(F("\t"));

      if (TEMP_TYPE)  Serial.print(mySensor.readTempC(), 2);
      else Serial.print(mySensor.readTempF(), 2);
  }
  Serial.print(F("\n"));

  return(true);
}

/**
 *  @brief : continued loop after fatal error
 *  @param mess : message to display
 *  @param r : error code
 *
 *  if r is zero, it will only display the message
 */
void Errorloop(char *mess, uint8_t r)
{
  if (r) ErrtoMess(mess, r);
  else Serial.println(mess);
  Serial.println(F("Program on hold"));
  for(;;) delay(100000);
}

/**
 *  @brief : display error message
 *  @param mess : message to display
 *  @param r : error code
 *
 */
void ErrtoMess(char *mess, uint8_t r)
{
  char buf[80];

  Serial.print(mess);

  sps30.GetErrDescription(r, buf, 80);
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
