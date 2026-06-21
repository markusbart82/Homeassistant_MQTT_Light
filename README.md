# Homeassistant MQTT Light
Homeassistant compatible MQTT Light implementation for RGB+CCT LEDs. Uses ESP8266 and PCA9685.

The idea started because I wanted "white with a color tint" smart lights. But apparently, this is not supported by smart home tech yet. It's either RGB, or white, never a mix of these. So this lamp here identifies as RGB, but mixes white with it.

# How Color Mixing Works

This lamp can display both **colored light** and **tunable white** (from warm 2300K to cool 7000K), and uniquely, it can **mix white with color** — producing effects like "pink-tinted white" or "blue-tinted daylight" that most smart lights can't do.

## The Color Pipeline

Here's what happens internally when you send a color command:

1. **Input from Home Assistant**: You send RGB values (0-255) and a color temperature in mireds (142–435 mireds = 7000K–2300K)
2. **White extraction**: The algorithm finds the achromatic (gray/white) component by taking `min(R, G, B)` — this is the amount of white already present in the color
3. **Color separation**: The white component is subtracted from R, G, B, leaving only the pure chromatic (saturated) colors
4. **White distribution**: The extracted white is split between warm white (WW) and cold white (CW) LEDs based on the requested color temperature
5. **PWM output**: All 5 channels (R, G, B, WW, CW) are scaled to 12-bit resolution (0-4095) and sent to the PCA9685 PWM driver

## Examples

### Pure white with temperature
| Input | Output (R,G,B,WW,CW) |
|-------|----------------------|
| Warm white (2300K) | (0, 0, 0, 4095, 0) |
| Neutral white | (0, 0, 0, 2048, 2048) |
| Cool white (7000K) | (0, 0, 0, 0, 4095) |

### White with a color tint
| Input | What you see |
|-------|--------------|
| RGB(255,200,200) + 4000K | A soft pink-tinted white |
| RGB(200,220,255) + 5000K | A cool blue-tinted white |
| RGB(255,255,255) + 3500K | Pure warm white (no color tint) |

### Full color
| Input | What you see |
|-------|--------------|
| RGB(255,0,0) + any temp | Pure red (white channels off, `min(255,0,0)=0`) |
| RGB(128,64,32) + 3000K | Orange with warm white boost (`min(128,64,32)=32` → WW adds white) |

## Why This Matters

Traditional smart lights force you to choose: RGB *or* white. With this lamp, you get both simultaneously — perfect for ambient lighting where you want a colored accent light that still feels natural and bright.

# Invisible color temperature updates

Most smart lamps only let you update the color temperature while they are on. So they turn on with the previous color temperature and then dim to the right one. To combat this behavior, send a message with `brightness=1` and the desired color temperature to do an invisible update. The lamp will remain in its previous state (OFF, that is). Such low brightness values usually are not shown in most lamps, either.

# Homeassistant integration

The lamp supports autodiscovery and identifies as "ESP-dimmer-NUMBER", where the number is derived from the MAC address. Supported features:

- JSON schema
- ON/OFF
- Brightness
- Color temperature
- RGB
- Transitions
- Flashing

# Effects

The lamp supports the following effects:

- **Colorwheel** — slowly changes through the RGB spectrum
- **Undulation** — slowly undulates randomly near the previously selected color

# Stand-alone mode

As a fallback when Home Assistant is unavailable:

- On powerup, the lamp turns on with neutral white
- WiFi/MQTT reconnection is handled automatically in the main loop

# Hardware

The lamp uses the PCA9685 PWM driver IC (16 channels, 12 bits, up to 1.5kHz PWM, I2C at 400kHz) to interface with the LEDs. Currently, channels 0–4 are assigned R, G, B, WW, CW.

## Ledcontroller_3x

This PCB includes the ESP8266, PCA9685 and BLM3400 MOSFETs, for controlling 24V LED strips with a common +24V.
