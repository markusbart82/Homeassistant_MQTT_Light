#ifndef CHANNELMANAGER_H
#define CHANNELMANAGER_H

#include <Arduino.h>
#include "channel.h"
#include "controller.h"

// a controller can have up to 3 channels
// order is fixed, controllers are filled with channels before next one is started
#define MAX_CHANNELS 9
#define MAX_CONTROLLERS 3

// holds channels and controllers
// channels are assigned ascending, so channel 1 uses LEDs 0..4 on controller 0 etc.
// channel 0 is a virtual channel, values are being relayed to the individual channels as an average
class Channelmanager{
  
  private:
    uint8_t numberOfChannels = 0; // actually used channels
    uint8_t numberOfControllers = 0; // actually used controllers
    Channel * channels[MAX_CHANNELS]; // pointers to potentially used channels, increase if necessary
    Controller * controllers[MAX_CONTROLLERS]; // pointers to potentially used controllers, increase if necessary
    
  public:
    // constructor, takes number of channels to create
    Channelmanager(uint8_t numberOfChannels);
    
    // loop function that does all the repeating/timing stuff
    void loop();
    
    // get a channel to interact with
    Channel* getChannel(uint8_t channelNumber);
    
    // get a controller to interact with
    Controller* getController(uint8_t controllerNumber);
        
};

#endif
