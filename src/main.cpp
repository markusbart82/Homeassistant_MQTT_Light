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
  noInterrupts();
  // call messagehandler to parse the message
  messagehandler.parseMessage(topic, payload, length);

  // TODO: update channel(s) with data from the parsed message
  // check on/off state
  // check effect
  // check flashing
  // check transition
  // check brightness
  // check RGB
  interrupts();
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

  // init wifi
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  // read mac address from wifi for unique names
  messagehandler.getMacAddress();

  // configure MQTT broker
  pubSubClient.setServer(MQTT_BROKER, 1883);
  pubSubClient.setCallback(receiveMessage);

  // random number generator
  randomSeed(analogRead(A0));

  // turn off internal LED
  digitalWrite(LED_BUILTIN, HIGH);

  // enable outputs
  digitalWrite(OE_PIN, LOW);

  // TODO: do other initialization stuff
}


void loop() {
  // make sure we only communicate with an active connection
  if (!pubSubClient.connected()) {
    while (!pubSubClient.connected()) {
      pubSubClient.connect(messagehandler.getClientName(), MQTT_USER,MQTT_PASS);
      delay(100);
    }
    // send discovery message and subscribe to command topics
    messagehandler.sendAutoDiscoveryMessage(pubSubClient, 3); // TODO: make this dynamic - for now, it's 3 static channels
    // TODO: send initial state topic
  }

  // update channels, do all the dimming etc. and send commands to physical controllers
  channelmanager.loop();

  // TODO: do other repeating stuff
}

