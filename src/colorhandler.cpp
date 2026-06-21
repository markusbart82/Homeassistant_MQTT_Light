// conversions between 8 and 12 bit, conversions between RGB+CCT and RGBWW

#include "colorhandler.h"

uint16_t Colorhandler::kelvinToMireds(uint16_t kelvin){
  uint16_t kelvin_int = kelvin;
  if(kelvin_int < WW_TEMP){kelvin_int = WW_TEMP;}
  if(kelvin_int > CW_TEMP){kelvin_int = CW_TEMP;}
  return (1000000u / kelvin);
}

uint16_t Colorhandler::miredsToKelvin(uint16_t mireds){
  uint16_t kelvin = 1000000u / mireds;
  if(kelvin < WW_TEMP){kelvin = WW_TEMP;}
  if(kelvin > CW_TEMP){kelvin = CW_TEMP;}
  return kelvin;
}

void Colorhandler::rgb8ToRgbww12(uint8_t r1, uint8_t g1, uint8_t b1, uint16_t ct, uint16_t &r2, uint16_t &g2, uint16_t &b2, uint16_t &ww2, uint16_t &cw2){
  // calculate values for WW and CW if they were at full brightness (still INPUT values, so 8 bit range)
  uint8_t cWfull = ((255u * (ct - WW_TEMP)) / (CW_TEMP - WW_TEMP));
  uint8_t wWfull = (255u - cWfull);
  // calculate white content in RGB
  uint8_t whiteContent = min(r1, min(g1, b1));
  // calculate 12 bit values using the translation table
  r2 = Colorhandler::pwmTableR[(r1 - whiteContent)];
  g2 = Colorhandler::pwmTableG[(g1 - whiteContent)];
  b2 = Colorhandler::pwmTableB[(b1 - whiteContent)];
  ww2 = Colorhandler::pwmTableWW[wWfull * whiteContent / 255u];
  cw2 = Colorhandler::pwmTableCW[cWfull * whiteContent / 255u];
}

void Colorhandler::rgbww12ToRgb8(uint16_t r1, uint16_t g1, uint16_t b1, uint16_t ww1, uint16_t cw1, uint8_t &r2, uint8_t &g2, uint8_t &b2, uint16_t &ct){
  // step 1: rgbww12 to rgbww8
  uint8_t ww2 = 0u;
  uint8_t cw2 = 0u;
  rgbww12ToRgbww8(r1, g1, b1, ww1, cw1, r2, g2, b2, ww2, cw2);
  // step 2: rgbww8 to rgb8
  uint8_t whiteContent = (ww2 + cw2) / 2u;
  r2 += whiteContent;
  g2 += whiteContent;
  b2 += whiteContent;
  // step 3: normalize ww/cw values to a sum of 255 for (ww + cw)
  // Guard against division by zero when both WW and CW are 0 (pure RGB mode)
  uint16_t wwCwSum = ww2 + cw2;
  if (wwCwSum > 0) {
    ww2 = ww2 * 255u / wwCwSum;
    cw2 = cw2 * 255u / wwCwSum;
  } else {
    // Pure RGB mode, no white - keep neutral values
    ww2 = 0;
    cw2 = 0;
  }
  // step 4: calculate color temperature in kelvin
  ct = ((cw2 * CW_TEMP) + (WW_TEMP * (255u - cw2))) / 255u;
}

void Colorhandler::rgbww8ToRgbww12(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t ww1, uint8_t cw1, uint16_t &r2, uint16_t &g2, uint16_t &b2, uint16_t &ww2, uint16_t &cw2){
  r2 = Colorhandler::pwmTableR[r1];
  g2 = Colorhandler::pwmTableG[g1];
  b2 = Colorhandler::pwmTableB[b1];
  ww2 = Colorhandler::pwmTableWW[ww1];
  cw2 = Colorhandler::pwmTableCW[cw1];
}

void Colorhandler::rgbww12ToRgbww8(uint16_t r1, uint16_t g1, uint16_t b1, uint16_t ww1, uint16_t cw1, uint8_t &r2, uint8_t &g2, uint8_t &b2, uint8_t &ww2, uint8_t &cw2){
  r2 = s12to8(r1, pwmTableR);
  g2 = s12to8(g1, pwmTableG);
  b2 = s12to8(b1, pwmTableB);
  ww2 = s12to8(ww1, pwmTableWW);
  cw2 = s12to8(cw1, pwmTableCW);
}

uint8_t Colorhandler::s12to8(uint16_t inVal, const uint16_t *pwmTable){
  // step 1: divide by 16 to get initial position in array
  uint8_t retVal = (uint8_t) (inVal/16u);
  uint8_t previousRetVal = retVal;
  // step 2: search up/down the translation table to find the best match
  if(pwmTable[retVal] < inVal){
    // value too low, increase until closest match is found
    while((retVal < 255u) && (pwmTable[retVal] < inVal)){
      previousRetVal = retVal;
      retVal++;
    }
  }else{
    // value too high, decrease until closest match is found
    while((retVal>0u) && (pwmTable[retVal] > inVal)){
      previousRetVal = retVal;
      retVal--;
    }
  }
  // as the search finds the first value over the match, not the closest match,
  // check if the step one earlier is better and use it instead if it is
  // MISRA-compliant absolute value calculation (unsigned types)
  uint16_t diffRetVal;
  if (pwmTable[retVal] > inVal) {
    diffRetVal = pwmTable[retVal] - inVal;
  } else {
    diffRetVal = inVal - pwmTable[retVal];
  }
  uint16_t diffPrevious;
  if (pwmTable[previousRetVal] > inVal) {
    diffPrevious = pwmTable[previousRetVal] - inVal;
  } else {
    diffPrevious = inVal - pwmTable[previousRetVal];
  }
  if (diffRetVal > diffPrevious) {
    retVal = previousRetVal;
  }
  return retVal;
}
