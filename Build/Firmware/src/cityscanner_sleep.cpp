#include "cityscanner_sleep.h"
#include "cityscanner.h"


CitySleep *CitySleep::_instance = nullptr;

CitySleep::CitySleep() :
  core(CS_core::instance()),
  sense(CitySense::instance()),
  store(CityStore::instance()),
  vitals(CityVitals::instance()),
  //motionService(MotionService::instance()),
  locationService(LocationService::instance())
{}

int CitySleep::init(){

}

void CitySleep::stop(){
    Log.info("Preparing for STOP mode");
    delay(100);
    //motionService.stop();
    locationService.stop();
    //stop sd card
    //store.stop();
    //SPI.endTransaction();
    //SPI.end();
   
    vitals.stop_all();
    sense.stop_all();
    core.enable3V3(FALSE);
    core.enableOPC(FALSE);
    core.enable5V(FALSE);
    //if(Particle.connected())
    //    Particle.publish("WAR","Going into STOP Mode");
    if(Cityscanner::instance().debug_mode){
        Log.info("Going into STOP mode");
        Cityscanner::instance().sendWarning("SLEEPING_zzz");
    }
    delay(100);
    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::STOP)
        .duration(std::chrono::hours(12))
        .gpio(WKP,CHANGE)
        .network(NETWORK_INTERFACE_CELLULAR, SystemSleepNetworkFlag::INACTIVE_STANDBY);
    System.sleep(config);
    if(Cityscanner::instance().debug_mode){
        Log.info("Back from STOP mode");
        waitFor(Particle.connected, 10000);
        Cityscanner::instance().sendWarning("WOKENUP");
    }
    //Particle.publish("WAR","Waking up from STOP Mode");
    delay(1s);
    core.enable3V3(TRUE);
    locationService.start();
    vitals.init();
    sense.startNOISE();
    sense.startTEMP();
    sense.startGAS();
    //init SD card
    //store.init();
    delay(1s);
    core.enable5V(TRUE);
    delay(1s);
    if(OPC_ENABLED)
        sense.startOPC();
}

void CitySleep::hibernate(uint8_t duration, uint8_t type){
    Log.info("Preparing for HIBERNATION mode");
    delay(100);
    store.stop();
    SPI.endTransaction();
    SPI.end();
    delay(500);
    //motionService.stop();
    locationService.stop();
    vitals.stop_all();
    sense.stop_all();
    core.enable3V3(FALSE);
    core.enableOPC(FALSE);
    core.enable5V(FALSE);
    delay(DTIME);

    Log.info("Going into HIBERNATION mode");
    if(Cityscanner::instance().debug_mode)
        Cityscanner::instance().sendWarning("HIBERNATING_brr");
    delay(100);

    SystemSleepConfiguration config2;

    switch (type)
    {
    case SECONDS:
        config2.mode(SystemSleepMode::HIBERNATE).duration(std::chrono::seconds(duration));
        break;
    case MINUTES:
        config2.mode(SystemSleepMode::HIBERNATE).duration(std::chrono::minutes(duration));
        break;
    case HOURS:
        config2.mode(SystemSleepMode::HIBERNATE).duration(std::chrono::hours(duration));
        break;
    default:
        config2.mode(SystemSleepMode::HIBERNATE).duration(std::chrono::seconds(duration));
        break;
    }    
    
    System.sleep(config2);
    //Here the sys should go into reset and the message below should never appear
    Log.info("Back from HIBERNATION mode"); 
    if(Cityscanner::instance().debug_mode)
        Cityscanner::instance().sendWarning("BACK_FROM_HIBERNATION");
    delay(100);

}