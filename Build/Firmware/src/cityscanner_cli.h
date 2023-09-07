#pragma once
#include "CS_core.h"
#include "Particle.h"
#include "location_service.h"
#include "cityscanner_sense.h"
#include "cityscanner_vitals.h"
#include "cityscanner_store.h"
#include "cityscanner_sleep.h"
#include "cityscanner.h"

int commandLine(String command);

int initCLI()
{
  Log.info("CLI started");
  Particle.function("CLI", commandLine);
  return 1;
}

void serialEvent()
{
  String s = "na";
  s = Serial.readStringUntil(char(13));
  commandLine(s);
}

int commandLine(String command)
{
  int index = command.indexOf(',');
  String first_parameter = "na";
  String second_parameter = "na";
  String third_parameter = "na";
  String fourth_parameter = "na";

  if (index == -1)
  {
    first_parameter = command;
  }
  else if (index > 0)
  {
    int secondCommaIndex = command.indexOf(',', index + 1);
    int thirdCommaIndex = command.indexOf(',', secondCommaIndex + 1);
    first_parameter = command.substring(0, index);
    second_parameter = command.substring(index + 1, secondCommaIndex);
    third_parameter = "na";
    fourth_parameter = "na";
    if (secondCommaIndex > 0 && thirdCommaIndex > 0)
    {
      third_parameter = command.substring(secondCommaIndex + 1, thirdCommaIndex);
      fourth_parameter = command.substring(thirdCommaIndex + 1);
    }
    else if (secondCommaIndex > 0)
    {
      third_parameter = command.substring(secondCommaIndex + 1);
    }
  }

  Log.info("RECEIVED: " + first_parameter + "," + second_parameter + "," + third_parameter + "," + fourth_parameter);
  
  //FUNCTION CALLS
  if (!first_parameter.compareTo("reboot"))
  {
    System.reset();
  }
  else if (!first_parameter.compareTo("stop"))
  {
    CitySleep::instance().stop();  // for 12 hours or WKP pin triggered
  }
  else if (!first_parameter.compareTo("hibernate"))
  {
    if(!third_parameter.compareTo("seconds"))
      CitySleep::instance().hibernate(second_parameter.toInt(),SECONDS);
    else if(!third_parameter.compareTo("minutes"))
      CitySleep::instance().hibernate(second_parameter.toInt(),MINUTES);
    else if(!third_parameter.compareTo("hours"))
      CitySleep::instance().hibernate(second_parameter.toInt(),HOURS);
  }

  else if (!first_parameter.compareTo("sd"))
  {
    if (!second_parameter.compareTo("dump")){
      if(!third_parameter.compareTo("all")){
        Log.info("Dumping All Data Files");
        CityStore::instance().dumpData(ALL_FILES);
        if (Particle.connected())
        Particle.publish("DUMP", "ended_dumping_all_data");
      }
      else{
        Log.info("Dumping Some Data Files");
        CityStore::instance().dumpData(third_parameter.toInt());
        if (Particle.connected())
        Particle.publish("DUMP", "ended_dumping_" + String(third_parameter.toInt()) + "_files");
      }
  }
  else if (!second_parameter.compareTo("files")){
    String files_in_queue = String::format("%u", CityStore::instance().countFilesInQueue());
    Log.info(files_in_queue);
    if (Particle.connected())
      Particle.publish("FILES", files_in_queue);
  }
  else if (!second_parameter.compareTo("format")){
    CityStore::instance().reInit();
    Log.info("Format SD");
    if (Particle.connected())
      Particle.publish("Format SD", "SD card initialized");
  }
  }

  else if (!first_parameter.compareTo("autosleep"))
  {
    if (!second_parameter.compareTo("on")){   
      MotionService::instance().setOverrideAutosleep(FALSE);
      Cityscanner::instance().sendWarning("Autosleep ON");
  }
  else if (!second_parameter.compareTo("off")){
      MotionService::instance().setOverrideAutosleep(TRUE);
      Cityscanner::instance().sendWarning("Autosleep OFF");
  }
  }

  else if (!first_parameter.compareTo("heat-cool"))
  {
    if (!second_parameter.compareTo("on")){   
      CS_core::instance().enableHEATER(TRUE);
      Cityscanner::instance().sendWarning("heat-cool ON");
  }
  else if (!second_parameter.compareTo("off")){
      CS_core::instance().enableHEATER(FALSE);
      Cityscanner::instance().sendWarning("heat-cool OFF");
  }
  }

  else if (!first_parameter.compareTo("last"))
  {
    String response = "error";
    if (!second_parameter.compareTo("payload"))
    {
      response = Time.now();
      response += "," + LocationService::instance().getGPSdata() + ",";
      response += Cityscanner::instance().data_payload;
      Log.info(response);
      if (Particle.connected())
          Particle.publish("last_payload", response);
    }
    else if (!second_parameter.compareTo("vitals"))
    {
      response = Time.now();
      response += "," + LocationService::instance().getGPSdata() + ",";
      response += Cityscanner::instance().vitals_payload;
      Log.info(response);
      if (Particle.connected())
          Particle.publish("last_vitals", response);
    }
  }

  else if (!first_parameter.compareTo("device-check"))
  {
     String response = "na";
     response = System.deviceID() + "," + LocationService::instance().getEpochTime() + "," + LocationService::instance().getGPSdata() + "," + Cityscanner::instance().data_payload;
     if (Particle.connected())
        Particle.publish("device",response);
  }

  else if (!first_parameter.compareTo("location"))
  {
    String status = "na";
    status = LocationService::instance().getGPSdata();
    Log.info(status);
    if (Particle.connected())
      Particle.publish("GPS", status);
  }
  else if (!first_parameter.compareTo("battery"))
  {
    String status = "na";
    status = CityVitals::instance().getBatteryData();
    Log.info(status);
    if (Particle.connected())
      Particle.publish("BATT", status);
  }
  else if (!first_parameter.compareTo("solar"))
  {
    String status = "na";
    status = CityVitals::instance().getSolarData();
    Log.info(status);
    if (Particle.connected())
      Particle.publish("SOLAR", status);
  } 
  else if (!first_parameter.compareTo("opc"))
  {
    String opcdata = CitySense::instance().last_opc_data;
    Log.info(opcdata);
    if (Particle.connected())
      Particle.publish("OPC", opcdata);
  }
  //END COMMANDS
  else if (!first_parameter.compareTo("enable3v"))
  {
    if (!second_parameter.compareTo("on"))
    {
      CS_core::instance().enable3V3(1);
    }
    else if (!second_parameter.compareTo("off"))
    {
      CS_core::instance().enable3V3(0);
    }
  }
  else if (!first_parameter.compareTo("enable5v"))
  {
    if (!second_parameter.compareTo("on"))
    {
      CS_core::instance().enable5V(1);
    }
    else if (!second_parameter.compareTo("off"))
    {
      CS_core::instance().enable5V(0);
    } 
  }
  else if (!first_parameter.compareTo("3v3on"))
  {
    Serial.println("Turning ON 3V3");
    CS_core::instance().enable3V3(true);
    delay(DTIME);
    Serial.println("Turning ON GPS");
    LocationService::instance().start();
    delay(DTIME);
    Serial.println("Turning ON Vitals");
    CityVitals::instance().init();
    delay(DTIME);
    Serial.println("Turning ON NOISE sensor");
    CitySense::instance().startNOISE();
    delay(DTIME);
    Serial.println("Turning ON Temperature sensor");
    CitySense::instance().startTEMP();
    delay(DTIME);
    Serial.println("Turning ON Gas ADC");
    CitySense::instance().startGAS();
    delay(DTIME);
  }
  else if (!first_parameter.compareTo("3v3off"))
  {
    Serial.println("Turning OFF GPS and ACC");
    MotionService::instance().stop();
    LocationService::instance().stop();
    delay(DTIME);
    Serial.println("Turning OFF Vitals");
    CityVitals::instance().stop_all();
    delay(DTIME);
    Serial.println("Turning OFF NOISE sensor");
    CitySense::instance().stopNOISE();
    delay(DTIME);
    Serial.println("Turning OFF Temperature sensor");
    CitySense::instance().stopTEMP();
    delay(DTIME);
    Serial.println("Turning OFF 3V3");
    CS_core::instance().enable3V3(false);
    delay(DTIME);
    Serial.println("Turning OFF Gas ADC");
    CitySense::instance().stopGAS();
    delay(DTIME);
  }
  else if (!first_parameter.compareTo("5von"))
  {
    Serial.println("Turning ON 5V");
    CS_core::instance().enable5V(true);
    delay(DTIME);
  }
  else if (!first_parameter.compareTo("5voff"))
  {
    Serial.println("Turning OFF OPC");
    CitySense::instance().stopOPC();
    delay(DTIME);
    Serial.println("Turning OFF 5V");
    CS_core::instance().enable5V(false);
  }
  else if (!first_parameter.compareTo("opcon")){
      CS_core::instance().enable5V(true);
      delay(1s);
      Serial.println("Turning ON OPC");
      CitySense::instance().startOPC();
      }
  else if (!first_parameter.compareTo("opcoff")){
    Serial.println("Turning OFF OPC");
      CitySense::instance().stopOPC();
  }
  return 1;
}