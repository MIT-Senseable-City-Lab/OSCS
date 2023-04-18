#pragma once
#include "Particle.h"
#include "kxtj3-1057.h"
#include "cityscanner_CONFIG.h"
#include "cityscanner_sleep.h"



class MotionService {

    public:
        /**
     * @brief Return instance of the motion service
     *
     * @retval MotionService&
     */
   
    static MotionService &instance()
    {
        if(!_instance)
        {
            _instance = new MotionService();
            
        }
        return *_instance;
    }
    static void timer_fnc(void);
    int start();
    int stop();
    bool motionservice_started = false;
    int enableMotionDetection();
    int disableMotionDetection();
    int waitOnEvent();
    bool OVVERRIDE_AUTOSLEEP = false;
    void setOverrideAutosleep(bool);
    void testAccelerometer();
    static void resetInactivityCounter();
    static int getInactivityCounter(void);
    void loop();

    private:
        MotionService();
        static MotionService *_instance;
        static int inactivity_counter;

};
