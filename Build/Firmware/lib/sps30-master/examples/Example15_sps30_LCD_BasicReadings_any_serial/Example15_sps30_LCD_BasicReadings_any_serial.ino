/************************************************************************************
 *  Copyright (c) January 2019, version 1.0     Paul van Haastrecht
 *
 *  Version 1.1 Paul van Haastrecht
 *  - Changed the I2C information / setup.
 *
 *  Version 1.1.1 Paul van Haastrecht / March 2020
 *  - Fixed compile errors and warnings.
 *
 *  Version 1.1.2 Paul van Haastrecht / March 2020
 *  - added versions level to GetDeviceInfo()
 *
 *  Version 1.4.2 Paul van Haastrecht / May 2020
 *  - demonstrate any serial channel selection
 *
 *  Version 1.4.9 Paul van Haastrecht / October 2020
 *  - as example12, but now also display the output on an LCD
 *    used the https://www.sparkfun.com/products/16396
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch will connect to an SPS30 for getting data and
 *  display the available data. You can select any serial port and use LCD.
 *
 *  =========================  Hardware connections =================================
 *  /////////////////////////////////////////////////////////////////////////////////
 *  ## UART UART UART UART UART UART UART UART UART UART UART UART UART UART UART  ##
 *  /////////////////////////////////////////////////////////////////////////////////
 *
 *  Sucessfully test has been performed on an ESP32:
 *
 *  Using Serial1, setting the RX-pin(25) and TX-pin(26)
 *
 *  SPS30 pin     ESP32    LCD
 *  1 VCC -------- VUSB ----  RAW 3V3-9
 *  2 RX  -------- TX                         pin 26 !! NEEDS A CHANGE ON LINE 221
 *  3 TX  -------- RX                         pin 25 !! NEEDS A CHANGE ON LINE 221
 *  4 Select                                  (NOT CONNECTED)
 *  5 GND -------- GND ------ GND
 *                 SDA ------- DA
 *                 SCL ------- CL
 *
 *
 *  The LCD has pull-up resistors to 3v3 already
 *  GND is close to edge of the SPS30, VCC is most inside pin
 *
 *  Also successfully tested on Serial2 (default pins TX:17, RX: 16)
 *  NO level shifter is needed as the SPS30 is TTL 5V and LVTTL 3.3V compatible
 *  ..........................................................
 *  Successfully tested on ATMEGA2560, Due
 *  Used Serial2. No need to set/change RX or TX pin
 *  SPS30 pin     ATMEGA   LCD
 *  1 VCC -------- 5V ---- RAW 3V3 -9
 *  2 RX  -------- TX2                      pin 16
 *  3 TX  -------- RX2                      pin 17
 *  4 Select                                (NOT CONNECTED)
 *  5 GND -------- GND ----GND
 *                 SDA ---- DA
 *                 SCL ---- CL
 *
 *  The LCD has pull-up resistors to 3v3 already
 *  GND is close to edge of the SPS30, VCC is most inside pin
 *
 *  Also tested on Serial1 and Serial3 successfully
 *  .........................................................
 *  Successfully tested on Apollo3 / Artemis ATP
 *  Used Serial1. No need to set/change RX or TX pin
 *  SPS30 pin     ATMEGA   LCD
 *  1 VCC -------- 5V ---- RAW 3V3 -9
 *  2 RX  -------- TX1
 *  3 TX  -------- RX1
 *  4 Select                                (NOT CONNECTED)
 *  5 GND -------- GND ----GND
 *                 SDA ---- DA
 *                 SCL ---- CL
 *
 *  .........................................................
 *  Failed testing on UNO
 *  Had to use softserial as there is not a separate serialport. But as the SPS30
 *  is only working on 115K the connection failed all the time with CRC errors.
 *
 *  Not tested ESP8266
 *  As the power is only 3V3 (the SPS30 needs 5V)and one has to use softserial.
 *
 *  ***********************************************************************************************
 * https://www.sparkfun.com/products/16396
 *
 * The SparkFun SerLCD is an AVR-based, serial enabled LCD that provides a simple and cost
 * effective solution for adding a 16x2 Black on RGB Liquid Crystal Display into your project.
 * Both the SPS30 and LCD can be connected on the same WIRE device.
 *
 * The Qwiic adapter should be attached to the display as follows. If you have a model (board or LCD)
 * without QWiic connect, or connect is indicated.
 * Display  / Qwiic Cable Color        LCD -connection without Qwiic
 * GND      / Black                    GND
 * RAW      / Red                      3V3 -9 v
 * SDA      / Blue                     I2c DA
 * SCL      / Yellow                   I2C CL
 *
 * Note: If you connect directly to a 5V Arduino instead, you *MUST* use
 * a level-shifter on SDA and SCL to convert the i2c voltage levels down to 3.3V for the display.
 * !!!! Measured with a scope it turns out that the pull up is already to 3V3 !!!!
 *
 * If ONLYONBUTTON is set, connect a push-button switch between pin BUTTONINPUT and ground.
 *
 *  ================================= PARAMETERS =====================================
 *  From line 140 there are configuration parameters for the program
 *
 *  ================================== SOFTWARE ======================================
 *   *  MAKE SURE TO INSTALL http://librarymanager/All#SparkFun_SerLCD.
 *
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
// define communication channel to use
/////////////////////////////////////////////////////////////
#define SP30_COMMS Serial1
#define LCDCON Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
 //////////////////////////////////////////////////////////////
#define DEBUG 0

//////////////////////////////////////////////////////////////////////////
//                SELECT LCD settings                                   //
//////////////////////////////////////////////////////////////////////////
// what are PM2.5 limits good , bad, ugly
// every region in the world has it's own definition (sometimes multiple (like Canada)
// for Europe : https://www.airqualitynow.eu/download/CITEAIR-Comparing_Urban_Air_Quality_across_Borders.pdf
// for US : https://en.wikipedia.org/wiki/Air_quality_index#cite_note-aqi_basic-11
// for UK : https://en.wikipedia.org/wiki/Air_quality_index
// for India : https://en.wikipedia.org/wiki/Air_quality_index#cite_note-aqi_basic-11
// for canada : https://www.publichealthontario.ca/-/media/documents/air-quality-health-index.pdf?la=en
//  or          https://en.wikipedia.org/wiki/Air_Quality_Health_Index_(Canada)
//
// default PM2_LIMITLOW = 55 andPM2_LIMITHIGH=110 is based on Europe
///////////////////////////////////////////////////////////////////////////

#define LCDBACKGROUNDCOLOR  1    // Normal background color: 1 = white, 2 = red, 3 = green 4 = blue 5 = off

float PM2_LIMITLOW = 55.0 ;      // Background LCD color will start LCDBACKGROUNDCOLOR and turn to blue if
                                 // PM2.5 is above this limit to return to LCDBACKGROUNDCOLOR background when below.
                                 // set to zero to disable

float PM2_LIMITHIGH = 110.0 ;    // Background LCD color will start LCDBACKGROUNDCOLOR and turn to red if
                                 // PM2.5 is above this limit to return to LCDBACKGROUNDCOLOR background when below.
                                 // set to zero to disable

#define ONLYONLIMIT false        // only display the results on the LCD display if the PM2_LIMIT is exceeded
                                 // set to false disables this option.
                                 // do NOT select together with ONLYONBUTTON
                                 // make sure to set PM2_LIMIT > 0 (compile will fail)

#define ONLYONBUTTON false       // only display the results on the LCD display (red) if the PM2_LIMIT
                                 // is exceeded OR for LCDTIMEOUT seconds if a button is pushed
                                 // set to false disables this option
                                 // do NOT select together with ONLYONLIMIT
                                 // if PM2_LIMIT is zero the LCD will only display when button is pressed
#if ONLYONBUTTON == true
#define BUTTONINPUT  10         // Digital input where button is connected for ONLYONBUTTON between GND
                                 // is ignored if ONLYONBUTTON is set to false
                                 // Artemis / Apollo3 set as D27
                                 // ESP32, Arduino set as 10

#define LCDTIMEOUT 10            // Number of seconds LCD is displayed after button was pressed
#endif                           // is ignored if ONLYONBUTTON is set to false


///////////////////////////////////////////////////////////////
///// NORMALLY NO CHANGES BEYOND THIS POINT NEEDED  ///////////
// BUT IF YOU NEED TO SET DIFFERENT PINS FOR SERIAL PORT     //
// AS IS THE CASE FOR ESP32 SERIAL1
// CHANGE LINE 221:  SP30_COMMS.begin(115200);               //
///////////////////////////////////////////////////////////////

// checks will happen at pre-processor time to
#if ONLYONLIMIT == true && ONLYONBUTTON == true
#error "you can NOT set BOTH ONLYONLIMIT and ONLYONBUTTON to true"
#endif

// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
void serialTrigger(char * mess);
void ErrtoMess(char *mess, uint8_t r);
void Errorloop(char *mess, uint8_t r);
void GetDeviceInfo();
bool read_all();

#include <SerLCD.h> // Click here to get the library: http://librarymanager/All#SparkFun_SerLCD
SerLCD lcd;         // Initialize the library with default I2C address 0x72

// create constructor
SPS30 sps30;

struct sps_values val;

void setup() {

  Serial.begin(115200);
  Serial.println(F("SPS30-Example16: Basic reading with any Serial channel select and LCD"));

  // set driver debug level
  sps30.EnableDebugging(DEBUG);

  LCDCON.begin();

  // initialize LCD
  lcdinit();

  Serial.println(F("Trying to connect."));

  // Initialize communication port
  // FOR ESP32 SERIAL1 CHANGE THIS TO : E.G. RX-PIN 25, TX-PIN 26
  // SP30_COMMS.begin(115200, SERIAL_8N1, 25, 26);

  SP30_COMMS.begin(115200);

  // Initialize SPS30 library
  if (! sps30.begin(&SP30_COMMS)){
    lcd.setBacklight(255, 0, 0);    // bright red
    lcd.clear();
    lcd.write("SPS30 comms");
    lcd.setCursor(0, 1);            // pos 0, line 1
    lcd.write("Error");
    Errorloop((char *) "Could not set serial communication channel.", 0);
  }

  // check for SPS30 connection
  if (sps30.probe()) Serial.println(F("Detected SPS30."));
  else {
    lcd.setBacklight(255, 0, 0);    // bright red
    lcd.clear();
    lcd.write("SPS30 probe");
    lcd.setCursor(0, 1);            // pos 0, line 1
    lcd.write("Error");
    Errorloop((char *) "could not probe / connect with SPS30.", 0);
  }

  // reset SPS30 connection
  if (! sps30.reset()) {
    lcd.setBacklight(255, 0, 0);    // bright red
    lcd.clear();
    lcd.write("Could not");
    lcd.setCursor(0, 1);            // pos 0, line 1
    lcd.write("reset SPS30");
    Errorloop((char *) "could not reset.", 0);
  }

#if ONLYONLIMIT == true
  if (PM2_LIMITLOW == 0 || PM2_LIMITHIGH == 0) {
    lcd.setBacklight(255, 0, 0);    // bright red
    lcd.clear();
    lcd.write("ERROR PM2 LIMIT");
    lcd.setCursor(0, 1);            // pos 0, line 1
    lcd.write("NOT SET");
    Errorloop((char *) "ERROR: you MUST set PM2_LIMIT when ONLYONLIMIT is true", 0);
  }
#endif

  // read device info
  GetDeviceInfo();

  // start measurement
  if (sps30.start()) Serial.println(F("Measurement started"));
  else {
    lcd.setBacklight(255, 0, 0);    // bright red
    lcd.clear();
    lcd.write("Could not start");
    lcd.setCursor(0, 1);            // pos 0, line 1
    lcd.write("measurement");
    Errorloop((char *) "Could NOT start measurement", 0);
  }
}

void loop() {
  read_all();

  // delay for 3 seconds but capture button pressed
  for (int i=0; i < 10; i++){
    delay(300);
    checkButton();
  }
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
    lcd.setCursor(0, 0);            // pos 0, line 0
    lcd.write("Snr:");
    lcd.setCursor(0, 1);            // pos 0, line 1
    if(strlen(buf) > 0)  {
      Serial.println(buf);
      lcd.write(buf);
    }
    else{
      Serial.println(F("not available"));
      lcd.write("Not available");
    }
  }
  else{
   lcd.setCursor(0, 0);            // pos 0, line 0
   lcd.write("Error during");
   lcd.setCursor(0, 1);            // pos 0, line 1
   lcd.write("reading snr.");

   ErrtoMess((char *) "could not get serial number", ret);
  }
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
  Serial.print(".");  Serial.println(v.minor);

  Serial.print(F("Hardware level: "));  Serial.println(v.HW_version);

  Serial.print(F("SHDLC protocol: ")); Serial.print(v.SHDLC_major);
  Serial.print(".");  Serial.println(v.SHDLC_minor);

  Serial.print(F("Library level : "));  Serial.print(v.DRV_major);
  Serial.print(".");  Serial.println(v.DRV_minor);

  delay(5000);
}

/**
 * @brief : read and display all values
 */
bool read_all()
{
  static bool header = true;
  uint8_t ret, error_cnt = 0;

  // loop to get data
  do {

    ret = sps30.GetValues(&val);

    // data might not have been ready
    if (ret == SPS30_ERR_DATALENGTH){

        if (error_cnt++ > 3) {
          ErrtoMess((char *) "Error during reading values: ",ret);
          printLCD(false);
          return(false);
        }
        delay(1000);
    }

    // if other error
    else if(ret != SPS30_ERR_OK) {
      ErrtoMess((char *) "Error during reading values: ",ret);
      printLCD(false);
      return(false);
    }

  } while (ret != SPS30_ERR_OK);

  // only print header first time
  if (header) {
    Serial.println(F("-------------Mass -----------    ------------- Number --------------   -Average-"));
    Serial.println(F("     Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
    Serial.println(F("P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPartSize\n"));
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
  Serial.print(F("\n"));

  printLCD(true);

  return(true);
}


// checks for button pressed to set the LCD on and keep on for
// LCDTIMEOUT seconds after button has been released.
// return true to turn on OR false to turn / stay off.
bool checkButton()
{
#if ONLYONBUTTON == true
  static unsigned long startTime = 0;

  // button pressed ?
  if (! digitalRead(BUTTONINPUT)){
    startTime = millis();
  }

  if (startTime > 0) {
    if (millis() - startTime < (LCDTIMEOUT*1000))  return true;
    else  startTime = 0;      // reset starttime
  }

  return false;

#endif //ONLYONBUTTON
  return true;
}

// initialize the LCD
void lcdinit()
{
  lcd.begin(LCDCON);
  lcdsetbackground();         // set background

#if ONLYONBUTTON == true
  pinMode(BUTTONINPUT,INPUT_PULLUP);
#endif
}

// set requested background color
void lcdsetbackground()
{

#if ONLYONLIMIT == true
  lcd.setBacklight(0, 0, 0);  // off
  return;
#endif

#if ONLYONBUTTON == true
  if(! checkButton()) {
    lcd.setBacklight(0, 0, 0);  // off
    return;
  }
#endif //ONLYONBUTTON

  switch(LCDBACKGROUNDCOLOR){

    case 2:   // red
      lcd.setBacklight(255, 0, 0); // bright red
      break;
    case 3:   // green
      lcd.setBacklight(0, 255, 0); // bright green
      break;
    case 4:   // blue
      lcd.setBacklight(0, 0, 255); // bright blue
      break;
    case 5:   // off
      lcd.setBacklight(0, 0, 0);
      break;
    case 1:   // white
    default:
      lcd.setBacklight(255, 255, 255); // bright white
  }
}

// print results on LCD
// @parameter dd : true is display new data else no-data indicator
void printLCD(bool dd)
{
  char buf[10];
  static bool limitLowWasSet = false;
  static bool limitHighWasSet = false;
  static bool MeasureInd = true;

  // change background to red on high limit (if limit was set)
  if (PM2_LIMITHIGH > 0) {

    if (val.MassPM2 > PM2_LIMITHIGH){
      // change once..
      if(! limitHighWasSet){
        lcd.setBacklight(255, 0, 0); // bright red
        limitHighWasSet = true;
      }
    }
    else if (limitHighWasSet){
      lcd.setBacklight(0, 0, 255); // bright blue
      limitHighWasSet = false;
    }
  }

  // change background on limit (if limit was set)
  if (PM2_LIMITLOW > 0) {

    if ( ! limitHighWasSet ){

        if (val.MassPM2 > PM2_LIMITLOW){
          // change once..
          if(! limitLowWasSet){
            lcd.setBacklight(0, 0, 255); // bright blue
            limitLowWasSet = true;
          }
        }
        else if (limitLowWasSet){
          lcdsetbackground();           // reset to original request
          limitLowWasSet = false;
        }
    }
  }

// only display if limit has been reached
#if ONLYONLIMIT == true
  if(! limitLowWasSet || ! limitLowWasSet) {
    lcd.clear();
    return;
  }
#endif //ONLYONLIMIT

// only display if button was pressed or limit has been reached
#if ONLYONBUTTON == true
  if(! checkButton() && ! limitWasSet) {
    lcd.clear();
    lcd.setBacklight(0, 0, 0);  // off
    return;
  }
#endif //ONLYONBUTTON

  // if no data available indicate with . and return
  if (!dd) {
    lcd.setCursor(15, 0);            // pos 15, line 0

    // display measurement indicator
    if (MeasureInd)  lcd.write(".");
    else lcd.write(" ");

    MeasureInd = !MeasureInd;
    return;
  }

  // just in case next no-data, start display .
  MeasureInd = true;

  lcd.clear();
  lcd.write("PM1: PM2: PM10:");

  lcd.setCursor(0, 1);            // pos 0, line 1
  FromFloat(buf, val.MassPM1,1);
  lcd.write(buf);

  lcd.setCursor(5, 1);            // pos 5, line 1
  FromFloat(buf, val.MassPM2,1);
  lcd.write(buf);

  lcd.setCursor(10, 1);           // pos 10, line 1
  FromFloat(buf, val.MassPM10,1);
  lcd.write(buf);
}

// This is a workaround as sprintf on Artemis/Apollo3 is not recognizing %f (returns empty)
// based on source print.cpp/ printFloat
int FromFloat(char *buf, double number, uint8_t digits)
{
  char t_buf[10];
  buf[0] = 0x0;

  if (isnan(number)) {
    strcpy(buf,"nan");
    return 3;
  }

  if (isinf(number)) {
    strcpy(buf,"inf");
    return 3;
  }

  if (number > 4294967040.0 || number <-4294967040.0) {
    strcpy(buf,"ovf");
    return 3;
  }

  // Handle negative numbers
  if (number < 0.0)
  {
     strcat(buf,"-");
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;

  sprintf(t_buf,"%ld", int_part);
  strcat(buf,t_buf);

  if (digits > 0) {

    // Print the decimal point, but only if there are digits beyond
    strcat(buf,".");

    // Extract digits from the remainder one at a time
    while (digits-- > 0)
    {
      remainder *= 10.0;
      unsigned int toPrint = (unsigned int)(remainder);
      sprintf(t_buf,"%d", toPrint);
      strcat(buf,t_buf);
      remainder -= toPrint;
    }
  }

  return (int) strlen(buf);
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
