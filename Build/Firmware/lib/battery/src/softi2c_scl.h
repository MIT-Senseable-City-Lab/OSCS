/*
Qiyun "Q" Gao - Sensable City Lab, MIT - Feb 20, 2021
Software I2C Master with clock stretching suppport 
Adapted from https://github.com/felias-fogg/SlowSoftI2CMaster/blob/master/SlowSoftI2CMaster.cpp by Bernhard Nebel
*/
#ifndef softi2c_scl_h
#define softi2c_scl_h

#include "Particle.h"

class softi2c_scl {
    public:
        softi2c_scl(uint8_t sda, uint8_t scl);
        uint32_t clkTick = 10; // microseconds
        void begin();
        int  i2c_start();
        int  i2c_restart();
        void i2c_stop();
        int  i2c_write(uint8_t data);
        int  i2c_read(uint8_t nack);
        void set0(uint8_t pin);
        void set1(uint8_t pin);
    private:
        uint8_t _sda;
        uint8_t _scl;
};

#endif