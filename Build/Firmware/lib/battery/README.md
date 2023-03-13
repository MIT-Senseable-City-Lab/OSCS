# cityscanner-batterylib-bq27
## Overview
Driver for GlobTek battery pack that contains a TI BQ27200 Gas Gauge chip. 

Chip datasheet: https://www.ti.com/lit/ds/symlink/bq27200.pdf

This library can read any register in BQ27200. Writing registers is currently not supported.

## Usage
### Setup
`BQ27200_I2C.h` and `BQ27200_I2C.cpp` are battery driver. `softi2c_scl.h` and `softi2c_scl.cpp` are software I2C driver used by the battery library.
`example.ino` shows how to initialize battery library using software I2C and hardware I2C. 

Constructor:
```
BQ27200_I2C bq = BQ27200_I2C(1); // use software I2C
BQ27200_I2C bq = BQ27200_I2C(0); // use hardware I2C
```

 - Use software I2C on Particle Boron since its hardware I2C does not support clock stretching which is used by BQ27200
 - Need to test if hardware I2C works on Particle Electron. However, software I2C should work on any platform. 
 - Don't forget to call `Wire.end()` to turn off hardware i2c before reading any data if you use software i2c. And turn hardware i2c back on when you are done! 
 
 ### Reading Data
 Note: `isCharging()` is used to calculate sign of current
 
 Function | Argument | Return type | Description | Unit |From
 ------   |------     |------ | ------ | ------ | -----
 `isCharging()` | None | boolean | 1 = charging | | Status Flag Register, 'CHGS'
 `isIdle()`     | None | boolean | 1 = idle     | | Status Flag Register, 'NOACT'
 `isCharged()`  | None | boolean | 1 = charged  | | Status Flag Register, 'IMIN'
 `capacityInaccurate()`  | None | boolean | 1 = bad  | | Status Flag Register, 'CI'
 `isLow()`  | None | boolean | 1 = SOC <= 6.25% | | Status Flag Register, 'EDV1'
 `isEmpty()` | None | Boolean | 1 = SOC < 0% | | Status Flag Register, 'EDVF'
 `current(arg)` | 0 (default) | float32 | 5.12s average current | mA | Register 'AIL' and 'AIH'
  | | 1 | float32 | standby current | mA | Register 'SIL' and 'SIH'
  | | 2 | float32 | max-load current | mA | Register 'MLIL' and 'MLIH'
  | | 3 | float32 | custom discharge rate | mA | Register 'ARL' and 'ARH'

# TO BE CONTINUED, PLEASE READ IN-LINE COMMENTS FOR NOW
