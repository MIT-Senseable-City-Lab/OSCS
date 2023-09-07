/************************************************************************************
 *  Copyright (c) february 2019, version 1.0     Paul van Haastrecht
 *
  *
 *  Version 1.1.1 Paul van Haastrecht / March 2020
 *  - Fixed compile errors and warnings.
 *
 *  Version 1.1.2 Paul van Haastrecht / March 2020
 *  - added versions level to GetDeviceInfo()
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch will connect to an SPS30 for getting data and
 *  display the available data. It will calculate the average particle and compare to
 *  the reported typical to calculates the error margin. It will show the actual
 *  error margin as well as a running average based on the last INCLUDE_ERRORCALC definition
 *
 *  An answer on the typical size from Sensirion:
 *
 *  The typical particle size (TPS) is not a function of the other SPS30 outputs,
 *  but an independent output. It gives an indication on the average particle diameter
 *  in the sample aerosol. Such output correlates with the weighted average of the number
 *  concentration bins measured with a TSI 3330 optical particle sizer.
 *  Following this definition, lighter aerosols will have smaller TPS values than heavier aerosols.
 *  The reactiveness of this output increases with the particle statistics: a larger number of
 *  particles in the environment will generate more rapidly meaningful
 *  TPS values than a smaller number of particles (i.e., clean air).
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
 *  Successfully tested on ATMEGA2560 / Due
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
 *  As the power is only 3V3 (the SPS30 needs 5V)and one has to use softserial
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
 *  Successfully tested on ATMEGA2560 / Due
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
 *  When UNO-board is detected the UART code is excluded as that
 *  does not work on UNO and will save memory. Also some buffers
 *  reduced and the call to GetErrDescription() is removed to allow
 *  enough memory.
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
 *  ================================= PARAMETERS =====================================
 *
 *  From line 156 there are configuration parameters for the program
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
 *  NO support, delivered as is, have fun, good luck !!
 */

#include "sps30.h"

/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid options:
 *   I2C_COMMS              use I2C communication
 *   SOFTWARE_SERIAL        Arduino variants (NOTE)
 *   SERIALPORT             ONLY IF there is NO monitor attached
 *   SERIALPORT1            Arduino MEGA2560, Due. Sparkfun ESP32 Thing : MUST define new pins as defaults are used for flash memory)
 *   SERIALPORT2            Arduino MEGA2560, Due and ESP32
 *   SERIALPORT3            Arduino MEGA2560 and Due only for now

 * NOTE: Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on any device. */
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

/////////////////////////////////////////////////////////////
/* define the maximum of the last measured values to include
 * as part of an running error margin calculation
 */
 //////////////////////////////////////////////////////////////
#define INCLUDE_ERRORCALC 10

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// create constructor
SPS30 sps30;

// for typical/average error margin
struct comp {
  float typical;
  float avg;
};

struct comp compta[INCLUDE_ERRORCALC+1];

// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
void serialTrigger(char * mess);
void ErrtoMess(char *mess, uint8_t r);
void Errorloop(char *mess, uint8_t r);
void GetDeviceInfo();
bool read_all();
double calc_avg(struct sps_values v);
double error_margin(struct sps_values v, double avg);
void print_aligned(double val, signed char width, unsigned char prec);

void setup() {

  Serial.begin(115200);

  serialTrigger((char *) "SPS30-Example9: Read values, calculate average size and error margin.  Press <enter> to start");

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

  // start measurement
  if (sps30.start()) Serial.println(F("Measurement started"));
  else Errorloop((char *) "Could NOT start measurement", 0);

  serialTrigger((char *) "Hit <enter> to continue reading");

  if (SP30_COMMS == I2C_COMMS) {
    if (sps30.I2C_expect() == 4)
      Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
}

void loop() {
  read_all();
  delay(5000);
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
    Serial.print(F("Serial number : "));
    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess((char *) "could not get serial number", ret);

  // try to get product name
  ret = sps30.GetProductName(buf, 32);
  if (ret == SPS30_ERR_OK)  {
    Serial.print(F("Product name  : "));

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
 * @brief will print nice aligned columns
 *
 * @param val : value to print
 * @param width : total width of value including decimal point
 * @param prec : precision after the decimal point
 */
void print_aligned(double val, signed char width, unsigned char prec)
{
  char out[15];

  dtostrf(val, width, prec, out);
  Serial.print(out);
  Serial.print(F("\t  "));
}

/**
 * @brief : read and display all values
 *
 */
bool read_all()
{
  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  do {

    ret = sps30.GetValues(&val);

    // data might not have been ready or value is 0 (can happen at start)
    if (ret == SPS30_ERR_DATALENGTH || val.MassPM1 == 0 ) {

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
    Serial.println(F("----------------------------Mass -----------------------------    -------------------------------- Number ---------------------------------      -------Partsize --------         ----- Error Margin -----"));
    Serial.println(F("                     Concentration [μg/m3]                                                 Concentration [#/cm3]                                           [μm]                          % "));
    Serial.print(F(" PM1.0             PM2.5           PM4.0           PM10             PM0.5           PM1.0           PM2.5           PM4.0           PM10          Typical         Average         Actual    last "));
    Serial.print(INCLUDE_ERRORCALC);
    Serial.println(F(" samples\n"));
    header = false;

    // often seen the first reading to be "out of bounds". so we skip it
    return(true);
  }

  print_aligned((double) val.MassPM1, 8, 5);
  print_aligned((double) val.MassPM2, 8, 5);
  print_aligned((double) val.MassPM4, 8, 5);
  print_aligned((double) val.MassPM10, 8, 5);
  print_aligned((double) val.NumPM0, 9, 5);
  print_aligned((double) val.NumPM1, 9, 5);
  print_aligned((double) val.NumPM2, 9, 5);
  print_aligned((double) val.NumPM4, 9, 5);
  print_aligned((double) val.NumPM10, 9, 5);
  print_aligned((double) val.PartSize, 7, 5);

  double tmp = calc_avg(val);
  print_aligned(tmp, 7, 5);

  // calculated actual Error Margin
  print_aligned((((double)val.PartSize - tmp) / (double)val.PartSize) * 100, 6, 2);

  tmp = error_margin(val, tmp)* 100;
  print_aligned(tmp, 6, 2);

  Serial.print(F("\n"));

  return(true);
}


/**
 * According to the datasheet: PMx defines particles with a size smaller than “x” micrometers (e.g., PM2.5 = particles smaller than 2.5 μm).
 *
 *assume :  PM0.5   PM1    PM2.5  PM4    PM10       Typical size
 *          30.75 / 35.2 / 35.4 / 35.4 / 35.4 #/cm³ -> 0.54μm
 *
 * That means (taking the samples mentioned above):
 * 30.75 have a size up to 0.5 um               >> avg. size impact = 30.75 * 0.499
 * 35.2 - 30.75 have a size between 0.5 and 1   >> avg. size impact = (35.2 - 30.75) * 0.99
 * 35.4 - 35.2 have a size between 1 and 2.5um >> etc
 *
 * Add the avg. size impact values ( 20.325) and divide by total = PM10 = 35.4) gives a calculated avg size of 0.57.
 *
 * PM0.5  PM1   PM2.5   PM4   PM10  avg size
 * 30.75   35.2  35.4  35.4  35.4  0.54
 * 0.499   0.99  2.49  3.99  9.99
 * 15.345  4.40  0.498    0    0   20.247
 *
 *                   Calculated average : 0.5719
 *
 * It is not a 100% fit. Maybe they apply different multiplier for size impact, maybe have more information in the sensor than exposed,
 * maybe include a number of the previous measurements in the calculations to prevent the number jump up and down too much
 * between the snap-shots.

 * I had a sketch running for 175 samples, sample every 3 seconds
 * The average for the 175 typical size was : 0,575451860465117 um
 * The average for 175 calculated avg was : 0,583083779069767 um,
 * Thus a delta of 0,007631918604651 over 175 samples. The error margin of 1.33%. I can live with that.
 *
 * One suprising aspect is when float's were used to calculate often 0.57620 is the outcome... When checking the values and calculations
 * with a spreadsheet, there was a mismatch in the result . (error with float measurement ?)
 *
 * Hence the double values are applied.
 *
 */
double calc_avg(struct sps_values v)
{
  double a,b;

  a = (double) v.NumPM0 * (double) 0.499;
  /*Serial.print(F("\n a: "));
  Serial.print(a);
  Serial.print(F("\t a: "));
*/
  b = (double) (v.NumPM1  - v.NumPM0);
  a += b * (double) 0.99;
  /*Serial.print(a);
  Serial.print(F("\t b "));
  Serial.print(b);
  Serial.print(F("\t a "));
*/
  b = (double) (v.NumPM2  - v.NumPM1);
  a += b * (double) 2.49;
  /*Serial.print(a);
  Serial.print(F("\t b "));
  Serial.print(b);
  Serial.print(F("\t a "));;
  */
  b = (double) (v.NumPM4  - v.NumPM2);
  a += b * (double) 3.99;
  /*Serial.print(a);
  Serial.print(F("\t b "));
  Serial.print(b);
  Serial.print(F("\t a "));
*/
  b = (double) (v.NumPM10  - v.NumPM4);
  a += b * (double) 9.99;
  /*Serial.print(a);
  Serial.print(F("\t b "));
  Serial.print(b);
  Serial.print(F("\t a "));

  Serial.print(a);
  Serial.print(F("\t b "));
  Serial.print(b);
  Serial.print(F("\n"));
  */
  return(a / (double) v.NumPM10);
}

/**
 * @brief : calculate error margin based X amount of samples
 * @param v: current read measurement
 * @param avg : current calculated average
 *
 * return : running error margin
 */
double error_margin(struct sps_values v, double avg)
{
  static uint8_t loaded = 0;    // number of loaded values
  uint8_t i;
  double  error = 0, tot_t = 0;

  // if loaded history is full
  if (loaded == INCLUDE_ERRORCALC)
  {
    // shift out oldest
    for(i = 0 ; i < loaded ; i++){
        compta[i].typical = compta[i+1].typical;
        compta[i].avg = compta[i+1].avg;
    }
  }

  // add new values
  compta[loaded].typical = v.PartSize;
  compta[loaded].avg = avg;

  // as long as history not fully loaded
  if (loaded < INCLUDE_ERRORCALC) loaded++;

  // get the running values (as far as they are in history)
  for (i = 0; i < loaded; i++){
    error += (double) (compta[i].typical - compta[i].avg);
    tot_t += (double) compta[i].typical;
  }

  // calculate error
  return( error /  tot_t);
}

/**
 *  @brief : continued loop after fatal error
 *  @param mess : message to display
 *  @param r : error code
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
