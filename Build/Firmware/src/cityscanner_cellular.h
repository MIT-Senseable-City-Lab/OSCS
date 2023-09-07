#include "Particle.h"
/*num_mins_to_reconnect = 2;      // Max number of minutes to wait for reconnection before turning off modem
uint8_t num_mins_waiting_to_reconnect = 0;  // Counter to Number of minutes elapsed while waiting for reconnection
bool trying_reconnect = false;          // Trying to reconnect
float num_hours_to_turn_on_cellular = 5.0 / 60.0;     // Number of hours after which cellular modem will turn on again
uint16_t num_mins_to_turn_on_cellular = 0;    // Counter to number of minutes elapsed while waiting to turn on the cellular modem again 
float curr_num_hours_to_turn_on_cellular = num_hours_to_turn_on_cellular;*/     // Current Number of hours value set after which cellular modem will turn on again

class CityCellular {
    public:
        static CityCellular &instance() {
            if(!_instance) {
                _instance = new CityCellular();
            }
            return *_instance;
        }

        int init(uint8_t num_min_recon, float num_hrs_on_cellular);
        bool smartconnect(void);

    private:
        CityCellular();
        static CityCellular* _instance;
        uint8_t num_mins_to_reconnect;      // Max number of minutes to wait for reconnection before turning off modem
        uint8_t num_mins_waiting_to_reconnect;  // Counter to Number of minutes elapsed while waiting for reconnection
        bool trying_reconnect;          // Trying to reconnect
        float num_hours_to_turn_on_cellular;     // Number of hours after which cellular modem will turn on again
        uint16_t num_mins_to_turn_on_cellular;    // Counter to number of minutes elapsed while waiting to turn on the cellular modem again 
        float curr_num_hours_to_turn_on_cellular;     // Current Number of hours value set after which cellular modem will turn on again
};