#include "cityscanner.h"
#include "Particle.h"
#include "cityscanner_cli.h"

Cityscanner *Cityscanner::_instance = nullptr;

bool flag_sampling = false;
void set_sample() { flag_sampling = true; }
Timer sample_timer(SAMPLE_RATE * 1000, set_sample);

bool flag_vitals = false;
void set_vitals() { flag_vitals = true; }
Timer vitals_timer(VITALS_RATE * 1000, set_vitals);

bool flag_routine = false;
void set_routine() { flag_routine = true; }
Timer routine_timer(ROUTINE_RATE * 1000, set_routine);

FuelGauge fuel;

// Timer values for smart connect
uint8_t num_min_recon = 2;                  // Number of minutes waiting for reconnection
float num_hrs_on_cellular = 1.0; //5.0 / 60.0;     // Number of hours waiting to turn on the modem

uint8_t num_min_to_turnoff_modem_pwrsave = 60;   //10 before// Max num of minutes to wait before turning off modem
uint8_t num_mins_waiting_to_turnoff_modem_pwrsave = 0;  // Number of minutes elapsed in waiting to turn off modem

float num_hrs_to_turn_on_cellular_pwrsave = 2.0; //150.0 / 60.0;     // Number of hours waiting to turn on the modem in power saving mode
uint16_t num_mins_to_turn_on_cellular_pwrsave = 120; //0;          // Max num of minutes to wait before turning on modem in power saving mode

bool msg_sent = false;

String deviceID = System.deviceID();

Cityscanner::Cityscanner() : core(CS_core::instance()),
                             sense(CitySense::instance()),
                             store(CityStore::instance()),
                             vitals(CityVitals::instance()),
                             motionService(MotionService::instance()),
                             locationService(LocationService::instance()),
                             cellular(CityCellular::instance())
{
}

void Cityscanner::startShippingMode()
{
}

void Cityscanner::startup()
{
  Log.info("Startup");
}

void blink(void);

int Cityscanner::init()
{
  delay(3s);
  Log.info("Init block");
  checkbattery();
  initCLI();
  Log.info("Starting Core Library");
  core.begin(HW_VERSION);
  delay(DTIME);
  Log.info("Turning ON Accelerometer");
  motionService.start();
  delay(DTIME);
  Serial.println("Turning ON SD card");
  store.init();
  if (SD_FORMAT_ONSTARTUP)
  store.reInit();
  delay(DTIME);
  if (MODE == PWRSAVE)
  {
    num_hrs_on_cellular = num_hrs_to_turn_on_cellular_pwrsave;
  }
  cellular.init(num_min_recon, num_hrs_on_cellular);
  delay(DTIME);
  
  if (CELLULAR_ON_STARTUP)
  {
    Cellular.on();
    Particle.connect();
    waitFor(Particle.connected, 60000);
    if (Particle.connected())
    {
      if (!msg_sent)
      {
        char strNumFiles[5];
        itoa(store.countFilesInQueue(), strNumFiles, 10);
        
        String output = String::format("%s,%s,%s,%s,%s",  
                                    deviceID.c_str(), 
                                    LocationService::instance().getEpochTime().c_str(), //Time
                                    LocationService::instance().getGPSdata().c_str(), // GPS data
                                    strNumFiles, // Number of files in Queue folder
                                    vitals.getBatteryData().c_str());  // battery data

        Particle.publish("DAT4", output);
        msg_sent = true;
      }
    }
    if (debug_mode)
      sendWarning("I_AM_OKAY");
  }

  sample_timer.start();
  vitals_timer.start();
  routine_timer.start();

  switch (MODE)
  {
  case TEST:
    break;
  default:
    Serial.println("Turning ON 3V3");
    core.enable3V3(true);
    delay(DTIME);
    delay(1s);
    Serial.println("Turning ON 5V line");
    core.enable5V(true);
    delay(DTIME);
    delay(1s);
    Serial.println("Turning ON GPS");
    locationService.start();
    delay(DTIME);
    Serial.println("Turning ON Vitals");
    vitals.init(); // TBC
    delay(DTIME);
    Serial.println("Turning ON NOISE sensor");
    sense.startNOISE();
    delay(DTIME);
    Serial.println("Turning ON Temperature sensor");
    sense.startTEMP(); // TBC
    delay(DTIME);
    //if (IR_ENABLED)
    //{
      Serial.println("Turning ON IR sensor");
      sense.startIR90640();
      delay(DTIME);
    //}
    Serial.println("Turning ON Gas sensor");
    sense.startGAS(); // TBC
    delay(DTIME);
    delay(1s);
    //if (OPC_ENABLED)
    //{
      Serial.println("Turning ON OPC");
      sense.startOPC();
      delay(DTIME);
    //}
    break;
  }
  Log.info("end INIT");
  return 1;
}

void Cityscanner::loop()
{

  if (flag_sampling)
  {
    flag_sampling = false;

    if (HARVARD_PILOT)
    {
      data_payload = String::format("%s,%s,%s", sense.getOPCdata(EXTENDED).c_str(), // PM1,PM25,PM10,[bins],flow_rate,countglitch,laser_status,tempOPC,humOPC,valid
                                    sense.getTEMPdata().c_str(),                    // temp,humidity
                                    sense.getGASdata().c_str());                    // w1,r1
    }
    else
    {

      data_payload = String::format("%s,%s,%s,%s,%s", sense.getOPCdata(OPC_DATA_VERSION).c_str(), sense.getTEMPdata().c_str(), sense.getGASdata().c_str(), sense.getNOISEdata().c_str(), sense.getIRdata90640().c_str());  //Temp,Gas,Noise,Thermal
      //data_payload = String::format("%s", sense.getIRdata90640().c_str());  //Temp,Gas,Noise,Thermal
      //data_payload = String::format("%s,%s,%s,%s", sense.getOPCdata(OPC_DATA_VERSION).c_str(), sense.getTEMPdata().c_str(), sense.getGASdata().c_str(), sense.getNOISEdata().c_str());  //Temp,Gas,Noise
                                                                                                                                 
      // data_payload = String::format("%s,%s", sense.getOPCdata(OPC_DATA_VERSION).c_str(), sense.getNOISEdata().c_str());            //TBC
    }

    switch (MODE)
    {
    case IDLE:
      Log.info("Idle Mode");
      break;
    case REALTIME:
      store.logData(BROADCAST_IMMEDIATE, Data, data_payload);
      Log.info("Real Time");
      break;
    case LOGGING:
      /* Dumping data over TCP every 50*sampling_rate seconds
      Serial.print("counter: "); Serial.println(counter++);
      if (counter % 50 == 0)
        store.dumpData(ALL_FILES);*/
      // printDebug();
      store.logData(BROADCAST_NONE, Data, data_payload);
      Log.info("Data Logging");
      // Serial.print("IR: "); Serial.println(sense.getIRdata());
      Serial.print("BATT: "); Serial.println(vitals.getBatteryData());
      break;
    case PWRSAVE:
      store.logData(BROADCAST_NONE, Data, data_payload);
      Log.info("Pwrsave");
      break;
    case TEST:
      Log.info("Test");
      // printDebug();
      // store.logData(BROADCAST_NONE, Data, data_payload);
      break;
    default:
      break;
    }
  }

  if (flag_vitals)
  {
    flag_vitals = false;

    vitals_payload = String::format("%s,%s,%s,%s,%s", vitals.getBatteryData().c_str(), // SOC,temp,voltage,voltage_Partice,current_mA,is_charging
                                    vitals.getChargingStatus().c_str(),                // isCharging,isCharged
                                    vitals.getTempIntData().c_str(),                   // temp_int,hum_int
                                    vitals.getSolarData().c_str(),                     // solar_volt,solar_current
                                    vitals.getSignalStrenght().c_str());               // Cellular signal strenght

    switch (MODE)
    {
      case LOGGING:
      store.logData(BROADCAST_NONE, Vitals, vitals_payload);
      Log.info("Vitals Logging");
      cellular.smartconnect();    // Check if cellular is connected, if not turn off modem
      break;
    case PWRSAVE:
      store.logData(BROADCAST_NONE, Vitals, vitals_payload);
      Log.info("Vitals Logging");
      break;
    default:
      break;
      /*cellular.smartconnect();    // Check if cellular is connected, if not turn off modem

        num_mins_waiting_to_turnoff_modem_pwrsave++;  // Increment elapsed time in waiting
        // If enough elpsed time in waiting has passed
        if (num_mins_waiting_to_turnoff_modem_pwrsave >= num_min_to_turnoff_modem_pwrsave)
        {
          Cellular.off();   // Turn off modem
        }
        //else    // If cellular is not connected to the cloud
        else
      {
        Particle.connect();   // Connect to cloud
        
      }*/

     }
}
/*
{
    case IDLE:
      Log.info("Idle Mode");
      break;
    case REALTIME:
      store.logData(BROADCAST_IMMEDIATE, Vitals, vitals_payload);
      Log.info("Real Time");
      break;
    case LOGGING:
      store.logData(BROADCAST_NONE, Vitals, vitals_payload);
      Log.info("Vitals Logging");
      break;
    case PWRSAVE:
      Log.info("Pwrsave");
      break;
    case TEST:
      Log.info("test");
      break;
    default:
      break;
    }
  }*/

  if (flag_routine)
  {
    Serial.println("Routine operations");
    Log.info("Routine operations");
    flag_routine = false;
    checkbattery();
    //cellular.smartconnect();
    switch (MODE)
    {
    case LOGGING:
      cellular.smartconnect();
      break;

    case PWRSAVE:
      Serial.println("Power Save Mode");
      if (Cellular.isOn())
      {
        Serial.println("PWRSAVE: Cellular is on");
        if (Particle.connected())
        {
          if (!msg_sent)
          {
            char strNumFiles[5];
            itoa(store.countFilesInQueue(), strNumFiles, 10);
            String output = String::format("%s,%s,%s,%s,%s",
                                        deviceID.c_str(),
                                        LocationService::instance().getEpochTime().c_str(), // Time
                                        LocationService::instance().getGPSdata().c_str(), // GPS data
                                        strNumFiles, // Number of files in Queue folder
                                        vitals.getBatteryData().c_str());  // battery data

            Particle.publish("DAT4", output);
            msg_sent = true;
          }
          Serial.println("PWRSAVE: Particle is connected");
          num_mins_waiting_to_turnoff_modem_pwrsave++;  // Increment elapsed time in waiting
          // If enough elpsed time in waiting has passed
          if (num_mins_waiting_to_turnoff_modem_pwrsave >= num_min_to_turnoff_modem_pwrsave)
          {
            num_mins_waiting_to_turnoff_modem_pwrsave = 0;
            Particle.disconnect();
            Cellular.off();   // Turn off modem
            msg_sent = false;
          }
        }
        else
        {
          Serial.println("PWRSAVE: Particle is not connected");
          bool retVal = cellular.smartconnect();
          if (retVal)
          {
            Serial.println("PWRSAVE: Smart connect True");
            num_mins_waiting_to_turnoff_modem_pwrsave = 0;
          }
          else
          {
            Serial.println("PWRSAVE: Smart connect False");
            num_mins_waiting_to_turnoff_modem_pwrsave = 0;
            num_mins_to_turn_on_cellular_pwrsave = 0;
          }
        }
      }
      else
      {
        Serial.println("PWRSAVE: Cellular is off");
        num_mins_to_turn_on_cellular_pwrsave++;
        if (num_mins_to_turn_on_cellular_pwrsave >= round(num_hrs_to_turn_on_cellular_pwrsave * 60))
        {
          Cellular.on();
          Particle.connect();
          num_mins_to_turn_on_cellular_pwrsave = 0;
        }
      }
    
    default:
      break;
    }
  }
  
  motionService.loop();
  // Serial.print("Tap: "); Serial.println(digitalRead(WKP));
}

void Cityscanner::checkbattery()
{
  int batt_volt_adc = analogRead(BATTERY_VOLTAGE_PIN);
  float battery_v = (batt_volt_adc / 4095.0) * 3.3 * 2;
  Log.info("Battery voltage:" + String(battery_v));
  if (battery_v < LOW_BATTERY_THRESHOLD)
  {
    String message = "LOW_BATTERY_" + String(battery_v) + "_v";
    Log.info(message);
    sendWarning(message);
    delay(2s);
    CitySleep::instance().hibernate(6, HOURS);
    }
}

void Cityscanner::sendWarning(String warning)
{
  // String tempp = deviceID + "," + Time.now() + "," + warning;
  // Log.info("Warning: " + tempp);
  store.logData(BROADCAST_NONE, Warning, warning);
  if (Particle.connected())
  {
    Particle.publish("WAR", warning);
  }
}

void blink()
{
  Serial.println("interrupt");
}

void Cityscanner::printDebug()
{
  Serial.println("-----------------------------------------------");
  Serial.print("GPS: ");
  Serial.print(locationService.getGPSdata());
  Serial.println(" *latitude,longitude*");
  //Serial.print("OPC: ");
  //Serial.print(sense.getOPCdata(BASE));
  //Serial.println(" *PM1,PM2.5,PM10,flow-rate,countglitch,laser_status,valid,tempOPC,humOPC*");
  Serial.print("TEMP-EXT: ");
  Serial.print(sense.getTEMPdata());
  Serial.println(" *temperature,humidity*");
  Serial.print("TEMP-INT: ");
  Serial.print(vitals.getTempIntData());
  Serial.println(" *temperature,humidity*");
  Serial.print("GAS: ");
  Serial.print(sense.getGASdata());
  Serial.println(" *w1,r1,w2,r2*");
  Serial.print("NOISE: ");
  Serial.print(sense.getNOISEdata());
  Serial.println(" *noise_level*");
  Serial.print("SOLAR: ");
  Serial.print(vitals.getSolarData());
  Serial.println(" *solar_volt,solar_current*");
  Serial.print("BATT: ");
  Serial.print(vitals.getBatteryData());
  Serial.println(" *SOC,temp,voltage,current_mA,is_charging*");
  Serial.print("BATT-CHARGER: ");
  Serial.print(vitals.getChargingStatus());
  Serial.println(" *isCharging,isCharged*");
  Serial.print("FILES ON SD CARD: ");
  Serial.println(store.countFilesInQueue());
  Serial.println("-----------------------------------------------");
  Serial.println();
}
