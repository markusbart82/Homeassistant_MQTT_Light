// one channel (default board has three, with multiple PCA9685 even more channels are possible)

#include "channel.h"

// constructor
Channel::Channel(){
}

// loop function that does all the repeating/timing stuff
void Channel::loop(){
  int now = millis();
  // overflow handling
  if(now - lastNow < 0){lastNow = now;}
  // act every 20ms
  if(now - lastNow > 20){
    lastNow = now;
    
    // do all the timing steps
    if(dimTime > 0){dimStep();}
    if(flashTime > 0){flashStep();}
  }
}

// set new target color to dim to, and time (in ms) to do so
void Channel::dimToColor(uint16_t r, uint16_t g, uint16_t b, uint16_t ww, uint16_t cw, uint16_t time){
  // set current values as previous to initiate new dimming cycle
  this->previousR = this->currentR;
  this->previousG = this->currentG;
  this->previousB = this->currentB;
  this->previousWW = this->currentWW;
  this->previousCW = this->currentCW;
  // set target color
  this->targetR = r;
  this->targetG = g;
  this->targetB = b;
  this->targetWW = ww;
  this->targetCW = cw;
  // set dim time in 20ms steps
  this->dimTime = (time/20);
  this->dimStart = (time/20);
}

// set color to flash, and time (in ms) to flash it for
void Channel::flashColor(uint16_t r, uint16_t g, uint16_t b, uint16_t ww, uint16_t cw, uint16_t time){
  // set color to flash with
  this->flashR = r;
  this->flashG = g;
  this->flashB = b;
  this->flashWW = ww;
  this->flashCW = cw;
  // set flashing time in 20ms steps
  this->flashTime = (time/20);
  this->flashStart = (time/20);
}

// set effect
void Channel::setEffect(effect_t fx){
  this->fx = fx;
}

// get updated colors
void Channel::getCurrentColors(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &ww, uint16_t &cw){
  r = this->currentR;
  g = this->currentG;
  b = this->currentB;
  ww = this->currentWW;
  cw = this->currentCW;
}

bool Channel::isUpdated(){
  if(this->updated){
    this->updated = false;
    return true;
  }else{
    return false;
  }
}

// manually request update (set the updated flag)
void Channel::requestUpdate(){
  this->updated = true;
}

// do one step of dimming, called every 20ms
void Channel::dimStep(){
  // dim one step further (20ms)
  // new color = ((target - previous) * progress) + previous
  // progress = (startTime - currentTime) / startTime
  this->currentR = ((this->targetR - this->previousR) * (dimStart - dimTime) / dimStart) + this->previousR;
  this->currentG = ((this->targetG - this->previousG) * (dimStart - dimTime) / dimStart) + this->previousG;
  this->currentB = ((this->targetB - this->previousB) * (dimStart - dimTime) / dimStart) + this->previousB;
  this->currentWW = ((this->targetWW - this->previousWW) * (dimStart - dimTime) / dimStart) + this->previousWW;
  this->currentCW = ((this->targetCW - this->previousCW) * (dimStart - dimTime) / dimStart) + this->previousCW;
  
  dimTime--;
  this->updated = true;
}

// do one step of flashing, calles every 20ms
void Channel::flashStep(){
  // this is called every 20ms but color only changes every 500ms for flashing, so every 25 steps
  // new color = (flashTime % 25 <= 12) ? (flash) : (dark)
  this->currentR = (flashTime % 25 <= 12) ? this->flashR : 0;
  this->currentG = (flashTime % 25 <= 12) ? this->flashG : 0;
  this->currentB = (flashTime % 25 <= 12) ? this->flashB : 0;
  this->currentWW = (flashTime % 25 <= 12) ? this->flashWW : 0;
  this->currentCW = (flashTime % 25 <= 12) ? this->flashCW : 0;
  
  flashTime--;
  this->updated = true;
}


