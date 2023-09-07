#include <Arduino.h>

struct DACandPowerStatus
{
    uint8_t fanOn;
    uint8_t laserDACOn;
    uint8_t fanDACVal;
    uint8_t laserDACVal;
    uint8_t laserSwitch;
    uint8_t gainAndAutoGainToggleSetting;
    bool valid;

    String toString();

};