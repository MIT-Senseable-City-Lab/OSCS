#include "OPCN3.h"

#include <Arduino.h>
#include <SPI.h>

OPCN3::OPCN3(uint8_t pinSelect, uint32_t speedSelect)
{
    _ssPin = pinSelect;
    _speedMaximum = speedSelect;
}

// Alpha Sensor Functions
void OPCN3::begin()
{
    Serial.println("Initiating SPI ");
    // set ss pin to output
    pinMode(_ssPin, OUTPUT);
    SPI1.begin();
    // set speed, order and mode
    SPI1.beginTransaction(SPISettings(_speedMaximum, MSBFIRST, SPI_MODE1));
    delay(1000);
}

void OPCN3::initialize()
{
    delay(1000);
    Serial.println("Initilize");
    delay(1000);
    begin();
    delay(1000);
    readSerialNumber();
    Serial.println("Serial Number:");
    Serial.println(getSerialNumber());
    delay(1000);
    Serial.println("DACandPowerStatus");
    readDACandPowerStatus();
    delay(1000);
    Serial.println("ConfigurationVariables");
    readConfigurationVariables();
    delay(4000);
    Serial.println("");
    delay(10000);
    Serial.println("Turn Fan on");
    struct FanDigitalPotShutdownState fanState = setFanDigitalPotShutdownState(true);
    delay(1000);
    Serial.println("Turn Laser on");
    struct LaserDigitalPotShutdownState laserState = setLaserDigitalPotShutdownState(true);
    delay(1000);
    Serial.println("Turn Laser Switch on");
    struct LaserPowerSwitchState laserPowerState = setLaserPowerSwitchState(true);
    delay(1000);
    Serial.println("High Gain");
    struct HighLowGainState gainState = setHighLowGainState(true);
    delay(2000);
    bool reset = resetHistogram();
    delay(2000);
}

struct DACandPowerStatus OPCN3::readDACandPowerStatus()
{
    DACandPowerStatus dACandPowerStatus = sendCommand<DACandPowerStatus>(0X13, 0X13, 6);
    Serial.println(dACandPowerStatus.toString());
    return dACandPowerStatus;
}

struct FanDigitalPotShutdownState OPCN3::setFanDigitalPotShutdownState(bool status)
{
    byte commandByte = status ? 0X03 : 0X02;
    FanDigitalPotShutdownState fanDigitalPotShutdownState = sendCommand<FanDigitalPotShutdownState>(0X03, commandByte, 1);
    fanDigitalPotShutdownState.fanOn = status;
    Serial.print("Validity: ");
    Serial.println(fanDigitalPotShutdownState.valid);
    Serial.print(fanDigitalPotShutdownState.fanOn);
    Serial.print(" ");
    return fanDigitalPotShutdownState;
}

struct LaserDigitalPotShutdownState OPCN3::setLaserDigitalPotShutdownState(bool status)
{
    byte commandByte = status ? 0X05 : 0X04;
    LaserDigitalPotShutdownState laserDigitalPotShutdownState = sendCommand<LaserDigitalPotShutdownState>(0X03, commandByte, 1);
    laserDigitalPotShutdownState.laserOn = status;
    Serial.print("Validity: ");
    Serial.println(laserDigitalPotShutdownState.valid);
    Serial.print(laserDigitalPotShutdownState.laserOn);
    Serial.print(" ");
    return laserDigitalPotShutdownState;
}

struct LaserPowerSwitchState OPCN3::setLaserPowerSwitchState(bool status)
{
    byte commandByte = status ? 0X07 : 0X06;
    LaserPowerSwitchState laserPowerSwitchState = sendCommand<LaserPowerSwitchState>(0X03, commandByte, 1);
    laserPowerSwitchState.laserOn = status;
    Serial.print("Validity: ");
    Serial.println(laserPowerSwitchState.valid);
    Serial.print(laserPowerSwitchState.laserOn);
    Serial.print(" ");
    return laserPowerSwitchState;
}

struct HighLowGainState OPCN3::setHighLowGainState(bool status)
{
    byte commandByte = status ? 0X10 : 0X11;
    HighLowGainState highLowGainState = sendCommand<HighLowGainState>(0X03, commandByte, 1);
    highLowGainState.highLow = status;
    Serial.print("Validity: ");
    Serial.println(highLowGainState.valid);
    Serial.print(highLowGainState.highLow);
    Serial.print(" ");
    return highLowGainState;
}

struct HistogramData OPCN3::readHistogramData()
{
    HistogramData histogramData = sendCommand<HistogramData>(0X30, 0X30, 86);
    //Serial.println(histogramData.toString());
    return histogramData;
}

bool OPCN3::resetHistogram()
{
    HistogramData data = readHistogramData();
    return data.valid;
}

struct SerialNumber OPCN3::readSerialNumber()
{
    SerialNumber serialNumber = sendCommand<SerialNumber>(0X10, 0X10, 60);
    Serial.print("Validity: ");
    Serial.println(serialNumber.valid);
    String info = "";
    for (int i = 0; i < 60; i++)
    {
        info += String(serialNumber.serial[i]);
    }
    serial = info;
    return serialNumber;
}

String OPCN3::getSerialNumber()
{
    return serial;
}

struct ConfigurationVariables OPCN3::readConfigurationVariables()
{
    ConfigurationVariables configurationVariables = sendCommand<ConfigurationVariables>(0X3C, 0X3C, 163);
    Serial.println(configurationVariables.toString());
    return configurationVariables;
}

void OPCN3::beginTransfer()
{
    digitalWrite(_ssPin, LOW);
    delay(1);
}

void OPCN3::endTransfer()
{
    delay(1);
    digitalWrite(_ssPin, HIGH);
}

bool OPCN3::validate(byte arrayOne[], byte arrayTwo[], int size)
{
    bool valid = true;
    for (int i = 0; i < size; i++)
    {
        if (arrayOne[i] != arrayTwo[i])
        {
            valid = false;
        }
    }

    return valid;
}
