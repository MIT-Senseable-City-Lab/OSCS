#include "cityscanner_cellular.h"

CityCellular *CityCellular::_instance = nullptr;

CityCellular::CityCellular() {}

int CityCellular::init(uint8_t num_min_recon, float num_hrs_on_cellular)
{
    num_mins_to_reconnect = num_min_recon;
    num_mins_waiting_to_reconnect = 0;
    trying_reconnect = false;
    num_hours_to_turn_on_cellular = num_hrs_on_cellular;
    num_mins_to_turn_on_cellular = 0;
    curr_num_hours_to_turn_on_cellular = num_hours_to_turn_on_cellular;
    return 0;
}

bool CityCellular::smartconnect()
{
    if (Cellular.isOff())     // If modem is already off
    {
      Serial.println("Cellular Modem is off");
      num_mins_to_turn_on_cellular++; // Increment counter to minutes elapsed before turning on the modem again
      Serial.print("Num of min to turn on cellular : ");
      Serial.println(num_mins_to_turn_on_cellular);
      // If number of minutes elapsed is greater than the time set in hours to turn on the modem again
      if (num_mins_to_turn_on_cellular >= round(curr_num_hours_to_turn_on_cellular * 60))
      {
        Serial.println("Turning Cellular Modem On");
        Cellular.on();    // Turn on Cellular modem
        Particle.connect();     // Connect to cloud
        num_mins_to_turn_on_cellular = 0;
      }
    }
    else      // If modem is on
    {
      Serial.println("Cellular Modem is on");
      if (!Cellular.ready())    // If modem is not connected to the cellular network
      {
        Serial.println("Cellular is not connected");
        if (trying_reconnect)     // If already trying to reconnect
        {
          num_mins_waiting_to_reconnect++;    // Increment the counter to number of minutes while witing for reconnection
          Serial.print("Trying to reconnect : ");
          Serial.println(num_mins_waiting_to_reconnect);
          // If number of minutes elapsed while waiting for reconnection has passed 
          if (num_mins_waiting_to_reconnect >= num_mins_to_reconnect)
          {
            Serial.println("Turning Cellular Modem Off");
            //multiplying_factor++;
            //num_hours_to_turn_on_cellular = num_hours_to_turn_on_cellular * multiplying_factor;
            curr_num_hours_to_turn_on_cellular = curr_num_hours_to_turn_on_cellular + num_hours_to_turn_on_cellular;
            Cellular.off();   // Turn off cellular modem
            trying_reconnect = false;
            return false;
          }
        }
        else
        {
          trying_reconnect = true;
          num_mins_waiting_to_reconnect = 0;
          Serial.print("Trying to reconnect : ");
          Serial.println(num_mins_waiting_to_reconnect);
        }
      }
      else    // Cellular modem is connected to cloud
      {
        return true;
        Serial.println("Cellular is connected");
      }
      return 0;
    }
    return 0;
}


