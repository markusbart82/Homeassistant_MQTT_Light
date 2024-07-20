#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <PCA9685.h>


class Controller{
  
  private:
    PCA9685 pca9685;
    
  public:
    // constructor
    Controller();
    
    // initialize the controller
    void init();
    
    // update outputs
    void updateOutputs(uint8_t count, uint16_t pwmvalue[]);
};

#endif
