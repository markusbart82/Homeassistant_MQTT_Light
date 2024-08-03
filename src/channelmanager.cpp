// functions for messagehandler and others to call, to set states etc. specific to channels

#include "channelmanager.h"

Channelmanager::Channelmanager(uint8_t numberOfChannels){
  this->numberOfChannels = numberOfChannels;
  this->numberOfControllers = 0;
  for(uint8_t channelIterator = 0; channelIterator < this->numberOfChannels; channelIterator++){
    channels[channelIterator] = new Channel();
    // create one controller each time the next batch of 3 channels is started
    if(channelIterator % 3 == 0){
      controllers[channelIterator / 3] = new Controller();
      this->numberOfControllers++;
    }
  }
}

void Channelmanager::loop(){
  // call loop function of all configured channels
  for(uint8_t channelIterator = 0; channelIterator < this->numberOfChannels; channelIterator++){
    channels[channelIterator]->loop();
  }
  
  // check for updates per channel to trigger their data updates
  for(uint8_t controllerIterator = 0; controllerIterator < this->numberOfControllers; controllerIterator++){
    bool updateNeeded = false;
    for(uint8_t channelIterator = controllerIterator * 3; channelIterator < ((controllerIterator+1) * 3) && (channelIterator < this->numberOfChannels); channelIterator++){
      // check the up to 3 channels connected to this controller
      if(channels[channelIterator]->isUpdated()){updateNeeded = true;}
    }
    if(updateNeeded){
      // trigger data update if the current controller has a channel with updates
      uint16_t values[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      uint8_t channelOnThisControllerIterator = 0;
      for(uint8_t channelIterator = controllerIterator * 3; channelIterator < ((controllerIterator+1) * 3) && (channelIterator < this->numberOfChannels); channelIterator++){
        // aggregate info from each channel
        uint8_t off = channelOnThisControllerIterator * 5; // offset into values array
        channels[channelIterator]->getCurrentColors(values[0+off], values[1+off], values[2+off], values[3+off], values[4+off]); // update values array for one channel
        channelOnThisControllerIterator++;
      }
      // send update to controller
      controllers[controllerIterator]->updateOutputs(15, values);
    }
  }
}

Channel* Channelmanager::getChannel(uint8_t channelNumber){
  return channels[channelNumber];
}

Controller* Channelmanager::getController(uint8_t controllerNumber){
  return controllers[controllerNumber];
}

// request update from all channels (during initialization)
void Channelmanager::requestUpdate(){
  for(uint8_t channelIterator = 0; channelIterator < this->numberOfChannels; channelIterator++){
    channels[channelIterator]->requestUpdate();
  }
}
