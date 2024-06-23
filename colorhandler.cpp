// conversions between 8 and 12 bit, conversions between RGB+CCT and RGBWW

#include "colorhandler.h"

uint16_t kelvinToMireds(uint16_t kelvin){
  uint16_t kelvin_int = kelvin;
  if(kelvin_int < WW_TEMP){kelvin_int = WW_TEMP;}
  if(kelvin_int > CW_TEMP){kelvin_int = CW_TEMP;}
  return (1000000/kelvin);
}

uint16_t miredsToKelvin(uint16_t mireds){
  uint16_t kelvin = 1000000/mireds;
  if(kelvin < WW_TEMP){kelvin = WW_TEMP;}
  if(kelvin > CW_TEMP){kelvin = CW_TEMP;}
  return kelvin;
}

void Colorhandler::rgb8ToRgbww12(uint8_t r1, uint8_t g1, uint8_t b1, uint16_t ct, uint16_t &r2, uint16_t &g2, uint16_t &b2, uint16_t &ww2, uint16_t &cw2){
  // calculate values for WW and CW if they were at full brightness (still INPUT values, so 8 bit range)
  uint8_t cWfull = ((255 * (ct - WW_TEMP)) / (CW_TEMP - WW_TEMP));
  uint8_t wWfull = (255 - cWfull);
  // calculate white content in RGB
  uint8_t whiteContent = min(r1, min(g1, b1));
  // calculate 12 bit values using the translation table
  r2 = Colorhandler::pwmTable[(r1 - whiteContent)];
  g2 = Colorhandler::pwmTable[(g1 - whiteContent)];
  b2 = Colorhandler::pwmTable[(b1 - whiteContent)];
  ww2 = Colorhandler::pwmTable[wWfull * whiteContent / 255];
  cw2 = Colorhandler::pwmTable[cWfull * whiteContent / 255];
}

void Colorhandler::rgbww12ToRgb8(uint16_t r1, uint16_t g1, uint16_t b1, uint16_t ww1, uint16_t cw1, uint8_t &r2, uint8_t &g2, uint8_t &b2, uint16_t &ct){
  // step 1: rgbww12 to rgbww8
  uint8_t ww2 = 0;
  uint8_t cw2 = 0;
  rgbww12ToRgbww8(r1, g1, b1, ww1, cw1, r2, g2, b2, ww2, cw2);
  // step 2: rgbww8 to rgb8
  uint8_t whiteContent = (ww2+cw2) / 2;
  r2 += whiteContent;
  g2 += whiteContent;
  b2 += whiteContent;
  // step 3: normalize ww/cw values to a sum of 255 for (ww + cw)
  ww2 = ww2 * 255 / (ww2 + cw2);
  cw2 = cw2 * 255 / (ww2 + cw2);
  // step 4: calculate color temperature in kelvin
  ct = ((cw2 * CW_TEMP) + (WW_TEMP * (255 - cw2))) / 255;
}

void Colorhandler::rgbww8ToRgbww12(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t ww1, uint8_t cw1, uint16_t &r2, uint16_t &g2, uint16_t &b2, uint16_t &ww2, uint16_t &cw2){
  r2 = Colorhandler::pwmTable[r1];
  g2 = Colorhandler::pwmTable[g1];
  b2 = Colorhandler::pwmTable[b1];
  ww2 = Colorhandler::pwmTable[ww1];
  cw2 = Colorhandler::pwmTable[cw1];
}

void Colorhandler::rgbww12ToRgbww8(uint16_t r1, uint16_t g1, uint16_t b1, uint16_t ww1, uint16_t cw1, uint8_t &r2, uint8_t &g2, uint8_t &b2, uint8_t &ww2, uint8_t &cw2){
  r2 = s12to8(r1);
  g2 = s12to8(g1);
  b2 = s12to8(b1);
  ww2 = s12to8(ww1);
  cw2 = s12to8(cw1);
}

uint8_t Colorhandler::s12to8(uint16_t inVal){
  // step 1: divide by 16 to get initial position in array
  uint8_t retVal = (uint8_t) (inVal/16);
  // step 2: search up/down the translation table to find the best match
  if(pwmTable[retVal] < inVal){
    // value too low, increase until closest match is found
    while((retVal < 255) && (pwmTable[retVal] < inVal)){
      retVal++;
    }
  }else{
    // value too high, decrease until closest match is found
    while((retVal>0) && (pwmTable[retVal] > inVal)){
      retVal--;
    }
  }
  return retVal;
}
