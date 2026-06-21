#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <PCA9685.h>
#include "ArduinoJson.h"
#include "secrets.h"
#include "channel.h"
#include "channelmanager.h"
#include "colorhandler.h"
#include "messagehandler.h"
#include "controller.h"
#include "types.h"


// output enable (low = enabled)
#define OE_PIN 16

// I2C
#define SCL_PIN 5
#define SDA_PIN 4

Channelmanager channelmanager(3); // TODO: make this dynamic - for now, it's 3 static channels
Messagehandler messagehandler;
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

// this function is the entry point for received messages
void receiveMessage(char* topic, byte* payload, unsigned int length) {
  Serial.println("receiveMessage");

  /* HomeAssistant will send these messages to the lamp using the UI:
   * OFF
   * ON
   * ON brightness
   * ON colortemp
   * ON RGB
   * ON effect
   * 
   * HomeAssistant will send these messages to the lamp using scripts:
   * ON brightness colortemp
   * ON transition
   * OFF transition
   * ON transition brightness
   * ON transition colortemp
   * ON transition brightness colortemp
   * ON flash RGB
   * ON flash brightness
   * ON flash colortemp
   * ON flash brightness colortemp
   * 
   * HomeAssistant will NOT send these messages:
   * OFF colortemp
   * ON brightness=0 colortemp
   * 
   * This lamp will use this message format to change the colortemp without turning on the lamp:
   * ON brightness=1 colortemp
   * And after the call, it reports back "OFF"
   */

  //TODO: move all of the following into the main loop, inside messageHandler.parseMessage, and have a buffer for the received values
  
  // call messagehandler to parse the message
  messagehandler.parseMessage(topic, payload, length);

  // temporary values for updating the channel
  uint16_t r = 0;
  uint16_t g = 0;
  uint16_t b = 0;
  uint16_t ww = 0;
  uint16_t cw = 0;
  uint8_t channelNumber = messagehandler.getChannelNumber();
  boolean writeUpdate = true; // set this variable to false if an update is not required

  // update channel(s) with data from the parsed message
  Channel* channel = channelmanager.getChannel(channelNumber);
  // check on/off state
  if(messagehandler.getState()){
    Serial.println("state is ON");
    // state is "ON"

    if(messagehandler.getEffect() != none){
      Serial.println("has effect");
      
      // if an effect is selected, use that
      channel->setEffect(messagehandler.getEffect());

    }else{
      Serial.println("has no effect");
      
      // if no effect is selected, disable effects and proceed
      channel->setEffect(none);

      // if color temperature is included, update the stored value
      if(messagehandler.getColortemp()!=UINT16_MAX){
        channel->setColorTemp(messagehandler.getColortemp());
      }

      // calculate colors to use for dim/flash
      if(messagehandler.getBrightness()>0){
        Serial.println("using brightness mode");
        // brightness is part of the message, go for uncolored mode
        if(messagehandler.getBrightness()==1){
          // special message to update color temperature and change nothing in the actual light
          writeUpdate = false; // do nothing, no changes to the light, just update the color temperature
        }else{
          // brightness >1, use it for RGB with stored color temperature
          uint8_t brightness = messagehandler.getBrightness();
          Colorhandler::rgb8ToRgbww12(brightness, brightness, brightness, channel->getColorTemp(), r, g, b, ww, cw);
        }
      }else if(messagehandler.getRed()>0 || messagehandler.getGreen()>0 || messagehandler.getBlue()>0){
        Serial.println("using RGB mode");
        // RGB is set, use it with stored color temperature
        Colorhandler::rgb8ToRgbww12(messagehandler.getRed(), messagehandler.getGreen(), messagehandler.getBlue(), channel->getColorTemp(), r, g, b, ww, cw);
      }else{
        // just "ON" command
        Colorhandler::rgb8ToRgbww12(255u, 255u, 255u, channel->getColorTemp(), r, g, b, ww, cw);
      }

      // dim or flash the lamp accordingly
      if(writeUpdate){ // skip the update for the invisible colortemp update
        if(messagehandler.getFlashTime()>0){
          Serial.println("flashing");
          // flashing
          channel->flashColor(r, g, b, ww, cw, messagehandler.getFlashTime());
        }else{
          Serial.println("dimming");
          // dimming
          channel->dimToColor(r, g, b, ww, cw, messagehandler.getTransitionTime());
        }
      }
    }
  }else{
    Serial.println("state is OFF");
    // state is "OFF"
    // turn off the lamp
    channel->dimToColor(0, 0, 0, 0, 0, messagehandler.getTransitionTime());
  }

  // send state message back to HomeAssistant server
  // convert to 8bit values before reporting them back to home assistant
  uint8_t r8 = 0;
  uint8_t g8 = 0;
  uint8_t b8 = 0;
  uint16_t ct16 = 0;
  Colorhandler::rgbww12ToRgb8(r, g, b, ww, cw, r8, g8, b8, ct16);
  messagehandler.sendStateMessage(pubSubClient, channelNumber, r8, g8, b8, ct16, messagehandler.getEffect());
  
}

void setup() {
  // configure pins
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDA_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // start I2C bus
  Wire.begin();
  Wire.setClock(400000); // PCA9685 supports up to 1 MHz (yet we currently use 400kHz for debugging)
  
  // init serial console
  Serial.begin(115200);
  delay(100);

  // init channel manager (thereby initializing physical controllers)
  channelmanager.init();

  // init wifi
  Serial.println("Connecting to WIFI");
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("WIFI connection established");

  // read mac address from wifi for unique names
  messagehandler.init();
  messagehandler.getMacAddress();

  // configure MQTT broker
  pubSubClient.setServer(MQTT_BROKER, 1883);
  pubSubClient.setCallback(receiveMessage);
  pubSubClient.setBufferSize(500);

  // random number generator
  randomSeed(analogRead(A0));

  // turn off internal LED
  digitalWrite(LED_BUILTIN, HIGH);

  // enable outputs
  digitalWrite(OE_PIN, LOW);

  // TODO: do other initialization stuff
}


void loop() {
  // make sure we only communicate with active WIFI
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WIFI");
    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    Serial.println("WIFI connection established");
  }
  // make sure we only communicate with an active MQTT connection
  if (!pubSubClient.connected()) {
    Serial.println("Connecting to MQTT");
    while (!pubSubClient.connected()) {
      pubSubClient.connect(messagehandler.getClientName(), MQTT_USER,MQTT_PASS);
      delay(100);
    }
    Serial.println("MQTT connection established");
    Serial.println("Sending auto discovery message to Home Assistant");
    // send discovery message and subscribe to command topics
    messagehandler.sendAutoDiscoveryMessage(pubSubClient, 3u); // TODO: make this dynamic - for now, it's 3 static channels
    // request update for all channels, the next loop will send current states of all channels to the controller(s)
    channelmanager.requestUpdate();
    Serial.println("Home Assistant configured");
  }

  // update channels, do all the dimming etc. and send commands to physical controllers
  channelmanager.loop();

  // parse incoming messages for MQTT
  pubSubClient.loop();

  // TODO: do other repeating stuff
}
