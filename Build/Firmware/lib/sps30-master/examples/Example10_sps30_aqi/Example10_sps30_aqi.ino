/************************************************************************************
 *  Copyright (c) March 2019, initial version Paul van Haastrecht
 *
 *  version 1.0.1 / March 2019
 *     Added base-option for PM2.5 and PM10
 *
 *  Version 1.1.2 Paul van Haastrecht / March 2020
 *  - added versions level to GetDeviceInfo()
 *
 *  =========================  Highlevel description ================================
 *
 * This sketch will connect to an SPS30 for getting data, store data and display the
 * available data. The sketch has been devloped and tested only for an ESP32.
 *
 * The data during-hour and during-day is stored RAM and at end-of-day it will be
 * combined with data from previous days that is stored in NVRAM.
 *
 * Based on the captured data and the region setting, it can calculate and display
 * an air qualityindex that is applicable for that region.
 *
 * While this example is using SPS30 as a sensor, there is no reason why the AQI library
 * could not be combined with other sensors. For the SPS30 you will need the library
 * from https://github.com/paulvha/sps30
 *
 * !!!!!!! See the seperate document (AQI.odt) for reasons and more information !!!!!.
 *
 *  =========================  SPS30 Hardware connections =================================
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
 *  //////////////////////////////////////////////////////////////////////////////////
 *  ## I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C ##
 *  //////////////////////////////////////////////////////////////////////////////////
 *  NOTE 1:
 *  Depending on the Wire / I2C buffer size we might not be able to read all the values.
 *  The buffer size needed is at least 60 while on many boards this is set to 32. The driver
 *  will determine the buffer size and if less than 64 only the MASS values are returned.
 *  You can manually edit the Wire.h of your board to increase (if you memory is large enough)
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
 *
 *  ================================= PARAMETERS =====================================
 *
 *  From line 101 there are configuration parameters for the program
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

#include <aqi.h>
#include <sps30.h>

/////////////////////////////////////////////////////////////
#define VERSION "1.0.1"

/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid options:
 *   I2C_COMMS              use I2C communication
 *   SOFTWARE_SERIAL        Arduino variants (NOTE)
 *   SERIALPORT             ONLY IF there is NO monitor attached
 *   SERIALPORT1            Arduino MEGA2560, Sparkfun ESP32 Thing : MUST define new pins as defaults are used for flash memory)
 *   SERIALPORT2            Arduino MEGA2560 and ESP32
 *   SERIALPORT3            Arduino MEGA2560 only for now

 * NOTE: Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on any device. */
/////////////////////////////////////////////////////////////
#define SP30_COMMS SERIALPORT1

/////////////////////////////////////////////////////////////
/* define RX and TX pin for softserial and Serial1 on ESP32 for SPS30
 * can be set to zero if not applicable / needed           */
/////////////////////////////////////////////////////////////
#define TX_PIN 26
#define RX_PIN 25

/////////////////////////////////////////////////////////////
/* define SPS30 driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
 //////////////////////////////////////////////////////////////
#define DEBUG 0

/////////////////////////////////////////////////////////////
/* maximum keyboard input buffer size */
#define KEYB_BUF 10
/////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
// SPS30 routines
void ErrtoMess(char *mess, uint8_t r);
void GetDeviceInfo();
bool read_all();

// AQI routines
void read_nvram();
void write_nvram();
void write_region();
void clear_nvram();
void force_upd();
void read_Ram();
void set_current_hour();
void get_current_hour();
void read_aqi();
void disp_region(uint8_t region);
void disp_bnd_europe(void *n, bool hour);

// supporting routines
void serialTrigger(char * mess);
uint8_t keyboard_inp(char * mess);
void disp_help();
void Errorloop(char *mess, uint8_t r);
void print_column(char *mess, uint8_t width);
void print_aligned(double val, signed char width, unsigned char prec, uint8_t w);
uint8_t yes_or_no();
uint8_t get_new_number();

// create constructors
SPS30 sps30;
AQI aqi;

// global variables to use
struct AQI_NVRAM nv;              // holds NVRAM info
char keyb[KEYB_BUF +1];           // holds keyboard input
bool show_measurements = false;   // display measurements performed
bool base = HISTORY;              // PM2.5 and PM10 values to use

void setup()
{
  Serial.begin(115200);

  serialTrigger((char *) "SPS30-Example10: Read values, store and calculate AQI. ESP32 ONLY !  Press <enter> to start");

  if (!EEPROM.begin(AQISIZE)) Errorloop((char *) "failed to initialise NVRAM", 0);

  Serial.println(F("Trying to connect to SPS30"));

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

  if (SP30_COMMS == I2C_COMMS) {
    if (sps30.I2C_expect() == 4)
      Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }

  disp_help();
}

void loop()
{
  static uint8_t cnt = 0;

  // check for keyboard input
  if (keyboard_inp((char *) "\nEnter your command (? for help)") > 0) {

    Serial.print(F("Received ==========>>>> "));
    Serial.println(keyb);
    if (strcmp(keyb,"nvram") == 0) read_nvram();
    else if ((strcmp(keyb,"?") == 0) || (strcmp(keyb,"help") == 0)) disp_help();
    else if (strcmp(keyb,"clear") == 0) clear_nvram();
    else if (strcmp(keyb,"update") == 0) write_nvram();
    else if (strcmp(keyb,"region") == 0) write_region();
    else if (strcmp(keyb,"ram") == 0) read_Ram();
    else if (strcmp(keyb,"force") == 0) force_upd();
    else if (strcmp(keyb,"sethour") == 0) set_current_hour();
    else if (strcmp(keyb,"gethour") == 0) get_current_hour();
    else if (strcmp(keyb,"aqi") == 0) read_aqi();
    else if (strcmp(keyb,"show") == 0)  show_measurements = !show_measurements;
    else if (strcmp(keyb,"base") == 0)  base = !base;
    else Serial.println(F("Unknown command"));
  }

  // read, store and show(?) values from SPS30 (update every 5 seconds)
  if (cnt++ == 10) {read_all(); cnt = 0;}

  delay(500);
}

/**
 * @brief : display help text
 */
void disp_help()
{
  Serial.print(F("\n*************** HELP **************** Version :"));
  Serial.println(VERSION);
  Serial.println(F("region : update region code"));
  Serial.print  (F("base   : toggle PM2.5 / PM10 value to use, currently "));
  if(base == YESTERDAY) Serial.println(F("from PREVIOUS day"));
  else Serial.println(F("from HISTORY"));
  Serial.println(F("aqi    : obtain air quality based region code"));
  Serial.println(F("gethour: get the current hour being captured"));
  Serial.println(F("sethour: set the current hour of current day"));
  Serial.println(F("nvram  : read after-day NVRAM values"));
  Serial.println(F("ram    : read during-hour & during-day volatile RAM values"));
  Serial.println(F("show   : toggle display current measured values"));
  Serial.println(F("clear  : clear NVRAM values"));
  Serial.println(F("update : update NVRAM values"));
  Serial.println(F("force  : force adding current day statistics to NVRAM"));
  Serial.println(F("************************************"));
}


///////////////////////////////////////////
/////////// SPS30 routines  ///////////////
/////////////////////////// ///////////////
/**
 * @brief : read and display device info SPS30
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
 * @brief : read and display all values
 */
bool read_all()
{
  static bool header = true;
  static bool first = true;
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

  // often seen that the first reading to be "out of bounds". so we skip it
  if (first) { first = false; return(true); }

  // save values
  aqi.Capture(val.MassPM2, val.MassPM10);

  // if NOT requested show the measurements
  if (! show_measurements) return(true);

  // only print header first time
  if (header) {
    Serial.println(F("---------------------Mass --------------------------    ------------------------------ Number ----------------------------------       Partsize"));
    Serial.println(F("             Concentration [μg/m3]                                              Concentration [#/cm3]                                     [μm]"));
    Serial.println(F(" PM1.0          PM2.5          PM4.0          PM10           PM0.5          PM1.0          PM2.5          PM4.0          PM10           Typical"));
    header = false;
  }

  print_aligned((double) val.MassPM1, 8, 5, 15);
  print_aligned((double) val.MassPM2, 8, 5, 15);
  print_aligned((double) val.MassPM4, 8, 5, 15);
  print_aligned((double) val.MassPM10, 8, 5, 15);
  print_aligned((double) val.NumPM0, 9, 5, 15);
  print_aligned((double) val.NumPM1, 9, 5, 15);
  print_aligned((double) val.NumPM2, 9, 5, 15);
  print_aligned((double) val.NumPM4, 9, 5, 15);
  print_aligned((double) val.NumPM10, 9, 5, 15);
  print_aligned((double) val.PartSize, 7, 5, 15);

  Serial.print(F("\n"));

  return(true);
}

///////////////////////////////////////////////////////////////
/////////// Air Quality Index supporting routines  ////////////
///////////////////////////////////////////////////////////////

/**
 * @brief : force an update of RAM values to NVRAM
 *
 * Performs a double check on the intention, which MUST be entered in capitals
 *
 * An end_of_day cycle will be done if at least a one hour of data was captured. The day and hour statistics and count are reset to start.
 */
void force_upd()
{
  while (keyboard_inp((char *)"type : FORCE <enter> to update NVRAM and restart hour and day (no = return without update)") == 0);

  if ((strcmp(keyb,"no") == 0) || (strcmp(keyb,"NO") == 0)){
    Serial.println(F("Cancelled"));
    return;
  }
  else if(strcmp(keyb,"FORCE") == 0)  {
    if (aqi.ForceUpdate())    Serial.print(F("completed"));
    else Serial.print(F("Nothing to update"));
  }
  else {
    Serial.print(F("Unknown command : "));
    Serial.println(keyb);
  }
}

/**
 * @brief : set starting hour in the current day.
 * will reset current day statistics
 */
void set_current_hour()
{
  uint8_t hour;

  // display current hour
  get_current_hour();

  if (get_new_number() == 1) {
    hour = (uint8_t) atoi(keyb);

    if (hour > 23) {
      Serial.print(hour);
      Serial.println(F(" : invalid hour number. No change"));
    }
    else {
      if (aqi.SetHour(hour)) Serial.print(F("completed"));
      else Serial.print(F("update failed"));
      get_current_hour();
    }
  }
  else {
    Serial.print(keyb);
    Serial.println(F(" : NO change"));
  }
}

/**
 * @brief : Display the current hour in the current day being captured
 */
void get_current_hour()
{
  Serial.print(F("Current hour of day being captured : "));
  Serial.println(aqi.GetHour());
}

/**
 * @brief : obtain the Air Quality Index for the selected region
 */
void read_aqi()
{
  struct AQI_info r;

  // read nvram and calculate AQI
  if ( ! aqi.GetAqi(&r, base)) {
    Serial.println(F("ERROR : No data or NO region selected"));
    return;
  }

  disp_region(r.nv._region);

  print_column((char *) "PM2.5 and PM10 used :",30);
  if(r.base == YESTERDAY) Serial.println(F("from PREVIOUS day"));
  else Serial.println(F("from HISTORY"));

  print_column((char *) "Air Quality Index calculated:",30);
  Serial.print(r.aqi_index);
  Serial.print(F("  means: "));
  Serial.println(r.aqi_name);

  print_column((char *) "The AQI is driven by  :",30);
  if(r.aqi_indicator == PM25) Serial.print(F("PM2.5  value: "));
  else Serial.print(F("PM10   value: "));
  Serial.println(r.aqi_pmvalue);

  print_column((char *) "Current pollution band      :",30);
  Serial.println(r.aqi_bnd);
  print_column((char *) "Current pollution band low  :",30);
  Serial.println(r.aqi_bnd_low);
  print_column((char *) "Current pollution band high :",30);
  Serial.println(r.aqi_bnd_high);

  disp_bnd_europe(&r.nv,false);
}

/**
 * @brief : reset the NVRAM area to zero
 *
 * Performs a double check on the intention, which MUST be entered in capitals
 */
void clear_nvram()
{
  while (keyboard_inp((char *)"type : CLEAR <enter> to clear NVRAM (no = return without update)") == 0);

    if ((strcmp(keyb,"no") == 0) || (strcmp(keyb,"NO") == 0)){
      Serial.println(F("cancelled"));
      return;
    }

    else if(strcmp(keyb,"CLEAR") == 0) {
      aqi.SetStats(NULL);
      Serial.print(F("completed"));
    }
    else {
      Serial.print(F("Cancelled unknown command : "));
      Serial.println(keyb);
    }
}

/**
 * @brief : display the volatile information of the current hour and day
 */
void read_Ram()
{
  struct AQI_stats r;
  byte x;

  // read Statistics in RAM
  aqi.ReadRam(&r);

  Serial.println(F("\n********** Current HOUR information ***********"));

  print_column((char *) "Amount of samples captured this hour:",40);
  Serial.println(r._within_hr_cnt);

  print_column((char *) "Total PM2.5 measured this hour : ",40);
  Serial.print(r._within_hr_25um);

  if (r._within_hr_cnt > 0) {
    Serial.print(F("\taverage: " ));
    Serial.print(r._within_hr_25um/r._within_hr_cnt);
  }

  Serial.println();
  print_column((char *) "Total PM10 measured this hour  : ",40);
  Serial.print(r._within_hr_10um);

  if (r._within_hr_cnt > 0) {
    Serial.print(F("\taverage: " ));
    Serial.print(r._within_hr_10um/r._within_hr_cnt);
  }

  Serial.println();
  print_column((char *) "Number of minutes to go   : ",40);
  Serial.println(60 - ((millis() - r._start_hour) / 60000));

  Serial.println(F("\n********** Current DAY information ***********"));

  print_column((char *) "Number of hours captured:",40);
  Serial.println(r._daily_cnt);

  print_column((char *) "Hours offset:",40);
  Serial.println(r._daily_offset);

  print_column((char *) "Number of hours to go:",40);
  Serial.println(24 -(r._daily_offset + r._daily_cnt));

  print_column((char *) "Total PM2.5 measured this day so far:",40);
  Serial.print(r._daily_25um);

  if (r._daily_cnt > 0) {
    Serial.print(F("\taverage: " ));
    Serial.print(r._daily_25um /r._daily_cnt);
  }

  Serial.println();
  print_column((char *) "Total PM10 measured this day so far:",40);
  Serial.print(r._daily_10um);

  if (r._daily_cnt > 0) {
    Serial.print(F("\taverage: " ));
    Serial.print(r._daily_10um/r._daily_cnt);
  }

  Serial.println();
  print_column((char *) "PM2.5 day maximum :",40);
  Serial.println(r._daily_25um_max);

  print_column((char *) "PM10 day maximum  :",40);
  Serial.println(r._daily_10um_max);

  disp_bnd_europe(&r, true);
}

/**
 * @brief : display current region
 */
void disp_region(uint8_t region)
{
  print_column((char *) "Region selected : ",30);

  for (byte x = 0 ; x < sizeof(regions)/sizeof(struct AQI_region) ; x++)
  {
    if (regions[x].ind == region) {
      Serial.println(regions[x].name);
      return;
    }
  }

  Serial.print(region);
  Serial.println(F("Unknown region code"));
}

/**
 * @brief : display europe specific band info
 * @param nn : pointer to structure
 * @bool hour :
 *   true :  display current hours only. struct AQI_stats is expected
 *   false : also display previous day hours and total days info struct AQI_NVRAM is expected
 *
 * This has been done to make it easier to display the Europe only information
 * from a single place
 */
void disp_bnd_europe(void *nn, bool hour)
{
  byte x;

  Serial.println(F("\n"));
  print_column((char *) "**** Europe only ****",30);
  for (x = 0; x < 5; x++)  print_column(AQI_CAQI_hrly[x].cat_name,15);
  Serial.println();

  if (hour) {
    struct AQI_stats *n = (struct AQI_stats *) nn;

    Serial.println();
    print_column((char *) "current day hours _bnd_PM2.5:",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_hrly_bnd_25um[x], 8, 5, 15);

    Serial.println();
    print_column((char *) "current day hours _bnd_PM10 :",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_hrly_bnd_10um[x], 8, 5, 15);
  }
  else {
    struct AQI_NVRAM *n = (struct AQI_NVRAM *) nn;

    print_column((char *) "previous day hours _bnd_PM2.5:",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_hrly_bnd_25um[x], 8, 5, 15);

    Serial.println();
    print_column((char *) "previous day hours _bnd_PM10 :",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_hrly_bnd_10um[x], 8, 5, 15);

    Serial.println();
    print_column((char *) "Captured days _bnd_PM2.5 :",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_daily_bnd_25um[x], 8, 5,15);

    Serial.println();
    print_column((char *) "Captured days _bnd_PM10  :",30);
    for (x = 0; x < 5; x++) print_aligned((double) n->_daily_bnd_10um[x], 8, 5,15);
  }

  Serial.println();
}

/**
 * @brief : read after-day NVRAM values
 */
void read_nvram()
{
  // read from NVRAM
  aqi.GetNv(&nv);

  Serial.println();
  disp_region(nv._region);

  print_column((char *) "Number of days captured :",30);
  Serial.println(nv._cnt);

  print_column((char *) "Total PM2.5 measured :",30);
  Serial.print(nv._25um);

  if (nv._cnt > 0) {
   Serial.print(F("\tdaily average: " ));
   Serial.print(nv._25um/nv._cnt);
  }

  Serial.println();
  print_column((char *) "Total PM10 measured :",30);
  Serial.print(nv._10um);

  if (nv._cnt > 0) {
    Serial.print(F("\tdaily average: " ));
    Serial.print(nv._10um/nv._cnt);
  }

  Serial.println();
  print_column((char *) "Value PM2.5 previous day :",30);
  Serial.println(nv._25um_prev);

  print_column((char *) "Value PM10 previous day  :",30);
  Serial.println(nv._10um_prev);

  print_column((char *) "Max value PM2.5 previous day :",30);
  Serial.println(nv._25um_max);

  print_column((char *) "Max value PM10 previous day  :",30);
  Serial.println(nv._10um_max);

  disp_bnd_europe(&nv,false);
}

/**
 * @brief : update the NVRAM values
 *
 */
void write_nvram()
{
  bool change_done = false;
  uint8_t  x, i;

  Serial.println(F("\n****** MEASUREMENT TEMP0RARILY SUSPENDED *************"));

  // read from NVRAM
  aqi.GetNv(&nv);

  disp_region(nv._region);
  x = yes_or_no();
  if (x == 2) goto write_end;
  if (x == 1) write_region();

  print_column((char *) "\nNumber of days captured :",30);
  Serial.println(nv._cnt);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._cnt = (uint16_t) strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._cnt);
      change_done = true;
  }

  print_column((char *) "\nTotal PM2.5 measured :",30);
  Serial.println(nv._25um);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._25um =  strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._25um);
      change_done = true;
  }

  print_column((char *) "\nTotal PM10 measured :",30);
  Serial.println(nv._10um);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._10um = strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._10um);
      change_done = true;
  }

  print_column((char *) "\nValue PM2.5 previous day :",30);
  Serial.println(nv._25um_prev);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._25um_prev = strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._25um_prev);
      change_done = true;
  }

  print_column((char *) "\nValue PM10 previous day  :",30);
  Serial.println(nv._10um_prev);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._10um_prev = strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._10um_prev);
      change_done = true;
  }

  print_column((char *) "\nMax value PM2.5 previous day :",30);
  Serial.println(nv._25um_max);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._25um_max = strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._25um_max);
      change_done = true;
  }

  print_column((char *) "\nMax value PM10 previous day  :",30);
  Serial.println(nv._10um_max);

  x = get_new_number();
  if (x == 2) goto write_end;
  if (x == 1) {
      nv._10um_max = strtod(keyb, NULL);
      Serial.print(F("value will be set to: "));
      Serial.println(nv._10um_max);
      change_done = true;
  }

  Serial.println("******************** Europe only ***********************\n");
  print_column((char *) "\nPrevious day hours _bndPM2.5 ",30);
  x = yes_or_no();
  if (x == 2) goto write_end;
  if (x == 1) {
      for (i = 0; i < 5; i++) {
        print_column(AQI_CAQI_hrly[i].cat_name,15);
        print_aligned((double) nv._hrly_bnd_25um[i], 8, 5, 15);
        Serial.println();
        x = get_new_number();
        if (x == 2) goto write_end;
        if (x == 1) {
                nv._hrly_bnd_25um[i] = strtod(keyb, NULL);
                Serial.print(F("value will be set to: "));
                Serial.println(nv._hrly_bnd_25um[i]);
                change_done = true;
        }
      }
  }

  print_column((char *) "\nPrevious day hours _bndPM10 ",30);
  x = yes_or_no();
  if (x == 2) goto write_end;
  if (x == 1) {
      for (i = 0; i < 5; i++) {
        print_column(AQI_CAQI_hrly[i].cat_name,15);
        print_aligned((double) nv._hrly_bnd_10um[i], 8, 5, 15);
        Serial.println();
        x = get_new_number();
        if (x == 2) goto write_end;
        if (x == 1) {
                nv._hrly_bnd_10um[i] = strtod(keyb, NULL);
                Serial.print(F("value will be set to: "));
                Serial.println(nv._hrly_bnd_10um[i]);
                change_done = true;
        }
      }
  }

  print_column((char *) "\nCaptured days _bnd_PM2.5 ",30);
  x = yes_or_no();
  if (x == 2) goto write_end;
  if (x == 1) {
      for (i = 0; i < 5; i++) {
        print_column(AQI_CAQI_hrly[i].cat_name,15);
        print_aligned((double) nv._daily_bnd_25um[i], 8, 5, 15);
        Serial.println();
        x = get_new_number();
        if (x == 2) goto write_end;
        if (x == 1) {
                nv._daily_bnd_25um[i] = strtod(keyb, NULL);
                Serial.print(F("value will be set to: "));
                Serial.println(nv._daily_bnd_25um[i]);
                change_done = true;
        }
      }
  }

  print_column((char *) "\nCaptured days _bnd_PM10 ",30);
  x = yes_or_no();
  if (x == 2) goto write_end;
  if (x == 1) {
      for (i = 0; i < 5; i++) {
        print_column(AQI_CAQI_hrly[i].cat_name,15);
        print_aligned((double) nv._daily_bnd_10um[i], 8, 5, 15);
        Serial.println();
        x = get_new_number();
        if (x == 2) goto write_end;
        if (x == 1) {
                nv._daily_bnd_10um[i] = strtod(keyb, NULL);
                Serial.print(F("value will be set to: "));
                Serial.println(nv._daily_bnd_10um[i]);
                change_done = true;
        }
      }
  }

  // write back
  if (change_done) aqi.SetStats(&nv);

write_end:
  Serial.println(F("\n****** MEASUREMENT RESUMING *************"));
}

/**
 * update region to use (defined in aqi_area.h)
 */
void write_region()
{
    uint8_t x, y;

    Serial.println(F("Available regions:\n"));

    for (x = 1; x < sizeof(regions)/sizeof(struct AQI_region); x++) {
      Serial.print(regions[x].ind);
      Serial.print(F(". "));
      Serial.println(regions[x].name);
    }

    Serial.println();

    if (get_new_number() != 1) return;

    y = (uint8_t) atoi(keyb);

    if (y > x-1) {
      Serial.print(y);
      Serial.println(F(" Invalid region. cancelled"));
      return;
    }

    aqi.SetRegion((region_t) y);
    Serial.print(F("Region set to "));
    Serial.println(regions[y].name);
}


////////////////////////////////////////////////////////////////////
////////////  General SUPPORTING ROUTINES  /////////////////////////
////////////////////////////////////////////////////////////////////

/**
 * @brief : reads yes or NO or cancel
 *
 * @return
 *  1 = yes
 *  2 = cancel
 *  0 = something else was answered
 */
uint8_t yes_or_no()
{
  while (keyboard_inp((char *) "Do you want change ? (yes or no or cancel) ") == 0);

  if ((strcmp(keyb,"yes") == 0) || (strcmp(keyb,"YES") == 0)) return 1;
  if ((strcmp(keyb,"cancel") == 0) || (strcmp(keyb,"CANCEL") == 0)) return 2;

  return 0;
}

/**
 * @brief : Tries to obtain a valid number
 *
 * @return
 *  0 = not a valid number available
 *  1 = valid number available in keyb-buffer
 *  2 = cancel
 */
uint8_t get_new_number()
{
  uint8_t x;

  x = yes_or_no();
  if (x != 1) return x;

  while (keyboard_inp((char *) "Enter number (no = return without update)") == 0);

  if ((strcmp(keyb,"no") == 0) || (strcmp(keyb,"NO") == 0)){
      Serial.println(F("Cancelled"));
      return 0;
  }

  // check for valid number/digits
  for (x = 0 ; x < strlen(keyb) ; x++) {

    // a '.' can happen in float
    if ( (!isdigit(keyb[x])) && (keyb[x] != '.') )  {
      Serial.print(keyb);
      Serial.println(" !!! Not a valid number. NO update !!!");
      return 0;
    }
  }

  return 1;
}

/**
 * @brief will print nice aligned columns from string
 *
 * @param mess : message to print
 * @param width : total width of column
 */
void print_column(char *mess, uint8_t width)
{
  uint8_t x, l = 0;

  if (width > strlen(mess)) l = width - strlen(mess);

  Serial.print(mess);

  // pad with spaces
  for (x = 0; x < l ; x++) Serial.print(F(" "));
}

/**
 * @brief will print nice aligned columns from value
 *
 * @param val  : value to print
 * @param width: total width of value including decimal point
 * @param prec : precision after the decimal point
 * @param w    : total column width (including blank space)
 */
void print_aligned(double val, signed char width, unsigned char prec, uint8_t w)
{
  char out[30];

  if (w > 30) w = 30;

  dtostrf(val, width, prec, out);
  print_column(out, w);
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

/**
 * @brief : prints message one time and reads keyboard input,
 * up to <enter> from the serial port.
 *
 * @param mess : message to display
 *
 * @return
 * 0 = no complete buffer received
 * x = number of bytes in complete buffer
 */
uint8_t keyboard_inp(char *mess)
{
  static uint8_t cnt = 0;
  static bool display_mess = true;
  uint8_t ret;

  if (display_mess)  {
     Serial.println(mess);
     display_mess = false;
  }

  while (Serial.available()) {

    keyb[cnt] = Serial.read();

    // line complete or buffer full
    if (keyb[cnt] == '\n' || keyb[cnt] == '\r' || cnt == KEYB_BUF) {
      keyb[cnt] = 0x0;     // terminate
      ret = cnt;
      cnt = 0;             // reset pointer
      display_mess = true; // print message next time
      return(ret);
    }

    cnt++;
  }

  return(0);           // no complete buffer
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
 *  @brief : display SPS30 error message
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
