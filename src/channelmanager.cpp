// functions for messagehandler and others to call, to set states etc. specific to channels

#include "channelmanager.h"

Channelmanager::Channelmanager(uint8_t numberOfChannels){
  this->numberOfChannels = numberOfChannels;
  this->numberOfControllers = 0;
  for(uint8_t channelIterator = 0; channelIterator < this->numberOfChannels; channelIterator++){
    this->channels[channelIterator] = new Channel();
    // create one controller each time the next batch of 3 channels is started
    if(channelIterator % 3 == 0){
      this->controllers[channelIterator / 3] = new Controller();
      this->numberOfControllers++;
    }
  }
}

void Channelmanager::init(){
  for(int controllerIterator = 0; controllerIterator < this->numberOfControllers; controllerIterator++){
    this->controllers[controllerIterator]->init();
  }
}

void Channelmanager::loop(){
  int now = millis();
  // overflow handling
  if(now - lastNow < 0){lastNow = now;}
  // act every 20ms
  if(now - lastNow > 20){
    lastNow = now;
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
        Serial.print("Controller ");
        Serial.print(controllerIterator);
        Serial.print(" update needed: ");
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
        Serial.print(values[0]);
        Serial.print(",");
        Serial.print(values[1]);
        Serial.print(",");
        Serial.print(values[2]);
        Serial.print(",");
        Serial.print(values[3]);
        Serial.print(",");
        Serial.print(values[4]);
        Serial.print(",");
        Serial.print(values[5]);
        Serial.print(",");
        Serial.print(values[6]);
        Serial.print(",");
        Serial.print(values[7]);
        Serial.print(",");
        Serial.print(values[8]);
        Serial.print(",");
        Serial.print(values[9]);
        Serial.print(",");
        Serial.print(values[10]);
        Serial.print(",");
        Serial.print(values[11]);
        Serial.print(",");
        Serial.print(values[12]);
        Serial.print(",");
        Serial.print(values[13]);
        Serial.print(",");
        Serial.println(values[14]);
        controllers[controllerIterator]->updateOutputs(15, values);
      }
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
