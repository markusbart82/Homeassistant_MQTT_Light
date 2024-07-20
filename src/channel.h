#ifndef CHANNEL_H
#define CHANNEL_H

#include <Arduino.h>

enum effect_t {
  none = 0,
  colorwheel = 1,
  undulation = 2
};


class Channel{

  private:
    // time keeping
    int lastNow = 0;
    
    // detect if colors have changed (i.e. an update is necessary)
    bool updated = true;

    // current colors (also during dimming)
    volatile uint16_t currentR = 0;
    volatile uint16_t currentG = 0;
    volatile uint16_t currentB = 0;
    volatile uint16_t currentWW = 2047;
    volatile uint16_t currentCW = 2047;
    
    // target colors (also flashing color, after flashing this will be replaced by previous colors)
    volatile uint16_t targetR = 0;
    volatile uint16_t targetG = 0;
    volatile uint16_t targetB = 0;
    volatile uint16_t targetWW = 2047;
    volatile uint16_t targetCW = 2047;
    
    // previous colors (for dimming or flashing)
    volatile uint16_t previousR = 0;
    volatile uint16_t previousG = 0;
    volatile uint16_t previousB = 0;
    volatile uint16_t previousWW = 2047;
    volatile uint16_t previousCW = 2047;
    
    // flashing color during flashing
    volatile uint16_t flashR = 0;
    volatile uint16_t flashG = 0;
    volatile uint16_t flashB = 0;
    volatile uint16_t flashWW = 2047;
    volatile uint16_t flashCW = 2047;
    
    // progress values start at defined values and decrement until zero
    // increments are 20ms, a dim time of e.g. 2s would be 100 steps
    // dimming/flashing is active while the value is non zero
    volatile uint16_t dimTime = 0; // used for dim value calculation
    volatile uint16_t dimStart = 0; // start value as a reference for progress
    volatile uint16_t flashTime = 0; // used for flashing timing calculation
    volatile uint16_t flashStart = 0; // start value as a reference for progress
    
    // effects
    volatile effect_t fx = none;

  public:
    // constructor
    Channel();

    // loop function that does all the repeating/timing stuff
    void loop();
    
    // set new target color to dim to, and time (in ms) to do so
    void dimToColor(uint16_t r, uint16_t g, uint16_t b, uint16_t ww, uint16_t cw, uint16_t time);
    
    // set color to flash, and time (in ms) to flash it for
    void flashColor(uint16_t r, uint16_t g, uint16_t b, uint16_t ww, uint16_t cw, uint16_t time);
    
    // set effect
    void setEffect(effect_t fx);
    
    // get updated colors
    void getCurrentColors(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &ww, uint16_t &cw);
    
    // reads and clears the updated flag (for checking whether new data has to be sent to controller)
    bool isUpdated();
    
  private:
    // do one step of dimming, called every 20ms
    void dimStep();
    
    // do one step of flashing, calles every 20ms
    void flashStep();
};

#endif
