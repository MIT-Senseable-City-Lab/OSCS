#include "Particle.h"

#include "cityscanner_config.h"
#include "cityscanner.h"
#include "CS_core.h"

//PRODUCT_ID(PILOT_CITY); 
//PRODUCT_VERSION(PILOT_CITY_VERSION);


SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

STARTUP(
  Cityscanner::startup();
)

void myWatchdogHandler(void)
{
  Cityscanner::instance().sendWarning("Reset due to watchdog");
  System.reset();
}

SerialLogHandler LogHandler;
ApplicationWatchdog wd(10min, myWatchdogHandler);

void setup() { 
  Serial.begin(9600);
  Cityscanner::instance().init();
}

void loop() {
  Cityscanner::instance().loop();
}

