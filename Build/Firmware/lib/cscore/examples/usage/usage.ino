#include "CS_core.h"

SYSTEM_MODE(MANUAL)

#define HW_VERSION V4


void setup() {
    CS_core::instance().begin(HW_VERSION);
    Serial.begin();
}

void loop() {
    delay(15s);
    Serial.println("Turning 3V ON");
    CS_core::instance().enable3V3(true);
    delay(5s);
    Serial.println("Turning 5V ON");
    CS_core::instance().enable5V(true);
    delay(5s);
    Serial.println("Turning GPS ON");
    CS_core::instance().enableGPS(true);
    delay(5s);
    Serial.println("Turning OPC ON");
    CS_core::instance().enableOPC(true);
    delay(5s);  
    Serial.println("Turning HEATER ON");
    CS_core::instance().enableHEATER(true);
    delay(5s);
    Serial.print("GPS Interrupt Status: "); Serial.println(CS_core::instance().getGPSstatus());
    Serial.print("isCharging?: "); Serial.println(CS_core::instance().isCharging());
    Serial.print("isCharged?: "); Serial.println(CS_core::instance().isCharged());
    delay(15s);
    Serial.println("Turning HEATER OFF");
    CS_core::instance().enableHEATER(false);
    delay(5s);
    Serial.println("Turning OPC OFF");
    CS_core::instance().enableOPC(false);
    delay(5s);
    Serial.println("Turning GPS OFF");
    CS_core::instance().enableGPS(false);
    delay(5s);
    Serial.println("Turning 5V OFF");
    CS_core::instance().enable5V(false);
    delay(5s);
    Serial.println("Turning 3V ON");
    CS_core::instance().enable3V3(false);
    delay(15s);
    
}
