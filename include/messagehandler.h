#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include "colorhandler.h"
#include "channel.h"

class Messagehandler{

  private:
// {
// "uniq_id":"esp_dimmer_MACADDRESS_ch0",
// "name":"esp dimmer MACADDRESS channel 0",
// "schema":"json",
// "brightness":true,
// "min_mirs":142,
// "max_mirs":435,
// "sup_clrm":["color_temp","rgb"],
// "effect":true,
// "fx_list":["none","colorwheel","undulation"],
// "dev":
// {
  // "name":"esp dimmer MACADDRESS",
  // "ids":"MACADDRESS"
// },
// "stat_t":"homeassistant/light/esp_dimmer_MACADDRESS_ch0/state",
// "cmd_t":"homeassistant/light/esp_dimmer_MACADDRESS_ch0/set"
// }

    // topic snippets
    const char * topicPrefix = "homeassistant/light/esp_dimmer_";
    char macAddressUid[33] = "\0"; // must be set by getMacAddress() before using MQTT communication
    const char * chInfix = "_ch";
    // channel number goes here
    const char * topicPostfixConfig = "/config";
    const char * topicPostfixState = "/state";
    const char * topicPostfixCommand = "/set";
    // variables used for dynamic creation of the changing topics
    char chNum[5] = "\0";
    char configTopic[128] = "\0";
    char stateTopic[128] = "\0";
    char commandTopic[128] = "\0";
    // name for MQTT client
    const char * clientNamePrefix = "esp_dimmer_";
    char mqttClientName[20] = "\0";
    
    // config message snippets (send one per channel)
    const char * configMsgPart1 = "{\"uniq_id\":\"esp_dimmer_";
    // mac address goes here
    // chInfix goes here
    // channel number goes here
    const char * configMsgPart2 = "\",\"name\":\"esp dimmer ";
    // mac address goes here
    // chInfix goes here
    // channel number goes here
    const char * configMsgPart3 = "\",\"schema\":\"json\",\"brightness\":true,\"min_mirs\":142,\"max_mirs\":435,\"sup_clrm\":[\"color_temp\",\"rgb\"],\"effect\":true,\"fx_list\":[\"none\",\"colorwheel\",\"undulation\"],\"dev\":{\"name\":\"esp dimmer ";
    // mac address goes here
    const char * configMsgPart4 = "\",\"ids\":[\"";
    // mac address goes here
    const char * configMsgPart5 = "\"]},\"stat_t\":\"homeassistant/light/esp_dimmer_";
    // mac address goes here
    // chInfix goes here
    // channel number goes here
    const char * configMsgPart6 = "/state\",\"cmd_t\":\"homeassistant/light/esp_dimmer_";
    // mac address goes here
    // chInfix goes here
    // channel number goes here
    const char * configMsgPart7 = "/set\"";

    enum effect_t {
      none = 0,
      colorwheel = 1,
      undulation = 2
    };

    // buffer for last received message (to get info from using separate APIs)
    // TODO: make all access to this buffer interrupt-locked
    uint8_t bufferChannelNumber = 0; // channel number this message is for
    uint8_t bufferR = 0; // red
    uint8_t bufferG = 0; // green
    uint8_t bufferB = 0; // blue
    uint16_t bufferCT = 0; // color temperature [mireds]
    uint8_t bufferBrightness = 0; // brightness
    uint8_t bufferFlashTime = 0; // time for flashing target color
    uint8_t bufferTransitionTime = 0; // time for transition to target color
    effect_t bufferEffect = none;
    bool bufferState = false; // on/off state, stored as true/false
  
  public:
    // constructor
    Messagehandler();

    // initialization of internal structures
    void init();
    
    // get mac address to use as part of the device name (and set client name)
    void getMacAddress();
    
    // create and send autodiscovery message for given number of channels (plus the sum channel "0") and subscribe to command topics
    // TODO: upgrade autodiscovery to announce individually controllable channels
    void sendAutoDiscoveryMessage(PubSubClient client, uint8 numberOfChannels);
    
    // create and send state message for one channel (RGB + colortemp)
    void sendStateMessage(PubSubClient client, uint8 channelNumber, uint8 r, uint8 g, uint8 b, uint8 ct, effect_t fx);
    
    // parse command message and extract commands from it
    void parseMessage(char* topic, byte* payload, unsigned int length);

    // TODO: APIs to get information from last (buffered) message - must be called with locked interrupts to ensure data consistency

    // return client name for use externally
    char * getClientName();

};

#endif
