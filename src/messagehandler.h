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
  
  public:
    // constructor
    Messagehandler();
    
    // get mac address to use as part of the device name
    void getMacAddress();
    
    // create and send autodiscovery message for given number of channels (plus the sum channel "0")
    void sendAutoDiscoveryMessage(PubSubClient client, uint8 numberOfChannels);
    
    // create and send state message for one channel (RGB + colortemp)
    void sendStateMessage(PubSubClient client, uint8 channelNumber, uint8 r, uint8 g, uint8 b, uint8 ct, effect_t fx);
    
    // parse command message and extract commands from it
    // TODO: find a good way to extract all necessary information for other classes to use
    //void parseMessage(char* topic, byte* payload, unsigned int length, ???);

    // TODO: upgrade autodiscovery to announce individually controllable channels

};

#endif
