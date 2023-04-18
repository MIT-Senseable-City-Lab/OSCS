#include "cityscanner_sense.h"
#include "CS_core.h"
#include <SensirionI2CSen5x.h>
#include <Wire.h>
#include "BME280.h"
//#include "Adafruit_ADS1015.h"
#include "DFRobot_SHT20.h"
#include "Adafruit_MLX90614.h"
#include "sps30.h"
#include <i2c_adc_ads7828.h>

CitySense *CitySense::_instance = nullptr;

//#define MAXBUF_REQUIREMENT 48
#define SP30_COMMS I2C_COMMS
//#define SP30_COMMS Wire

//SensirionI2CSen5x sen5x;
SPS30 sps30;
uint8_t ret, error_cnt = 0;
struct sps_values val;
//Adafruit_ADS1115 gas;  
BME280 tempext;
DFRobot_SHT20 sht20;
Adafruit_MLX90614 mlx1 = Adafruit_MLX90614(); 

// device 0
// Address: A1=0, A0=0
// Command: SD=1, PD1=1, PD0=1
ADS7828 device(0, SINGLE_ENDED | REFERENCE_OFF | ADC_ON, 0x0F);
ADS7828* adc = &device;
ADS7828Channel* adc0 = adc->channel(0);
ADS7828Channel* adc1 = adc->channel(1);
ADS7828Channel* adc2 = adc->channel(2);
ADS7828Channel* adc3 = adc->channel(3);

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

    /*Wire.begin();
    sen5x.begin(Wire);

    uint16_t error;
    char errorMessage[256];
    error = sen5x.deviceReset();
    if (error) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        OPC_started = false;
        return 0;
    }

    float tempOffset = 0.0;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        OPC_started = false;
        return 0;
    } else {
        Serial.print("Temperature Offset set to ");
        Serial.print(tempOffset);
        Serial.println(" deg. Celsius (SEN54/SEN55 only");
    }

    // Start Measurement
    error = sen5x.startMeasurement();
    if (error) {
        Serial.print("Error trying to execute startMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        OPC_started = false;
        return 0;
    } else {
        Serial.println(F("Initialized SEN55."));
        OPC_started = true;
        return 1;
    }*/
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

        /*uint16_t error;
        char errorMessage[256];

        // Read Measurement
        float massConcentrationPm1p0;
        float massConcentrationPm2p5;
        float massConcentrationPm4p0;
        float massConcentrationPm10p0;
        float ambientHumidity;
        float ambientTemperature;
        float vocIndex;
        float noxIndex;

        String opcdata = "na";

        error = sen5x.readMeasuredValues(
            massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
            massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
            noxIndex);

        if (error) {
            Serial.print("Error trying to execute readMeasuredValues(): ");
            errorToString(error, errorMessage, 256);
            Serial.println(errorMessage);
            return "na,na,na,na,na,na,na,na";
        } else {
            Serial.print("MassConcentrationPm1p0:");
            Serial.print(massConcentrationPm1p0);
            opcdata = String::format("%.2f", massConcentrationPm1p0) + ","; 
            Serial.print("\t");
            Serial.print("MassConcentrationPm2p5:");
            Serial.print(massConcentrationPm2p5);
            opcdata = opcdata + String::format("%.2f", massConcentrationPm2p5) + ","; 
            Serial.print("\t");
            Serial.print("MassConcentrationPm4p0:");
            Serial.print(massConcentrationPm4p0);
            opcdata = opcdata + String::format("%.2f", massConcentrationPm4p0) + ",";
            Serial.print("\t");
            Serial.print("MassConcentrationPm10p0:");
            Serial.print(massConcentrationPm10p0);
            opcdata = opcdata + String::format("%.2f", massConcentrationPm10p0) + ","; 
            Serial.print("\t");
            Serial.print("AmbientHumidity:");
            if (isnan(ambientHumidity)) {
                Serial.print("n/a");
                opcdata = opcdata + "na" + ",";
            } else {
                Serial.print(ambientHumidity);
                opcdata = opcdata + String::format("%.2f", ambientHumidity) + ",";
            }
            Serial.print("\t");
            Serial.print("AmbientTemperature:");
            if (isnan(ambientTemperature)) {
                Serial.print("n/a");
                opcdata = opcdata + "na" + ",";
            } else {
                Serial.print(ambientTemperature);
                opcdata = opcdata + String::format("%.2f", ambientTemperature) + ",";
            }
            Serial.print("\t");
            Serial.print("VocIndex:");
            if (isnan(vocIndex)) {
                Serial.print("n/a");
                opcdata = opcdata + "na" + ",";
            } else {
                Serial.print(vocIndex);
                opcdata = opcdata + String::format("%.2f", vocIndex) + ",";
            }
            Serial.print("\t");
            Serial.print("NoxIndex:");
            if (isnan(noxIndex)) {
                Serial.println("n/a");
                opcdata = opcdata + "na";
            } else {
                Serial.println(noxIndex);
                opcdata = opcdata + String::format("%.2f", noxIndex);
            }
        }

        return opcdata;*/
    
    } else {
        return "na,na,na,na,na,na,na,na";
        //return "na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na,na";
    }
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
    //gas.begin();
    // enable I2C communication
    ADS7828::begin();

    // adjust scaling on an individual channel basis
    adc0->minScale = 220;
    adc0->maxScale = 470;

    adc1->minScale = 220;
    adc1->maxScale = 420;

    adc2->minScale = 270;
    adc2->maxScale = 330;

    adc3->minScale = 270;
    adc3->maxScale = 300;

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
            /*float op1,op2;
            op1 = gas.readADC_Differential_0_1() * 0.1875F;
            op2 = gas.readADC_Differential_2_3() * 0.1875F; 
            return String::format("%.3f,%.3f",op1,op2);*/
        } 
        else
        {
        /*int16_t adc0, adc1, adc2, adc3;
        adc0 = gas.readADC_SingleEnded(0); //SN1 working
        adc1 = gas.readADC_SingleEnded(1); //SN1 Reference
        adc2 = gas.readADC_SingleEnded(2); //SN2 working
        adc3 = gas.readADC_SingleEnded(3); //SN2 reference
        int16_t sn1_w,sn1_r,sn2_w,sn2_r;
        sn1_w = adc0 * 0.1875F;
        sn1_r = adc1 * 0.1875F;
        sn2_w = adc2 * 0.1875F;
        sn2_r = adc3 * 0.1875F;*/
        int16_t sn1_w,sn1_r,sn2_w,sn2_r;
        ADS7828::updateAll();
        sn2_w = adc0->value();
        sn2_r = adc1->value(); 
        sn1_w = adc2->value();
        sn1_r = adc3->value();
        //Serial.print("SN1_W : ");
        //Serial.println(sn1_w);
        String payload = "n/a";
        payload = String::format("%d,%d,%d,%d", sn2_w,sn2_r,sn1_w,sn1_r);
        return payload;
        }
     } else
    return "na,na,na,na";
}
