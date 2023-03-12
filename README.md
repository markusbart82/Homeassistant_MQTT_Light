# Homeassistant MQTT Light
Homeassistant compatible MQTT Light implementation for RGB+CCT LEDs.

The idea started because I wanted "white with a color tint" smart lights. But apparently, this is not supported by smart home tech yet. It's either RGB, or white, never a mix of these. So this lamp here identifies as RGB, but mixes white with it.

# Color mixing
To mix colors, the lamp takes the minimum of the three R,G,B colors, and replaces it with white.
* RGB(255,255,255) --> RGBW(0,0,0,255)
* RGB(255,128,0) --> RGBW(255,128,0,0)
* RGB(255,192,128) --> RGBW(128,64,0,128)

# White mixing
In addition, this lamp supports tuneable white, so mixing warm white and cold white with everything else. In order to make Homeassistant use this, the lamp remembers the last color temperature setting and uses this.
* RGB(255,255,255),TEMP(255) --> RGBWW(0,0,0,0,255)
* RGB(255,255,255),TEMP(0) --> RGBWW(0,0,0,255,0)
* RGB(255,128,0),TEMP(255) --> RGBWW(255,128,0,0,0)
* RGB(255,192,128),TEMP(64) --> RGBWW(128,64,0,192,64)
