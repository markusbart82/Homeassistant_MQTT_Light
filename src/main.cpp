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

  // random number generator
  randomSeed(analogRead(A0));

  // turn off internal LED
  digitalWrite(LED_BUILTIN, HIGH);

  // enable outputs
  digitalWrite(OE_PIN, LOW);

}


void loop() {
}

