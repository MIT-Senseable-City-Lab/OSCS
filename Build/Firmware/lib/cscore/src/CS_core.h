#pragma once

// This will load the definition for common Particle variable types

#include "Particle.h"
#include <Wire.h>

#define V3 0
#define V4 1

//#define MB_RST A3
//#define MB_INT A1		// Interrupt from Accelerometer. In previous short with mikrobus interrupt
#define MB_AN A6
#define MB_CS D23
#define MB_PWM A2

//#define STAT1 B0
//#define STAT2 B3

#define EN_3V D6
#define EN_3V_GPS D5
#define EN_5V D7
//#define EN_OPC C5
//#define EN_HEATER C4

#define BATTERY_VOLTAGE_PIN A5

//#define INT_ACC WKP
#define INT_ACC A1
//#define INT_GPS A0

// This is your main class that users will import into their application
class CS_core
{
public:
  static CS_core &instance()
  {
    if (!_instance)
    {
      _instance = new CS_core();
    }
    return *_instance;
  }

  void begin(uint8_t hw_release);
  void enable3V3(bool command);
  void enable5V(bool command);
  void enableOPC(bool command);
  void enableGPS(bool command);
  void activateGPS(bool command);
  void enableHEATER(bool command);
  void enableALL(bool command);
  uint8_t getGPSstatus();
  uint8_t isCharging();
  uint8_t isCharged();

private:
  CS_core();
  static CS_core* _instance;
  uint8_t m_inp;
  uint8_t m_out;
  uint8_t m_pol;
  uint8_t m_ctrl;
  uint8_t hw_version;
  boolean pinMode_ext(uint8_t address, uint8_t pin, uint8_t mode);
  boolean pinPolarity(uint8_t address, uint8_t pin, uint8_t polarity);
  boolean digitalWrite_ext(uint8_t address, uint8_t pin, boolean val);
  boolean digitalRead_ext(uint8_t address, uint8_t pin);
};

