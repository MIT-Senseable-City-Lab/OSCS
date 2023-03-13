#include "ISL28022.h"

#define ISL28022PV    0x45    //power meter on PV
#define ISL28022BATT  0x42    //power meter on battery

#define BATT_SENSING 1
#define PV_SENSING 0

ISL28022 PV;
ISL28022 BATT; 

//Current monitors
PV.begin(ISL28022PV, PV_SENSING);
BATT.begin(ISL28022BATT, BATT_SENSING);

void setup(){
  PV.begin(ISL28022PV, PV_SENSING);
  BATT.begin(ISL28022BATT, BATT_SENSING);
}

void loop()
{
  float shunt_voltage_pv = PV.getShuntVoltage_mV();
  float bus_voltage_pv = PV.getBusVoltage_V();
  float current_mA_pv = PV.getCurrent_mA();

  float shunt_voltage_batt = BATT.getShuntVoltage_mV();
  float bus_voltage_batt = BATT.getBusVoltage_V();
  float current_mA_batt = BATT.getCurrent_mA();

  delay(1s);
}