#include <Arduino.h>

struct ConfigurationVariables
{

    uint16_t binBoundriesADC[25];

    uint16_t binBoundriesDiametor[25];

    uint16_t binWeightings[24];

    uint16_t pmDiametorA;
    uint16_t pmDiametorB;
    uint16_t pmDiametorC;

    uint16_t maximumTimeOfFlight;
    uint16_t AMSamplingIntervalCount;
    uint16_t AMIdleIntervalCount;
    uint16_t AMMaxDataArraysInFile;

    uint8_t AMOnlySavePMData;
    uint8_t AMFanOnInIdle;

    bool valid;

    String toString();
};