# AI Agent Instructions for Homeassistant MQTT Light

## Project Overview
Homeassistant-compatible MQTT Light implementation for RGB+CCT LED controllers. This project implements a smart light that mixes RGB colors with tunable white (warm/cold white), controlled via MQTT and Home Assistant autodiscovery.

**Key Innovation**: Enables "white with color tint" - a feature not natively supported by most smart home systems. The firmware calculates white mixing to preserve neutral tones.

## Technology Stack
- **Platform**: ESP8266 (esp12e) using PlatformIO and Arduino framework
- **Key Libraries**:
  - `PubSubClient` (^2.8) - MQTT client
  - `ArduinoJson` (^7.1.0) - JSON parsing for Home Assistant protocol
  - Custom `PCA9685` library (from `markusbart82/PCA9685` GitHub repo) - PWM driver control via I2C
- **Hardware**: Custom PCB with ESP8266, PCA9685 PWM driver, BLM3400 MOSFETs (ledcontroller_3x)

## Build & Development
- **Build Tool**: PlatformIO
- **Monitor**: `platformio device monitor` (115200 baud, ESP8266 exception decoder enabled)
- **Build Target**: `env:esp12e` (configured in [platformio.ini](platformio.ini))
- **Upload Method**: esptool via NodeMCU reset

**Key Pin Mapping** (from [src/main.cpp](src/main.cpp)):
- OE_PIN = 16 (PWM output enable, active low)
- SCL_PIN = 5, SDA_PIN = 4 (I2C for PCA9685)
- I2C clock: 400kHz (PCA9685 supports up to 1MHz)
- PCA9685 PWM frequency: 1kHz (12-bit resolution)

## Code Architecture

### Core Components

- **[Controller](include/controller.h)** - Manages PCA9685 PWM IC (16 channels, 12-bit, I2C)
- **[Channelmanager](include/channelmanager.h)** - Manages LED channels across multiple controllers (up to 9 channels, 3 controllers)
- **[Colorhandler](include/colorhandler.h)** - Color space conversions & mixing logic
- **[Messagehandler](include/messagehandler.h)** - MQTT message parsing (processing moved from callback to main loop - see TODO)
- **[Channel](include/channel.h)** - Individual channel state (brightness, transitions, effects)
- **[types](include/types.h)** - Shared type definitions (effect_t enum)

### Data Flow
1. **WiFi/MQTT** → `receiveMessage()` callback in [src/main.cpp](src/main.cpp)
2. **Message parsing** → [Messagehandler](include/messagehandler.h) (JSON from Home Assistant)
3. **Color conversion** → [Colorhandler](include/colorhandler.h) (RGB + temperature → RGBWC)
4. **Channel updates** → [Channelmanager](include/channelmanager.h) (brightness, transitions)
5. **PWM output** → [Controller](include/controller.h) → PCA9685 → LEDs

### PWM Channel Mapping
PCA9685 channels (I2C 12-bit PWM, 1kHz):
- Channel 0: Red
- Channel 1: Green
- Channel 2: Blue
- Channel 3: Warm White
- Channel 4: Cold White

## Color Mixing Algorithm
The firmware implements subtractive color mixing to combine RGB color with tunable white:

1. **White extraction**: `whiteContent = min(R, G, B)` - extracts the achromatic component
2. **Color separation**: `R_color = R - whiteContent`, etc. - leaves only the chromatic component
3. **White distribution**: WW and CW values calculated from color temperature (142-435 mireds)
   - WW ratio = (CW_TEMP - ct) / (CW_TEMP - WW_TEMP)
   - CW ratio = (ct - WW_TEMP) / (CW_TEMP - WW_TEMP)
   - `WW = ww_ratio * whiteContent`, `CW = cw_ratio * whiteContent`

**Data flow**: Home Assistant RGB (8-bit) + mireds → white mixing → 12-bit PWM scaling → PCA9685

**Key constants**: WW_TEMP=2300K (435 mireds), CW_TEMP=7000K (142 mireds)

**Invisible Temperature Updates**: Send `brightness=1` with desired temperature to update without changing lamp state (used for initialization).

## Home Assistant Integration
- **Feature Set**: JSON schema, ON/OFF, brightness, color temp, RGB, transitions, flashing
- **Autodiscovery**: Identifies as "ESP-dimmer-{MAC_ADDRESS}"
- **MQTT Topics**: Configured via autodiscovery protocol
- **Effects Supported**: Colorwheel (RGB spectrum), Undulation (random near-color)

## Effects
- **Colorwheel**: Slowly changes through the RGB spectrum
- **Undulation**: Slowly undulates randomly near the previously selected color

## Fallback Mode
As a fallback when Home Assistant is unavailable:
- On powerup, the lamp turns on with neutral white
- WiFi/MQTT reconnection is handled automatically in the main loop

## Known Issues & TODO Items
See [TODO.md](TODO.md) for complete list:
- Fix random WiFi reconnects
- ColorHandler::rgbww12ToRgb8() causes exceptions (investigate main.cpp usage)
- MessageHandler: Move message processing from MQTT callback to main loop (add buffer & busy flag)
- ChannelManager needs debug cleanups
- Dynamic channel count (currently hardcoded to 3)

## Configuration
- **WiFi/MQTT Credentials**: [include/secrets.h](include/secrets.h) (must be user-provided)
- **Hardware Settings**: Pin definitions in [src/main.cpp](src/main.cpp)

## Testing
- Test directory exists ([test/README](test/README)) but currently minimal
- Recommend testing color mixing logic and MQTT message handling

## When Modifying...

- **Color logic**: Edit [colorhandler.cpp](src/colorhandler.cpp) - verify mixing algorithm with test cases
- **Channel/brightness**: Edit [channelmanager.cpp](src/channelmanager.cpp) and [channel.cpp](src/channel.cpp)
- **MQTT/Home Assistant**: Edit [messagehandler.cpp](src/messagehandler.cpp) - follow JSON schema strictly
- **I2C/PWM**: Edit [controller.cpp](src/controller.cpp) - verify timing and channel assignments
- **Startup/Main Loop**: Edit [main.cpp](src/main.cpp) - mind the MQTT callback context for message receiving

---

**Last Updated**: 2026-06-19  
**For detailed project context**, see [README.md](README.md)
