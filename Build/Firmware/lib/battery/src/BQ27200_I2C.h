#ifndef BQ27200_I2C_h
#define BQ27200_I2C_h

#include "Particle.h"
#include "softi2c_scl.h"
//                      NAME               UNIT       Calculated LSB Value
enum BQ27200_REG_INDEX {BQ27200_CTRL,   // BIT ARRAY  -
                        BQ27200_MODE,   // BIT ARRAY  -
                        BQ27200_AR,     // 3.57uV     0.1785 mA
                        BQ27200_ARTTE,  // Minutes    1 minute
                        BQ27200_TEMP,   // 0.25 K     0.25 K
                        BQ27200_VOLT,   // mV         1 mV
                        BQ27200_FLAGS,  // BIT ARRAY  -
                        BQ27200_RSOC,   // %          1 %
                        BQ27200_NAC,    // 3.57uVh    0.1785 mAh
                        BQ27200_CACD,   // 3.57uVh    0.1785 mAh
                        BQ27200_CACT,   // 3.57uVh    0.1785 mAh
                        BQ27200_LMD,    // 3.57uVh    0.1785 mAh
                        BQ27200_AI,     // 3.57uV     0.1785 mA
                        BQ27200_TTE,    // Minutes    1 minute
                        BQ27200_TTF,    // Minutes    1 minute
                        BQ27200_SI,     // 3.57uV     0.1785 mA
                        BQ27200_STTE,   // Minutes    1 minute
                        BQ27200_MLI,    // 3.57uV     0.1785 mA
                        BQ27200_MLTTE,  // Minutes    1 minute
                        BQ27200_SAE,    // 29.2uV^2h  1.46 mWh
                        BQ27200_AP,     // 29.2uV^2   1.46 mW
                        BQ27200_TTECP,  // Minutes    1 minute
                        BQ27200_CYCL,   // Cycles     1
                        BQ27200_CYCT,   // Cycles     1
                        BQ27200_CSOC,   // %          1 %
                       };

enum BQ27200_FLAGS_BIT_INDEX {BQ27200_EDVF,
                              BQ27200_EDV1,
                              BQ27200_VDQ,
                              BQ27200_CALIP,
                              BQ27200_CI,
                              BQ27200_IMIN,
                              BQ27200_NOACT,
                              BQ27200_CHGS,
                             };

// Write these to MODE register, then write 0xA9 or 0x56 to CTRL register to make BQ27200 do these commands
enum BQ27200_MODE_BIT_INDEX_A9 {BQ27200_SHIP,
                                BQ27200_FRST,
                                BQ27200_POR,
                                BQ27200_PRST,
                                BQ27200_DONE,
                                BQ27200_WRTNAC,
                                BQ27200_GPSTAT,
                                BQ27200_GPIEN,
                               };
enum BQ27200_MODE_BIT_INDEX_56 {BQ27200_CIO = 4,
                                BQ27200_CEO = 5,
                               };
// BQ27200 has no SHIP function
// DO NOT WRITE any of these MODE registers other than DONE
// unless we are re-calibrating the battery

class BQ27200_I2C {
  public:
    BQ27200_I2C(bool use_soft_i2c = 0);
    int read_reg(uint8_t reg_addr, uint8_t reg_size);
    int read_reg_soft(uint8_t reg_addr, uint8_t reg_size);
    int read_data(uint8_t index);
    bool get_flag_bit(uint8_t pos, uint8_t flags);

    bool isCharging();
    bool isIdle();
    bool isCharged();
    bool capacityInaccurate();
    bool isLow();   // SOC less than or equal to 6.25%
    bool isEmpty(); // SOC 0%

    int current_raw(uint8_t arg = 0);
    float current(uint8_t arg = 0);
    
    int voltage_raw();
    float voltage();
    
    int temperature_raw();
    float temperature();
    
    int power_raw();
    float power();
    
    int available_energy_raw();
    float available_energy();
    
    int time_to_empty_raw(uint8_t arg = 0);
    float time_to_empty(uint8_t arg = 0);
    
    int time_to_full_raw();
    float time_to_full();
    
    int last_measured_capacity_raw();
    float last_measured_capacity();
    
    int available_capacity_raw(uint8_t arg = 0);
    float available_capacity(uint8_t arg = 0);
    
    int state_of_charge_raw(uint8_t arg = 0);
    float state_of_charge(uint8_t arg = 0);
    
    int cycle_count(uint8_t arg = 0);
    
  private:
    bool _use_softwire;
    softi2c_scl softwire = softi2c_scl(D0,D1);

};

#endif