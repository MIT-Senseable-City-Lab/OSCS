#include "cityscanner_vitals.h"
#include "BQ27200_I2C.h"
#include "HTS221.h"
#include "ISL28022.h"
#include "CS_core.h"

CityVitals *CityVitals::_instance = nullptr;
BQ27200_I2C batt = BQ27200_I2C(0);
HTS221 temp_internal;
ISL28022 solar;
FuelGauge fuelg; 

CityVitals::CityVitals() {}


int CityVitals::init()
{   if(!BATT_started && BATT_ENABLED)
        startBattery();
    delay(DTIME);
    if(!SOLAR_started)
        startSolar();
    delay(DTIME);
    if(!TEMPint_started)
        startTempInt();
    delay(DTIME);
    return 1;
}

int CityVitals::stop_all()
{
    if(BATT_started)
        stopBattery();
    delay(DTIME);
    if(SOLAR_started)
        stopSolar();
    delay(DTIME);
    if(TEMPint_started)
        stopTempInt();
    delay(DTIME);
    return 1;
}

bool CityVitals::startBattery()
{
    BATT_started = true;
    return 1;
}

bool CityVitals::stopBattery()
{
    BATT_started = false;
    return 1;
}

String CityVitals::getBatteryData(){
    if(BATT_started)
    {
    batt.read_data(BQ27200_VOLT);
    String battery = "na";
    float voltage_v = 5;
    voltage_v = batt.voltage() / 1000;
    battery = String::format("%.0f", batt.state_of_charge()) + "," +
                String::format("%.1f", batt.temperature()) + "," +
                String::format("%.2f", voltage_v) + "," +
                String::format("%.2f", fuelg.getVCell()) + "," +
                String::format("%.0f", batt.current()) + "," +
                String::format("%u", batt.isCharging());
    return battery;
    }
    else
    return "na,na,na,na,na";
}

bool isBatteryLow(){
    float battery_v = 5;
    battery_v = fuelg.getVCell() < 3.8;
    Log.info("Battery voltage:" + String(battery_v));
    if(battery_v < 3.8)
        return true;
    else
        return false;
}


String CityVitals::getChargingStatus(){
    String charge_status = String::format("%u,%u", CS_core::instance().isCharging(), CS_core::instance().isCharged());
    return charge_status;
}

bool CityVitals::startSolar(){
    solar.begin(0x45, PV_SENSING);
    SOLAR_started = true;
    return 1;
}

bool CityVitals::stopSolar(){
    SOLAR_started = false;
    return 1;
}

String CityVitals::getSolarData(){
    if(SOLAR_started)
    return String::format("%.2f",solar.getBusVoltage_V()) + "," + String::format("%.1f",solar.getCurrent_mA());
    else
    return "na,na";
}

bool CityVitals::startTempInt(){
    
    Serial.println(temp_internal.begin());
    TEMPint_started = true;
    return 1;
}

bool CityVitals::stopTempInt(){
    TEMPint_started = false;
    return 1;
}

String CityVitals::getTempIntData(){
    if(TEMPint_started)
    return String::format("%.1f", temp_internal.readTemperature()) + "," + String::format("%.1f", temp_internal.readHumidity());
    else
    return "na,na";
}

String CityVitals::getSignalStrenght(){
    if(Cellular.isOn()){
        CellularSignal sig = Cellular.RSSI();
        float strength = sig.getStrength();
        return String::format("%.1f", strength);
    }
    else
        return "na";
    

}