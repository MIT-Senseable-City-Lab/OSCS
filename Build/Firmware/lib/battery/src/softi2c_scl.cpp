#include "softi2c_scl.h"
#define START_TIMEOUT 1000
#define STRETCH_TIMEOUT 100000 // 100ms

softi2c_scl::softi2c_scl(uint8_t sda, uint8_t scl){
    _sda = sda;
    _scl = scl;
}

void softi2c_scl::set0(uint8_t pin){
    pinMode(pin,OUTPUT_OPEN_DRAIN);
    digitalWrite(pin,0);
}

void softi2c_scl::set1(uint8_t pin){
    pinMode(pin,INPUT);
    digitalWrite(pin,1);
}

void softi2c_scl::begin(){
    set1(_sda);
    set1(_scl);
}

int softi2c_scl::i2c_start(){
    // enter with both lines released (HIGH)
    set1(_sda);
    set1(_scl);
    uint32_t retry_count = 0;
    while (digitalRead(_sda) == 0 || digitalRead(_scl) == 0){
        retry_count += 1;
        if (retry_count > START_TIMEOUT){
            return(-1);
        }
        delayMicroseconds(1);
    }
    delayMicroseconds(clkTick);
    set0(_sda);
    delayMicroseconds(clkTick);
    set0(_scl);
    delayMicroseconds(clkTick);
    // here we have both lines low. 
    return(0);
}

int softi2c_scl::i2c_restart(){
    // enter with clock low
    set1(_scl); // release clock
    uint32_t retry_count = 0;
    while (digitalRead(_scl) == 0){  // wait for clock to go high
        retry_count += 1;
        if (retry_count > STRETCH_TIMEOUT){
            return(-1);
        }
        delayMicroseconds(1);
    }
    set0(_sda);
    delayMicroseconds(clkTick);
    set0(_scl);
    delayMicroseconds(clkTick);
    // here we have both lines low. 
    return(0);
}

void softi2c_scl::i2c_stop(){
    // enter with clock low
    set0(_sda); 
    delayMicroseconds(clkTick);
    set1(_scl); //release clock
    delayMicroseconds(clkTick/2);
    set1(_sda);
    delayMicroseconds(clkTick/2);
}


int softi2c_scl::i2c_write(uint8_t data){
    // enter with SCL driven LOW.
    for (uint8_t pos = 0; pos < 8; pos++){
        bool state = ( data << pos ) & 0b10000000;
        if(state){
            set1(_sda);
        } else {
            set0(_sda);
        }
        delayMicroseconds(clkTick/2);
        set1(_scl);
        delayMicroseconds(clkTick/2);
        set0(_scl);
    }
    // now SCL is LOW and SDA is not sure, release SDA
    set1(_sda);
    delayMicroseconds(clkTick/2);

    // drive SCL high to lock in the ACK state
    set1(_scl);
    delayMicroseconds(clkTick/2);
    bool nack = digitalRead(_sda);
    set0(_scl);
    
    // exit with SCL LOW.
    if (nack){
        return(-1); // NACKed by slave
    } else {
        return(0); // ACKed by slave
    }
}

int softi2c_scl::i2c_read(uint8_t nack){
    // enter with SCL low, SDA either way
    delayMicroseconds(clkTick/2);   // wait for half a tick
    set1(_scl);           // release clock

    uint32_t retry_count = 0;
    while (digitalRead(_scl) == 0){
        // wait for clock stretching to finish
        retry_count += 1;
        if (retry_count > STRETCH_TIMEOUT){
            return(-1);
        }
        delayMicroseconds(1);
    }
    // data shall be ready when slave releases the clock

    int data = 0;
    for (uint8_t pos = 0; pos < 8; pos++){
        set1(_scl); // rising edge, data locks in 
        delayMicroseconds(clkTick/2); // we are in the high period, wait half a tick
        // now we are right before the falling edge of the first bit
        data = data << 1;
        if (digitalRead(_sda) == 1){ // read that bit
            data = data | 0b1; 
        }
        set0(_scl); // create the falling edge
        delayMicroseconds(clkTick/2); // low period, slave prepare data
    }
    if (nack) {
        set1(_sda);
    } else {
        set0(_sda);
    }
    set1(_scl);
    delayMicroseconds(clkTick/2);
    set0(_scl);
    set1(_sda);
    // ACK or NACK sent, SCL = low, SDA = anything
    return (data);
}

