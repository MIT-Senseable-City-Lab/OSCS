#ifndef OPCN3_H
#define OPCN3_H

#include <Arduino.h>
#include <SPI.h>

#include "models/DACandPowerStatus.h"
#include "models/FanDigitalPotShutdownState.h"
#include "models/HistogramData.h"
#include "models/ConfigurationVariables.h"
#include "models/LaserDigitalPotShutdownState.h"

class OPCN3
{
private:
    uint8_t _ssPin;
    uint32_t _speedMaximum;
    String serial;
    void beginTransfer();
    void endTransfer();
    bool validate(byte arrayOne[], byte arrayTwo[], int size);
    template <class ResponseType>
    ResponseType sendCommand(byte inputByte, byte commandByte, int outputSize)
    {
        byte validator[2] = {0X31, 0XF3};
        byte initial[2];
        byte dataIn[outputSize];

        beginTransfer();
        initial[0] = SPI1.transfer(inputByte);
        delay(10);
        initial[1] = SPI1.transfer(inputByte);
        for (int i = 0; i < outputSize; i++)
        {
            delayMicroseconds(10);
            dataIn[i] = SPI1.transfer(commandByte);
        }
        endTransfer();

        ResponseType response;
        memcpy(&response, &dataIn, sizeof(response));
        response.valid = validate(initial, validator, 2);
        return response;
    };

public:
    OPCN3(uint8_t pinSelect, uint32_t speedSelect = 500000);
    void begin();
    void initialize();
    struct DACandPowerStatus readDACandPowerStatus();
    struct FanDigitalPotShutdownState setFanDigitalPotShutdownState(bool status);
    struct LaserDigitalPotShutdownState setLaserDigitalPotShutdownState(bool status);
    struct LaserPowerSwitchState setLaserPowerSwitchState(bool status);
    struct HighLowGainState setHighLowGainState(bool status);
    struct HistogramData readHistogramData();
    struct SerialNumber readSerialNumber();
    struct ConfigurationVariables readConfigurationVariables();
    bool resetHistogram();
    String getSerialNumber();
};


struct LaserPowerSwitchState
{
    bool laserOn;
    bool valid;
};

struct HighLowGainState
{
    bool highLow;
    bool valid;
};

struct SerialNumber
{
    char serial[60];
    bool valid;
};


#endif