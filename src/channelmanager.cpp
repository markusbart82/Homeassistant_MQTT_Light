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
      // TODO: trigger data update if the current controller has a channel with updates
    }
  }
}

Channel* Channelmanager::getChannel(uint8_t channelNumber){
  return channels[channelNumber];
}

Controller* Channelmanager::getController(uint8_t controllerNumber){
  return controllers[controllerNumber];
}
