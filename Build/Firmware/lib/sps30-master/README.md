# Sensirion SPS30

## ===========================================================

A program to set instructions and get information from an SPS30. It has been
tested to run either UART or I2C communication on ESP32, MEGA2560, ESP8266, UNO, Due, nRF52840 and Apollo3.
In the meantime many other boards have been added to the test as well as extended
interface options. (see below)
<br> A detailed description of the options and findings are in SPS30.odt

## Getting Started
As part of a larger project I am looking at analyzing and understanding the air quality.
I have done a number of projects on air-sensors. This is a version of a working driver + examples.
More work continues to happen to create examples and compare against other sensors.

A word of warning: the SPS30 needs a female plug of ZHR-5 from JST Sales America Inc.
I have not been able to find a good source for that and was glad to buy the Sparkfun version
(https://www.sparkfun.com/products/15103) which does include a cable with this plug.

<br> May 2019 : there is also a library available for Raspberry Pi (https://github.com/paulvha/sps30_on_raspberry)
<br> January 2023 : there is also SPS30 BLE-peripheral, BLE-central and Android BLE-APP (https://github.com/paulvha/apollo3/tree/master/ArduinoBLE_special)

## Prerequisites
Examples 4, 5, 7, 8, 10, 15 and 16 have a dependency on other libraries. Documented in sketch.

## Software installation
Obtain the zip and install like any other

## Program usage
### Program options
Please see the description in the top of the sketch and read the documentation (odt)

### Communication channel selection
From the start I had decided to take an embedded communication channel setup. This made it
much easier for the user to select and have the right setup for Serial or I2C. It was
initially tested on an UNO, MEGA, ESP32, ESP8266, Due (1.4.4) and works well.
Overtime code has been adjusted to support more boards with different pin-outs based
on user request and feedback.
Given the large number of new boards that continue to hit the market, with different pin-outs,
as well as boards with multiple I2C channels, I have decided to add the option
for the user provide the communication channel also differently. This means the
user will perform in the sketch the initialization of the channel (serial or I2C) and
provide that to the SP30 library. Example12 (for serial communication) and Example13
(for I2C communication) has been added to demonstrate the usage.
The embedded approach, and thus backward compatibility, continue to be available.

## Versioning

### Version 1.4.16 / Janaury 2023
 * fixed compile error in the embedded approach as Serial2 is not defined by default for ESP32C3 over Espressif 5.0.0 and also over Espressif 6.0.0

### Version 1.4.15 / January 2023
 * autodetection added for Nano MBED i2C size (needed for NANO BLE 33 and nRF52480)

### version 1.4.14 / May 2022
 * changed ERR_xxx to SPS30_ERR_xxx due to conflict with other program

### version 1.4.13 / January 2022
 * Different corrections in SPS30.odt

### version 1.4.12 / October 2021
 * Updated example13 and example16 to allow lower I2C speed for stability

### version 1.4.11 / July 2021
 * Fixed error handling in Getvalues()

### version 1.4.10 / February 2021
 * Fixed typos in autodetection for Nano BLE 33 / Apollo3 for SoftwareSerial detection

### version 1.4.9 / October 2020
 * added example15 and Example16 to display the SPS30 output on an LCD

### version 1.4.8 / October 2020
 * added check on return code in GetStatusReg()
 * added support for Artemis / Apollo3
 * added setClock() for I2C as the Artemis/Apollo3 is standard 400K. SPS30 can handle up to 100K
 * added flushing in case of chk_zero() (handling a problem in Artemis library 2.0.1)

### version 1.4.7 / September 2020
 * corrected another return code in instruct() (Thanks for pointing out Robert R. Fenichel)

### version 1.4.6 / September 2020
 * corrected return code in instruct() (Thanks for pointing out Robert R. Fenichel)

### version 1.4.5 / August 2020
 * added example20 for connecting multiple SPS30 (5!) to single board
 * updated sps30.odt around multiple SPS30 connected to Mega2560, DUE and ESP32

### version 1.4.4 / July 2020
 * added embedded support for Arduino Due
 * as I now have a SPS30 firmware level 2.2 to test, corrected GetStatusReg() and SetOpMode()
 * changed Example11 to demonstrate reading status register only
 * added Example14 to demonstrate sleep and wakeup function.

### version 1.4.3 / June 2020
 * update to I2C_WAKEUP code

### version 1.4.2  / May 2020
 * added NANO 33 IOT board  = SAMD21G18A (addition from Firepoo)
 * added option to select in sketch any serial or wire channel to use (many user requests)
 * added example12 and example13 sketches to demonstrate any channel selection option

### version 1.4.1  / May 2020
 * Fixed issue in setOpmode() when NO UART is available, only I2C.
 * Added setOpmode() to exclude in small footprint

### version 1.4  / April 2020
 * Based on the new SPS30 datasheet (March 2020) a number of functions are added or updated. Some are depending on the new firmware.
 * Added sleep() and wakeup(). Requires firmware 2.0
 * Added GetVersion() to obtain the current firmware / hardware / library info
 * Added structure SPS30_version for GetVersion()
 * Added GetStatusReg() to obtain SPS30 status information. Requires firmware 2.2
 * Added internal function to check on correct firmware level
 * Added INCLUDE_FWCHECK in SPS30.h to enable /disable check.
 * Changed probe() to obtain firmware levels instead of serial number.
 * Changed on how to obtaining product-type
 * Depreciated GetArticleCode(). Still supporting backward compatibility
 * Update the example sketches to include version levels
 * Added example11 for sleep(), wakeup() and GetStatusreg()
 * Update to documentation
 * Added the new datasheet in extras-folder

### version 1.3.10 / April 2020
 * Updated examples for new compile errors and warnings wih IDE 1.8.12
 * support for SODAQ AFF/SARA board (examples on ![https://github.com/paulvha/sodaq]([https://github.com/paulvha/sodaq)
 * changed debug message handling
 * Added DEBUGSERIAL to define the Serial port for messages
 * fixed some typo's and cosmetic update
 * still fully backward compatible with earlier sketches
 * updated documentation

### version 1.3.9 / February 2020
 * optimized autodetection for SAMx1D SERCOM and ESP32 to undef softwareSerial

### version 1.3.8 / January 2020
 * optimized the fix from October 2019 for I2C max bytes

### version 1.3.7 / December 2019
 * fixed ESP32 serial connection / flushing

### version 1.3.6 / September 2019
 * fixed I2C_Max_bytes error when I2C is excluded in sps30.h
 * improve receive buffer checks larger than 3 bytes
 * A special version for Feather Lora 32U4 has been created ![https://github.com/paulvha/SPS30_lora](https://github.com/paulvha/SPS30_lora)

### version 1.3.5 / May 2019
 * added support for MKRZERO/SAMD I2C buffer detection and disable softerial
 * updated documentation for PROMINI I2C buffer adjustment in odt-file (thanks to input Bert Heusinkveld)

### version 1.3.4 / April 2019
 * corrected the stop measurement command (spotted by detamend)

### version 1.3.3 / March 2019
 * Added example 10 for ESP32 only: use SPS30 to create an airquality index by region
 * update documentation to 1.3

### version 1.3.2 / February 2019
 * Added example 9 (with compare typical size)
 * Updated the documentation with compare results to SDS011 and Dylos-1700
 * Update all examples to have prototypes upfront as the ESP32 pre-processor sometimes does not create

### version 1.3.1 / February 2019
 * fixed the PM10 number always showing 0 issue.

### version 1.3.0 / February 2019
 * Added check on the I2C receive buffer. If at least 64 bytes it try to read ALL information else only MASS results
 * Updated examples / documentation / instructions
 * Added example 8 (SPS30 + SCD30 + BME280)
 * Added || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) for boards with small footprint (thanks Just van den Broecke)
 * Although the __AVR_ATmega32U4__ has a UART, it does NOT support 115K and can only be connected over I2C.

### version 1.2.1 / February 2019
 * Added example 7 (with SCD)
 * Added flag in sps30.h SOFTI2C_ESP32 to use SoftWire on ESP32 in case of SCD30 and SPS30 working on I2C
 * Update documentation / instructions

### version 1.2  / January 2019
 * Added force serial1 when TX = RX = 8
 * Added flag INCLUDE_SOFTWARE_SERIAL to optionally exclude software Serial
 * Tested by Ryan Brown on a Sparkfun Photon RED board and the code should also work with the Photon, P1, and Electrons

### version 1.1.0 / January 2019
 * Added example 6 (plotting data)
 * Added ESP8266 support info

### version 1.0.1 / January 2019
 * Added examples 4 (with DS18x20) and 5 (with BME280)

### version 1.0 / January 2019
 * Initial version Arduino, ESP32, UNO

## Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

## Acknowledgments

### Make sure to read the datasheet from Sensirion, March 2020 version.<br>
### In case you are new to electronics and wonder about pull-up resistors for I2C, see below (thanks to Shane Diller)
![Uno and SP30](extras/sensirion.png)
<br>

### June 2021, Input from CCDZAPPER:

This is not an issue - just a note to help others that may have damaged or lost the flimsy cable with the ZHR-5 connector. It is available inexpensively on eBay: ![https://www.ebay.com/itm/114551266422](https://www.ebay.com/itm/114551266422)
Make sure you pick the correct type, which is 1.25mm 5 Pin.
(10 Sets JST SH 1.0 ZH 1.5 PH 2.0 XH 2.5 Housing Connector Female Male Wire)

### October 2021: Input from Urs Utzinger

Reducing the I2C speed to 50K instead of 100K improves longer time stability.

### October 2021 : Input from Nkea

I also found another compatible connector. I also recommend buying spare crimp contacts as they are very small and fragile. Needs a crimp tool. I use a Connector Pliers model PA-09 by Engineer I had from other works. It may be useful if special lengths are needed of there is no stock on JST ones

Housing:
https://www.mouser.es/ProductDetail/Wurth-Elektronik/648005113322?qs=%2Fha2pyFaduguH2zIpdkgUWxmzUvrTES979PXEupx7lQusLC5mK%2FQfQ%3D%3D

crimp contacts
https://www.mouser.es/ProductDetail/Wurth-Elektronik/64800113722DEC?qs=%2Fha2pyFaduguH2zIpdkgUUfd6dp6pTTujW8FuBzdSDO2pxvJN95p5w%3D%3D

