#include "cityscanner_sense.h"
#include "CS_core.h"
#include "OPCN3.h"
#include <Wire.h>
#include "BME280.h"
#include "Adafruit_ADS1015.h"
#include "DFRobot_SHT20.h"
#include "Adafruit_MLX90614.h"
#include "sps30.h"

CitySense *CitySense::_instance = nullptr;

#define SP30_COMMS I2C_COMMS

//OPCN3 myOPCN3(D5);
SPS30 sps30;
uint8_t ret, error_cnt = 0;
struct sps_values val;
Adafruit_ADS1115 gas;  
BME280 tempext;
DFRobot_SHT20 sht20;
Adafruit_MLX90614 mlx1 = Adafruit_MLX90614(); 

CitySense::CitySense() {}


int CitySense::init()
{   
    if(!TEMPext_started)
        startTEMP();
    delay(DTIME);
    if(!NOISE_started)
        startNOISE();
    delay(DTIME);
    if(!GAS_started)
        startGAS();
    delay(DTIME);
    if(!OPC_started && OPC_ENABLED)
        startOPC();
    delay(DTIME);
    startIR();
    return 1;
}

int CitySense::stop_all()
{
    if(OPC_started)
        stopOPC();
    delay(DTIME);
    if(TEMPext_started)
        stopTEMP();
    delay(DTIME);
    if(NOISE_started)
        stopNOISE();
    delay(DTIME);
    if(GAS_started)
        stopGAS();
    delay(DTIME);
    return 1;
}

bool CitySense::startOPC()
{
    //CS_core::instance().enableOPC(1);
    //myOPCN3.initialize();
    if (!sps30.begin(SP30_COMMS))
      Serial.println("could not initialize communication channel.");

    // check for SPS30 connection
    if (!sps30.probe()) {
        Serial.println("could not probe / connect with SPS30.");
        OPC_started = false;
        return 0;
    } else {
        Serial.println(F("Detected SPS30."));
        OPC_started = true;
        return 1;
    }
}

bool CitySense::startIR(){
    mlx1.begin();
    IR_started = true;
    return 1;
}

bool CitySense::stopIR(){
    IR_started = false;
    return 1;
}

String CitySense::getIRdata(){
    if(IR_started)
    {
        //mlx.begin(); 
        return String::format("%.1f,%.1f", mlx1.readAmbientTempC(), mlx1.readObjectTempC());
        /*double (Adafruit_MLX90614::)() ATC, OTC, ATF, OTF;
        ATC = mlx.readAmbientTempC;
        OTC = mlx.readObjectTempC;
        ATF = mlx.readAmbientTempF;
        OTF = mlx.readObjectTempF;
        return String::format("%.3f", "%.3f", "%.3f", "%.3f",ATC, OTC, ATF, OTF); */
    }
    else 
    return "na,na";
}

bool CitySense::stopOPC()
{
    //CS_core::instance().enableOPC(0);
    OPC_started = false;
    return 1;
}

String CitySense::getOPCdata(int option)
{
    if(OPC_started){
    /*HistogramData hist = myOPCN3.readHistogramData();
    String opcdata = "na";
    opcdata = String::format("%.2f", hist.pm1) + "," + 
              String::format("%.2f", hist.pm2_5) + "," + 
              String::format("%.2f", hist.pm10) + ",";
   if(option == EXTENDED){
        for (int i = 0; i < 24; i++) {
        opcdata += hist.binCounts[i];
        opcdata += ",";
        }
    }               
    opcdata +=  String(hist.sampleFlowRate) + "," + 
    hist.rejectCountGlitch + "," + 
    hist.laserStatus + "," + 
    String::format("%.2f,%.2f",hist.getTempC(), hist.getHumidity()) + "," + hist.valid; 
    last_opc_data = opcdata;
    return opcdata;*/
        do
        {
            ret = sps30.GetValues(&val);
            // data might not have been ready
            if (ret == SPS30_ERR_DATALENGTH)
            {
                if (error_cnt++ > 3)
                {
                    Serial.println("Error during reading values: ");
                }
                delay(1000);
            }

            // if other error
            else if (ret != ERR_OK)
            {
                Serial.println("Error during reading values: ");
            }

        } while (ret != ERR_OK); 
        String opcdata = "na";

        opcdata = String::format("%.2f", val.MassPM1) + "," + 
              String::format("%.2f", val.MassPM2) + "," + 
              String::format("%.2f", val.MassPM4) + "," +
              String::format("%.2f", val.MassPM10) + "," + 
              String::format("%.2f", val.NumPM0) + "," +
              String::format("%.2f", val.NumPM1) + "," + 
              String::format("%.2f", val.NumPM2) + "," +
              String::format("%.2f", val.NumPM4) + "," + 
              String::format("%.2f", val.NumPM10) + "," +
              String::format("%.2f", val.PartSize);

        return opcdata;
    
    } else
        return "na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na";
}

bool CitySense::startTEMP()
{   
    Wire.begin();
    if(OLD_TEMPERATURE_SENSOR)
     sht20.initSHT20();
    else
      Serial.println(tempext.beginI2C());
    TEMPext_started = true;
    return 1;
}

bool CitySense::stopTEMP()
{   
    TEMPext_started = false;
    return 1;
}

String CitySense::getTEMPdata()
{
    if(TEMPext_started)
    {
    if(OLD_TEMPERATURE_SENSOR)
     return String::format("%.2f,%.2f", sht20.readTemperature(), sht20.readHumidity());
    else 
     return String::format("%.1f,%.1f", tempext.readTempC(), tempext.readFloatHumidity());   
    } else
    return "na,na";
}

bool CitySense::startNOISE(){
    pinMode(A4,INPUT);
    NOISE_started = true;
    return 1;
}
bool CitySense::stopNOISE(){
    NOISE_started = false;
    return 1;
}

String CitySense::getNOISEdata(){
    if(NOISE_started)
    return String(analogRead(A4));
    else 
    return "na";
}

bool CitySense::startGAS()
{
    gas.begin();
    GAS_started = true;
    return 1;
}

bool CitySense::stopGAS()
{
    GAS_started = false;
    return 1;
}

String CitySense::getGASdata(){
    // TODO:Implement gas sensing in a non-blocking way
    if(GAS_started)
    {
    if(HARVARD_PILOT)
    {
        float op1,op2;
        op1 = gas.readADC_Differential_0_1() * 0.1875F;
        op2 = gas.readADC_Differential_2_3() * 0.1875F; 
        return String::format("%.3f,%.3f",op1,op2);
    } 
    else
    {
    int16_t adc0, adc1, adc2, adc3;
    adc0 = gas.readADC_SingleEnded(0); //SN1 working
    adc1 = gas.readADC_SingleEnded(1); //SN1 Reference
    adc2 = gas.readADC_SingleEnded(2); //SN2 working
    adc3 = gas.readADC_SingleEnded(3); //SN2 reference
    int16_t sn1_w,sn1_r,sn2_w,sn2_r;
    sn1_w = adc0 * 0.1875F;
    sn1_r = adc1 * 0.1875F;
    sn2_w = adc2 * 0.1875F;
    sn2_r = adc3 * 0.1875F;
    String payload = "n/a";
    payload = String::format("%d,%d,%d,%d", sn1_w,sn1_r,sn2_w,sn2_r);
    return payload;
    }} else
    return "na,na,na,na";
}