/*
Qiyun "Q" Gao - Sensable City Lab, MIT - Feb 20, 2021
Driver for TI BQ27200 Single-Cell Battery Gas Gauge
Reference https://www.ti.com/tool/BQ27XXXSW-LINUX
Reference https://www.ti.com/lit/ds/symlink/bq27200.pdf
*/
#include "BQ27200_I2C.h"

#define BQ27200_ADDR 0x55
#define R_SERIES 20 // mOhm (measured)
#define MILLIAMP_LSB   0.1785
#define MILLIWATT_LSB  1.4600

static uint8_t BQ27200_REG_ADDR[25] =  {[BQ27200_CTRL]  = 0x00,
                                        [BQ27200_MODE]  = 0x01,
                                        [BQ27200_AR]    = 0x02,
                                        [BQ27200_ARTTE] = 0x04,
                                        [BQ27200_TEMP]  = 0x06,
                                        [BQ27200_VOLT]  = 0x08,
                                        [BQ27200_FLAGS] = 0x0A,
                                        [BQ27200_RSOC]  = 0x0B,
                                        [BQ27200_NAC]   = 0x0C,
                                        [BQ27200_CACD]  = 0x0E,
                                        [BQ27200_CACT]  = 0x10,
                                        [BQ27200_LMD]   = 0x12,
                                        [BQ27200_AI]    = 0x14,
                                        [BQ27200_TTE]   = 0x16,
                                        [BQ27200_TTF]   = 0x18,
                                        [BQ27200_SI]    = 0x1A,
                                        [BQ27200_STTE]  = 0x1C,
                                        [BQ27200_MLI]   = 0x1E,
                                        [BQ27200_MLTTE] = 0x20,
                                        [BQ27200_SAE]   = 0x22,
                                        [BQ27200_AP]    = 0x24,
                                        [BQ27200_TTECP] = 0x26,
                                        [BQ27200_CYCL]  = 0x28,
                                        [BQ27200_CYCT]  = 0x2A,
                                        [BQ27200_CSOC]  = 0x2C,
                                       };

static uint8_t BQ27200_REG_SIZE[25] =  {[BQ27200_CTRL]  = 1,
                                        [BQ27200_MODE]  = 1,
                                        [BQ27200_AR]    = 2,
                                        [BQ27200_ARTTE] = 2,
                                        [BQ27200_TEMP]  = 2,
                                        [BQ27200_VOLT]  = 2,
                                        [BQ27200_FLAGS] = 1,
                                        [BQ27200_RSOC]  = 1,
                                        [BQ27200_NAC]   = 2,
                                        [BQ27200_CACD]  = 2,
                                        [BQ27200_CACT]  = 2,
                                        [BQ27200_LMD]   = 2,
                                        [BQ27200_AI]    = 2,
                                        [BQ27200_TTE]   = 2,
                                        [BQ27200_TTF]   = 2,
                                        [BQ27200_SI]    = 2,
                                        [BQ27200_STTE]  = 2,
                                        [BQ27200_MLI]   = 2,
                                        [BQ27200_MLTTE] = 2,
                                        [BQ27200_SAE]   = 2,
                                        [BQ27200_AP]    = 2,
                                        [BQ27200_TTECP] = 2,
                                        [BQ27200_CYCL]  = 2,
                                        [BQ27200_CYCT]  = 2,
                                        [BQ27200_CSOC]  = 1,
                                       };

BQ27200_I2C::BQ27200_I2C(bool use_soft_i2c) {
  _use_softwire = use_soft_i2c;
}

int BQ27200_I2C::read_reg(uint8_t reg_addr, uint8_t reg_size) {
  Wire.beginTransmission(BQ27200_ADDR);
  Wire.write(reg_addr);
  Wire.endTransmission(false);
  uint8_t nbytes = Wire.requestFrom(BQ27200_ADDR, reg_size, 1);
  if (nbytes != reg_size) {
    while (Wire.available()) {
      Wire.read();
    }
    return (-1);
  }
  int ret = 0;
  for (byte i = 0; i < reg_size; i++) {
    int data = Wire.read();
    if (data < 0){
      return (-1);
    }
    ret = ret + (data << (8*i));
  }
  return (ret);
}

int BQ27200_I2C::read_reg_soft(uint8_t reg_addr, uint8_t reg_size){
  uint8_t write_addr = (BQ27200_ADDR << 1); // softwire library use 8bit 'raw' addressing 
  uint8_t read_addr = write_addr + 1;
  softwire.begin();
  softwire.i2c_start();
  softwire.i2c_write(write_addr);
  softwire.i2c_write(reg_addr);
  softwire.i2c_restart();
  softwire.i2c_write(read_addr);
  int ret = 0;
  if (reg_size == 1){
    ret = softwire.i2c_read(1);
  } else if (reg_size == 2){
    ret = softwire.i2c_read(0);
    ret += (softwire.i2c_read(1)) * 256;
  } else {
    ret = -1;
  }
  softwire.i2c_stop();
  return (ret);
}

int  BQ27200_I2C::read_data(uint8_t index) {
  if (_use_softwire){
    return read_reg_soft(BQ27200_REG_ADDR[index], BQ27200_REG_SIZE[index]);
  }
  return read_reg(BQ27200_REG_ADDR[index], BQ27200_REG_SIZE[index]);
}
bool BQ27200_I2C::get_flag_bit(uint8_t pos, uint8_t flags) {
  return ((flags >> pos) & 0b1);
}

bool BQ27200_I2C::isCharging() {
  return (get_flag_bit(BQ27200_CHGS, read_data(BQ27200_FLAGS)));
}
bool BQ27200_I2C::isIdle() {
  return (get_flag_bit(BQ27200_NOACT, read_data(BQ27200_FLAGS)));
}
bool BQ27200_I2C::isCharged() {
  return (get_flag_bit(BQ27200_IMIN, read_data(BQ27200_FLAGS)));
}
bool BQ27200_I2C::capacityInaccurate() {
  return (get_flag_bit(BQ27200_CI, read_data(BQ27200_FLAGS)));
}
bool BQ27200_I2C::isLow() {
  return (get_flag_bit(BQ27200_EDV1, read_data(BQ27200_FLAGS)));
}
bool BQ27200_I2C::isEmpty() {
  return (get_flag_bit(BQ27200_EDVF, read_data(BQ27200_FLAGS)));
}

int BQ27200_I2C::current_raw(uint8_t arg) {
  // argument     returns
  // 0            5.12s average
  // 1            standby current
  // 2            max load current
  // 3            custom discharge rate AR used in ARTTE calculation
  switch (arg) {
    case 0:      return ((1 - isCharging() * 2) * read_data(BQ27200_AI));
    case 1:      return (read_data(BQ27200_SI));
    case 2:      return (read_data(BQ27200_MLI));
    case 3:      return (read_data(BQ27200_AR));
    default:     return (-1);
  }
}
float BQ27200_I2C::current(uint8_t arg) {
  return (current_raw(arg) * MILLIAMP_LSB);
}
int BQ27200_I2C::temperature_raw() {
  return (read_data(BQ27200_TEMP));
}
float BQ27200_I2C::temperature() {
  return (temperature_raw() * 0.25 - 273.15);
}
int BQ27200_I2C::voltage_raw() {
  return (read_data(BQ27200_VOLT));
}
float BQ27200_I2C::voltage() {
  return (voltage_raw() * 1.00);
}
int BQ27200_I2C::power_raw() {
  return (read_data(BQ27200_AP));
}
float BQ27200_I2C::power() {
  return (power_raw() * MILLIWATT_LSB);
}
int BQ27200_I2C::available_energy_raw() {
  return (read_data(BQ27200_SAE));
}
float BQ27200_I2C::available_energy() {
  return (available_energy_raw() * MILLIWATT_LSB);
}
int BQ27200_I2C::time_to_empty_raw(uint8_t arg) {
  // argument     returns
  // 0            at constant current as measured now
  // 1            at standby current
  // 2            at measured max load current
  // 3            at custom discharge current (AR)
  // 4            at constant power as measured now
  switch (arg) {
    case 0:      return (read_data(BQ27200_TTE));
    case 1:      return (read_data(BQ27200_STTE));
    case 2:      return (read_data(BQ27200_MLTTE));
    case 3:      return (read_data(BQ27200_ARTTE));
    case 4:      return (read_data(BQ27200_TTECP));
    default:     return (-1);
  }
}
float BQ27200_I2C::time_to_empty(uint8_t arg) {
  return (time_to_empty_raw(arg) * 1.00);
}
int BQ27200_I2C::time_to_full_raw() {
  return (read_data(BQ27200_TTF));
}
float BQ27200_I2C::time_to_full() {
  return (time_to_full_raw() * 1.00);
}
int BQ27200_I2C::last_measured_capacity_raw() {
  return (read_data(BQ27200_LMD));
}
float BQ27200_I2C::last_measured_capacity() {
  return (last_measured_capacity_raw() * MILLIAMP_LSB);
}
int BQ27200_I2C::available_capacity_raw(uint8_t arg) {
  // argument     returns
  // 0            Nominal Capacity
  // 1            Discharge Compensated
  // 2            Temperature Compensated
  switch (arg) {
    case 0:      return (read_data(BQ27200_NAC));
    case 1:      return (read_data(BQ27200_CACD));
    case 2:      return (read_data(BQ27200_CACT));
    default:     return (-1);
  }
}
float BQ27200_I2C::available_capacity(uint8_t arg) {
  return (available_capacity_raw(arg) * MILLIAMP_LSB);
}
int BQ27200_I2C::state_of_charge_raw(uint8_t arg) {
  // argument     returns
  // 0            Relative SOC (nominal capacity / last measured discharge)
  // 1            Compensated SOC
  switch (arg) {
    case 0:      return (read_data(BQ27200_RSOC));
    case 1:      return (read_data(BQ27200_CSOC));
    default:     return (-1);
  }
}
float BQ27200_I2C::state_of_charge(uint8_t arg) {
  return (state_of_charge_raw(arg) * 1.00);
}
int BQ27200_I2C::cycle_count(uint8_t arg) {
  // argument     returns
  // 0            Cycle count total
  // 1            Cycle count since learning cycle
  switch (arg) {
    case 0:      return (read_data(BQ27200_CYCT));
    case 1:      return (read_data(BQ27200_CYCL));
    default:     return (-1);
  }
}
