// interaction with the physical controller through I2C

#include "controller.h"

Controller::Controller(){
}

void Controller::init(){
  this->pca9685.begin();
}

void Controller::updateOutputs(uint8_t count, uint16_t pwmvalue[]){
  Serial.println("Sending values to physical controller.");
  this->pca9685.setOutputs(count, pwmvalue);
}
