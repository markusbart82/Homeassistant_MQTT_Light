// interaction with the physical controller through I2C

#include "controller.h"

Controller::Controller(){
}

void Controller::init(){
  this->pca9685.begin();
}

void Controller::updateOutputs(uint8_t count, uint16_t pwmvalue[]){
  this->pca9685.setOutputs(count, pwmvalue);
}
