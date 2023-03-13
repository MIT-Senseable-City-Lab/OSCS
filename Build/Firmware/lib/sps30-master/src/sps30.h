/**
 * sps-30 Library Header file
 *
 * Copyright (c) January 2019, Paul van Haastrecht
 *
 * All rights reserved.
 * Will work with either UART or I2c communication.
 * The I2C link has a number of restrictions. See detailed document
 *
 * Development environment specifics:
 * Arduino IDE 1.8.12 and 1.8.13
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 * Version 1.0   / January 2019
 * - Initial version by paulvha
 *
 * Version 1.2   / January 2019
 * - added force serial1 when TX = RX = 8
 * - added flag  INCLUDE_SOFTWARE_SERIAL to exclude software Serial
 *
 * version 1.2.1 / February 2019
 * - added flag in sps30.h SOFTI2C_ESP32 to use SoftWire on ESP32 in case of SCD30 and SPS30 working on I2C
 *
 * version 1.3.0 / February 2019
 * - added check on the I2C receive buffer. If at least 64 bytes it try to read ALL information else only MASS results
 * - added || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) for small footprint
 *
 * version 1.3.1 / April 2019
 * - corrected bool stop() {return(Instruct(SER_STOP_MEASUREMENT));}
 *
 * version 1.3.2 / May 2019
 * - added support to detect SAMD I2C buffer size
 *
 * Version 1.3.6 / October 2019
 * - fixed I2C_Max_bytes () error when I2C is excluded
 * - improve receive buffer checks larger than 3 bytes
 *
 * Version 1.3.7 / December 2019
 *  - fixed ESP32 serial connection / flushing
 *
 * version 1.3.8 / January 2020
 *  - optimized the fix from October 2019 for I2C max bytes
 *
 * version 1.3.9 / February 2020
 *  - optimized autodetection for SAMD SERCOM and ESP32 to undef softwareSerial
 *  - removed to typedef warnings
 *
 * version 1.3.10 / April 2020
 *  - changed debug message handling
 *  - added DEBUGSERIAL to define the Serial port for messages
 *  - some typo's and cosmetic update
 *  - still backward compatible with earlier sketches
 *
 * version 1.4  / April 2020
 *  - Based on the new SPS30 datasheet (March 2020) a number of functions
 *    are added or updated. Some are depending on the new firmware.
 *  - Added sleep() and wakeup(). Requires firmware 2.0
 *  - Added GetVersion() to obtain the current firmware / hardware info
 *  - Added GetStatusReg() to obtain SPS30 status information. Requires firmware 2.2
 *  - Added structure SPS30_version for GetVersion
 *  - Added internal function to check on correct firmware level
 *  - Added INCLUDE_FWCHECK in SPS30.h to enable /disable check.
 *  - Changed probe() to obtain firmware levels instead of serial number.
 *  - Changed on how to obtaining product-type
 *  - Depreciated GetArticleCode(). Still supporting backward compatibility
 *  - Update the example sketches to include version levels
 *  - Added example11 for sleep(), wakeup() and GetStatusreg()
 *  - Update to documentation
 *  - Added the new datasheet in extras-folder
 *
 * version 1.4.1  / May 2020
 *  - fixed issue in setOpmode() when NO UART is available.
 *  - added setOpmode() to exclude in small footprint
 *
 * version 1.4.2  / May 2020
 *  - added NANO 33 IOT board  = SAMD21G18A (addition from Firepoo)
 *  - added option to select in sketch any serial or wire channel to use (many user requests)
 *  - added example12 and example13 sketches to demonstrate any channel selection option
 *
 * version 1.4.3 / June 2020
 *  - update to I2C_WAKEUP code
 *
 * version 1.4.4 / July 2020
 *  - added embedded support for Arduino Due
 *  - As I now have a SPS30 firmware level 2.2 to test, corrected GetStatusReg() and SetOpMode()
 *  - changed Example11 to demonstrate reading status register only
 *  - added Example14 to demonstrate sleep and wakeup function.
 *
 * version 1.4.5 / August 2020
 *  - added example20 for connecting multiple SPS30 (5!) to single board
 *  - updated sps30.odt around multiple SPS30 connected to Mega2560, DUE and ESP32
 *
 * version 1.4.6 / September 2020
 *  - corrected return code in instruct()
 *
 * version 1.4.7 / September 2020
 *  - corrected another return code in instruct()
 *
 * version 1.4.8 / October 2020
 *  - added support for Artemis / Apollo3 for SoftwareSerial detection
 *  - added check on return code in GetStatusReg()
 *  - added setClock() for I2C as the Artemis/Apollo3 is standard 400K
 *  - added flushing in case of Checkzero() (problem in Artemis)
 *
 * version 1.4.9 / December 2020
 *  - autodetection for Nano BLE 33 to undef softwareSerial
 *
 * version 1.4.10 / February 2021
 *  - Fixed typos in autodetection for Nano BLE 33 / Apollo3 for SoftwareSerial detection
 *
 * version 1.4.11 / July 2021
 *  - fixed error handling in Getvalues()
 *
 * version 1.4.15 / January 2023
 *  - autodetection for Nano MBED i2C size (needed for NANO BLE 33 and nRF52480)
 *
 *********************************************************************
*/
#ifndef SPS30_H
#define SPS30_H


/**
 * library version levels
 */
#define DRIVER_MAJOR 1
#define DRIVER_MINOR 4

/**
 * select debug serial (1.3.10)
 */
#define SPS30_DEBUGSERIAL Serial            // default

#if defined(ARDUINO_SODAQ_AUTONOMO) || defined(ARDUINO_SODAQ_SARA) || defined(ARDUINO_SODAQ_SFF)
#define SPS30_DEBUGSERIAL_SODAQ SerialUSB
#endif

enum debug_serial {
    STANDARD = 0,                           // default
#ifdef SPS30_DEBUGSERIAL_SODAQ
    SODAQ = 1
#endif
};

/**
 * ADDED version 1.4
 * New firmware levels have been slipped streamed into the SPS30
 * The datasheet from March 2020 shows added / updated functions on new
 * firmware level. E.g. sleep(), wakeup(), status register are new
 *
 * On serial connection the new functions are accepted and positive
 * acknowledged on lower level firmware, but execution does not seem
 * to happen or should be expected.
 *
 * On I2C reading Status register gives an error on lower level firmware.
 * Sleep and wakeup are accepted and positive acknowledged on lower level
 * firmware, but execution does not seem to happen or should be expected.
 *
 * Starting version 1.4 of this driver a firmware level check has been implemented
 * and in case a function is called that requires a higher level than
 * on the current SPS30, it will return an error.
 * By setting INCLUDE_FWCHECK to 0, this check can be disabled
 */
#define INCLUDE_FWCHECK 1

/**
 * To EXCLUDE I2C communication, maybe for resource reasons,
 * comment out the line below.
 */
#define INCLUDE_I2C   1

/**
 * To EXCLUDE the serial communication, maybe for resource reasons
 * as your board does not have a seperate serial, comment out the line below
 * It will also exclude Software_serial
 */
#define INCLUDE_UART 1

/**
 * On some IDE / boards software Serial is not available
 * comment out line below in that case
 * 1.3.9 : Autodetection for SAMD SERCOM and ESP32 to undef softwareSerial
 */
#define INCLUDE_SOFTWARE_SERIAL 1

/**
 * If the platform is an ESP32 AND it is planned to connect an SCD30 as well,
 * you have to remove the comments from the line below
 *
 * The standard I2C on an ESP32 does NOT support clock stretching
 * which is needed for the SCD30. You must have SCD30 library downloaded
 * from https://github.com/paulvha/scd30 and included in your sketch
 * (see examples)
 *
 * If you do not plan the SPS30 to run on I2C you can exclude the I2C in total
 */
//#define SOFTI2C_ESP32 1

#include <Arduino.h>                // Needed for Stream

/**
 *  Auto detect that some boards have low memory. (like Uno)
 */

#if defined (__AVR_ATmega328__) || defined(__AVR_ATmega328P__)|| defined(__AVR_ATmega16U4__) || (__AVR_ATmega32U4__)
    #define SMALLFOOTPRINT 1

    #if defined INCLUDE_UART
        #undef INCLUDE_UART
    #endif //INCLUDE_UART

#endif // AVR definition check

#if defined INCLUDE_I2C

    #if defined SOFTI2C_ESP32       // in case of use in combination with SCD30
        #include <SoftWire/SoftWire.h>
    #else
        #include "Wire.h"           // for I2c
    #endif

    /** Version 1.3.0
     *
     * The read results is depending on the Wire / I2c buffer size, defined in Wire.h.
     *
     * The buffer size needed for each float value is 6 (LSB + MSB + CRC ++++ LSB + MSB + CRC)
     * To read all values an I2C buffer of atleast 6 x 10 = 60 bytes is needed
     * On many boards the default buffer size is set to 32 in Wire.h, thus providing 5 valid float values.
     * You can increase (if memory size allows) that yourself in Wire.h
     *
     * Here we determine the buffersize and the calculation is done in the constructor for sps30
     * IF the buffer size is less than 64 only the MASS values are provided. This is for I2C only!!
     *
     * From a sketch you can check the impact by calling I2C_expect(), which will return the number of valid float values.
     */

    #define I2C_LENGTH 32

    #if defined BUFFER_LENGTH           // Arduino  & ESP8266 & Softwire
        #undef  I2C_LENGTH
        #define I2C_LENGTH  BUFFER_LENGTH
    #endif

    #if defined I2C_BUFFER_LENGTH       // ESP32
        #undef  I2C_LENGTH
        #define I2C_LENGTH  I2C_BUFFER_LENGTH
    #endif

    /* version 1.3.2 added support for SAMD SERCOM detection */
    /* version 1.4.8 autodetection for Apollo3 */
    /* version 1.4.15 autdetection for MBED Nano (Micromod nRF52840 */

    // Depending on definition in wire.h (RingBufferN<256> rxBuffer;)
    #if defined ARDUINO_ARCH_SAMD || defined ARDUINO_ARCH_SAM21D || defined ARDUINO_ARCH_APOLLO3 || defined ARDUINO_ARCH_MBED_NANO
        #undef  I2C_LENGTH
        #define I2C_LENGTH  256
    #endif

#endif // INCLUDE_I2C

#if defined INCLUDE_UART

    #if defined INCLUDE_SOFTWARE_SERIAL

      /* version 1.3.2 added support for SAMD SERCOM detection */
      /* version 1.3.9 autodetection for SAMD SERCOM and ESP32 to undef softwareSerial */
      /* version 1.4.4 autodetection for Arduino DUE to undef softwareSerial */
      /* version 1.4.8 autodetection for Apollo3 to undef softwareSerial */
      /* version 1.4.9 autodetection for Nano BLE 33 to undef softwareSerial */

      #if defined ARDUINO_ARCH_SAMD || defined ARDUINO_ARCH_SAM21D || defined ARDUINO_ARCH_ESP32 || defined ARDUINO_SAM_DUE || defined ARDUINO_ARCH_APOLLO3 ||defined ARDUINO_ARCH_NRF52840
        #undef  INCLUDE_SOFTWARE_SERIAL
      #else
         //#include <Software/Serial.h>        // softserial
      #endif // not defined ARDUINO_ARCH_SAMD & ESP32 & DUE & Apollo3
    #endif // INCLUDE_SOFTWARE_SERIAL

#endif // INCLUDE_UART

/**
 *  The communication it can be :
 *   I2C_COMMS              use I2C communication
 *   SOFTWARE_SERIAL        Arduino variants and ESP8266 (On ESP32 software Serial is NOT very stable)
 *   SERIALPORT             ONLY IF there is NO monitor attached
 *   SERIALPORT1            Arduino MEGA2560, 32U4, Sparkfun ESP32 Thing : MUST define new pins as defaults are used for flash memory)
 *   SERIALPORT2            Arduino MEGA2560, Due and ESP32 (NOT ESP32C3 + Espressif version 5.0.0 and higher)
 *   SERIALPORT3            Arduino MEGA2560 and Due only for now
 *   NONE                   No port defined
 *
 * Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on most devices.
 */

enum serial_port {
    I2C_COMMS = 0,
    SOFTWARE_SERIAL = 1,
    SERIALPORT = 2,
    SERIALPORT1 = 3,
    SERIALPORT2 = 4,
    SERIALPORT3 = 5,
    COMM_TYPE_SERIAL = 6,      // added 1.4.2
    NONE = 6
};

/* structure to return all values */
struct sps_values {
    float   MassPM1;        // Mass Concentration PM1.0 [μg/m3]
    float   MassPM2;        // Mass Concentration PM2.5 [μg/m3]
    float   MassPM4;        // Mass Concentration PM4.0 [μg/m3]
    float   MassPM10;       // Mass Concentration PM10 [μg/m3]
    float   NumPM0;         // Number Concentration PM0.5 [#/cm3]
    float   NumPM1;         // Number Concentration PM1.0 [#/cm3]
    float   NumPM2;         // Number Concentration PM2.5 [#/cm3]
    float   NumPM4;         // Number Concentration PM4.0 [#/cm3]
    float   NumPM10;        // Number Concentration PM4.0 [#/cm3]
    float   PartSize;       // Typical Particle Size [μm]
};

/* used to get single value */
#define v_MassPM1 1
#define v_MassPM2 2
#define v_MassPM4 3
#define v_MassPM10 4
#define v_NumPM0 5
#define v_NumPM1 6
#define v_NumPM2 7
#define v_NumPM4 8
#define v_NumPM10 9
#define v_PartSize 10

/* needed for conversion float IEE754 */
typedef union {
    byte array[4];
    float value;
} ByteToFloat;

/* needed for auto interval timing */
typedef union {
    byte array[4];
    uint32_t value;
} ByteToU32;

/*************************************************************/
/* error codes */
#define SPS30_ERR_OK          0x00
#define SPS30_ERR_DATALENGTH  0X01
#define SPS30_ERR_UNKNOWNCMD  0x02
#define SPS30_ERR_ACCESSRIGHT 0x03
#define SPS30_ERR_PARAMETER   0x04
#define SPS30_ERR_OUTOFRANGE  0x28
#define SPS30_ERR_CMDSTATE    0x43
#define SPS30_ERR_TIMEOUT     0x50
#define SPS30_ERR_PROTOCOL    0x51
#define SPS30_ERR_FIRMWARE    0x88        // added version 1.4

/* Receive buffer length. Expected is 40 bytes max
 * but you never know in the future.. */
#if defined SMALLFOOTPRINT
#define MAXRECVBUFLENGTH 50         // for light boards

#else
#define MAXRECVBUFLENGTH 128

struct Description {
    uint8_t code;
    char    desc[80];
};
#endif

/**
 * added version 1.4
 *
 * New call was explained to obtain the version levels
 * datasheet SPS30 March 2020, page 14
 *
 */
struct SPS30_version {
    uint8_t major;                  // Firmware level
    uint8_t minor;
    uint8_t HW_version;             // zero on I2C
    uint8_t SHDLC_major;            // zero on I2C
    uint8_t SHDLC_minor;            // zero on I2C
    uint8_t DRV_major;
    uint8_t DRV_minor;
};

/**
 * added version 1.4
 *
 * Status register result
 *
 * REQUIRES FIRMWARE LEVEL 2.2
 */
enum SPS_status {
    STATUS_OK = 0,
    STATUS_SPEED_ERROR = 1,
    STATUS_LASER_ERROR = 2,
    STATUS_FAN_ERROR = 4
};

/**
 * added version 1.4
 *
 * Measurement can be done in FLOAR or unsigned 16bits
 * page 6 datasheet SPS30 page 6.
 *
 * This driver only uses float
 */
#define START_MEASURE_FLOAT         0X03
#define START_MEASURE_UNS16         0X05

/*************************************************************/
/* SERIAL COMMUNICATION INFORMATION */
#define SER_START_MEASUREMENT       0x00
#define SER_STOP_MEASUREMENT        0x01
#define SER_READ_MEASURED_VALUE     0x03
#define SER_SLEEP                   0x10        // added 1.4
#define SER_WAKEUP                  0x11        // added 1.4
#define SER_START_FAN_CLEANING      0x56
#define SER_RESET                   0xD3

#define SER_AUTO_CLEANING_INTERVAL  0x80    // Generic autoclean request
#define SER_READ_AUTO_CLEANING          0x81    // read autoclean
#define SER_WRITE_AUTO_CLEANING         0x82    // write autoclean

#define SER_READ_DEVICE_INFO        0xD0    // GENERIC device request
#define SER_READ_DEVICE_PRODUCT_TYPE    0xF0     // CHANGED 1.4
#define SER_READ_DEVICE_RESERVED1       0xF1     // CHANGED 1.4
#define SER_READ_DEVICE_RESERVED2       0xF2     // CHANGED 1.4
#define SER_READ_DEVICE_SERIAL_NUMBER   0xF3

#define SER_READ_VERSION            0xD1         // Added 1.4
#define SER_READ_STATUS             0xD2         // Added 1.4

#define SHDLC_IND   0x7e                   // header & trailer
#define TIME_OUT    5000                   // timeout to prevent deadlock read
#define RX_DELAY_MS 100                    // wait between write and read

/*************************************************************/
/* I2C COMMUNICATION INFORMATION  */
#define I2C_START_MEASUREMENT       0x0010
#define I2C_STOP_MEASUREMENT        0x0104
#define I2C_READ_DATA_RDY_FLAG      0x0202
#define I2C_READ_MEASURED_VALUE     0x0300
#define I2C_SLEEP                   0X1001  // ADDED 1.4
#define I2C_WAKEUP                  0X1103  // ADDED 1.4 / update 1.4.3
#define I2C_START_FAN_CLEANING      0x5607
#define I2C_AUTO_CLEANING_INTERVAL  0x8004
#define I2C_SET_AUTO_CLEANING_INTERVAL      0x8005

#define I2C_READ_PRODUCT_TYPE       0xD002 // CHANGED 1.4
#define I2C_READ_SERIAL_NUMBER      0xD033
#define I2C_READ_VERSION            0xD100 // ADDED 1.4
#define I2C_READ_STATUS_REGISTER    0xD206 // ADDED 1.4
#define I2C_CLEAR_STATUS_REGISTER   0xD210 // ADDED 1.4 / update 1.4.4
#define I2C_RESET                   0xD304

#define SPS30_ADDRESS 0x69                 // I2c address
/***************************************************************/

class SPS30
{
  public:

    SPS30(void);

    /**
    * @brief  Enable or disable the printing of sent/response HEX values.
    *
    * @param act : level of debug to set
    *  0 : no debug message
    *  1 : sending and receiving data
    *  2 : 1 +  protocol progress
    *
    * @param SelectDebugSerial : select Serial port (see top of SPS30.h)
    *  This will allow to select a different port than Serial for debug
    *  messages. As real example an SODAQ NB board is using SerialUSB.
    */
    void EnableDebugging(uint8_t act, debug_serial SelectDebugSerial = STANDARD);

    /**
     * @brief Initialize the communication port
     *
     * @param port : communication channel to be used (see sps30.h)
     */
    bool begin(serial_port port = SERIALPORT2); // If user doesn't specify Serial2 will be used

    /**
     * @brief Manual assigment of the serial communication port  added 1.4.2
     *
     * @param serialPort: serial communication port to use
     *
     * User must have preformed the serialPort.begin(115200) in the sketch.
     */
    bool begin(Stream *serialPort);
    bool begin(Stream &serialPort);

    /**
     * @brief Manual assigment I2C communication port added 1.4.2
     *
     * @param port : I2C communication channel to be used
     *
     * User must have preformed the wirePort.begin() in the sketch.
     */
    bool begin(TwoWire *wirePort);

    /**
     * @brief : Perform SPS-30 instructions
     */
    bool probe();
    bool reset() {return(Instruct(SER_RESET));}
    bool start() {return(Instruct(SER_START_MEASUREMENT));}
    bool stop()  {return(Instruct(SER_STOP_MEASUREMENT));}
    bool clean() {return(Instruct(SER_START_FAN_CLEANING));}

    /**
     * Added 1.4
     * @brief Set SPS30 to sleep or wakeup
     * Requires Firmwarelevel 2.0
     */
    uint8_t sleep() {return(SetOpMode(SER_SLEEP));}
    uint8_t wakeup(){return(SetOpMode(SER_WAKEUP));}

    /**
     * @brief : Set or get Auto Clean interval
     */
    uint8_t GetAutoCleanInt(uint32_t *val);
    uint8_t SetAutoCleanInt(uint32_t val);

    /**
     * @brief : retrieve Error message details
     */
    void GetErrDescription(uint8_t code, char *buf, int len);

    /**
     * @brief : retrieve device information from the SPS-30
     *
     * On none of the device so far Article code and Product name are
     * available.
     */
    uint8_t GetSerialNumber(char *ser, uint8_t len) {return(Get_Device_info( SER_READ_DEVICE_SERIAL_NUMBER, ser, len));}
    uint8_t GetProductName(char *ser, uint8_t len)  {return(Get_Device_info(SER_READ_DEVICE_PRODUCT_TYPE, ser, len));}      // CHANGED 1.4

    /**
     * CHANGED 1.4
     * Depreciated in Datasheet March 2020
     * left for backward compatibility with older sketches
     */
    uint8_t GetArticleCode(char *ser, uint8_t len)  {ser[0] = 0x0; return SPS30_ERR_OK;}

    /** ADDED 1.4
     * @brief : retrieve software/hardware version information from the SPS-30
     *
     */
    uint8_t GetVersion(SPS30_version *v);

    /** ADDED 1.4
     * @brief : Read Device Status from the SPS-30
     *
     * REQUIRES FIRMWARE 2.2
     * The commands are accepted and positive acknowledged on lower level
     * firmware, but do not execute.
     *
     * @param  *status
     *  return status as an 'or':
     *   STATUS_OK = 0,
     *   STATUS_SPEED_ERROR = 1,
     *   STATUS_SPEED_CURRENT_ERROR = 2,
     *   STATUS_FAN_ERROR = 4
     *
     * @return
     *  SPS30_ERR_OK = ok, no isues found
     *  else SPS30_ERR_OUTOFRANGE, issues found
     */
    uint8_t GetStatusReg(uint8_t *status);

    /**
     * @brief : retrieve all measurement values from SPS-30
     */
    uint8_t GetValues(struct sps_values *v);

    /**
     * @brief : retrieve a specific value from the SPS-30
     */
    float GetMassPM1()  {return(Get_Single_Value(v_MassPM1));}
    float GetMassPM2()  {return(Get_Single_Value(v_MassPM2));}
    float GetMassPM4()  {return(Get_Single_Value(v_MassPM4));}
    float GetMassPM10() {return(Get_Single_Value(v_MassPM10));}
    float GetNumPM0()   {return(Get_Single_Value(v_NumPM0));}
    float GetNumPM1()   {return(Get_Single_Value(v_NumPM1));}
    float GetNumPM2()   {return(Get_Single_Value(v_NumPM2));}
    float GetNumPM4()   {return(Get_Single_Value(v_NumPM4));}
    float GetNumPM10()  {return(Get_Single_Value(v_NumPM10));}
    float GetPartSize() {return(Get_Single_Value(v_PartSize));}

   /**
     * @brief : set RX and TX pin for softserial and Serial1 on ESP32
     * Setting both to 8 (tx=rx=8) will force a Serial1 communication
     * on any device (assuming the pins are hard coded)
     */
    void SetSerialPin(uint8_t rx, uint8_t tx);

#if defined INCLUDE_I2C
    /**
     * @brief : Return the expected number of valid values read from device
     *
     * This is depending on the buffer defined in Wire.h
     *
     * Return
     *  4 = Valid Mass values only
     * 10 = All values are expected to be valid
     */
    uint8_t I2C_expect();
#else
    uint8_t I2C_expect() {return 0;}
#endif

  private:
    void DebugPrintf(const char *pcFmt, ...);

    /** shared variables */
    uint8_t _Receive_BUF[MAXRECVBUFLENGTH]; // buffers
    uint8_t _Send_BUF[10];
    uint8_t _Receive_BUF_Length;
    uint8_t _Send_BUF_Length;

    serial_port _Sensor_Comms;          // communication channel to use
    int _SPS30_Debug;                   // program debug level
    debug_serial _SPS30_Debug_Serial;   // serial debug-port to use
    bool _started;                      // indicate the measurement has started
    bool _sleep;                        // indicate that SPS30 is in sleep (added 1.4)
    bool _WasStarted;                   // restart if SPS30 was started before setting sleep (added 1.4)
    uint8_t Reported[11];               // use as cache indicator single value
    uint8_t _I2C_Max_bytes;
    uint8_t Serial_RX = 0, Serial_TX = 0; // softserial or Serial1 on ESP32
    uint8_t _FW_Major, _FW_Minor;       // holds firmware major (added 1.4)

    /** shared supporting routines */
    uint8_t Get_Device_info(uint8_t type, char *ser, uint8_t len);
    bool Instruct(uint8_t type);
    uint8_t SetOpMode(uint8_t mode);            // added 1.4
    bool FWCheck(uint8_t major, uint8_t minor); // added 1.4
    float byte_to_float(int x);
    uint32_t byte_to_U32(int x);
    float Get_Single_Value(uint8_t value);

#if defined INCLUDE_UART
    /** UART / serial related */
    // calls
    bool setSerialSpeed();
    uint8_t ReadFromSerial();
    uint8_t SerialToBuffer();
    uint8_t SendToSerial();
    bool SHDLC_fill_buffer(uint8_t command, uint32_t parameter = 0);
    uint8_t SHDLC_calc_CRC(uint8_t * buf, uint8_t first, uint8_t last);
    int ByteStuff(uint8_t b, int off);
    uint8_t ByteUnStuff(uint8_t b);

    // variables
    Stream *_serial;        // serial port to use
#endif // INCLUDE_UART


#if defined INCLUDE_I2C
    /** I2C communication */
    TwoWire *_i2cPort;      // holds the I2C port
    void I2C_init();
    void I2C_fill_buffer(uint16_t cmd, uint32_t interval = 0);
    uint8_t I2C_ReadToBuffer(uint8_t count, bool chk_zero);
    uint8_t I2C_SetPointer_Read(uint8_t cnt, bool chk_zero = false);
    uint8_t I2C_SetPointer();
    bool I2C_Check_data_ready();
    uint8_t I2C_calc_CRC(uint8_t data[2]);

#endif // INCLUDE_I2C

};
#endif /* SPS30_H */
