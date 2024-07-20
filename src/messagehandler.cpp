// parses incoming MQTT messages and extracts data, creates outgoing MQTT messages from internal data

#include "messagehandler.h"

// constructor
Messagehandler::Messagehandler(){
}

// get mac address to use as part of the device name
void Messagehandler::getMacAddress(){
  // this stupid mac address trickery seems to be necessary because there seems to be no working conversion between unsigned char[] and char[]
  unsigned char mac[6];
  WiFi.macAddress(mac);
  int macNum = mac[0]*100000 + mac[1]*10000 + mac[2]*1000 + mac[3]*100 + mac[4]*10 + mac[5];
  itoa(macNum, this->macAddressUid, 10);
}

// create and send autodiscovery message for given number of channels (plus the sum channel "0")
void Messagehandler::sendAutoDiscoveryMessage(PubSubClient client, uint8 numberOfChannels){
  // loop over all channels, send an autodiscovery message for each one
  for(uint8 channelNumber = 0; channelNumber <= numberOfChannels; channelNumber++){
    // get channel number as a char array
    itoa(channelNumber, chNum, 10);
    // assemble config topic
    strcpy(configTopic, topicPrefix);
    strcat(configTopic, macAddressUid);
    strcat(configTopic, chInfix);
    strcat(configTopic, chNum);
    strcat(configTopic, topicPostfixConfig);
    // assemble command topic
    strcpy(commandTopic, topicPrefix);
    strcat(commandTopic, macAddressUid);
    strcat(commandTopic, chInfix);
    strcat(commandTopic, chNum);
    strcat(commandTopic, topicPostfixCommand);
    // assemble and send  config message
    uint16_t configMessageLength = 0;
    configMessageLength += strlen(configMsgPart1);
    configMessageLength += strlen(configMsgPart2);
    configMessageLength += strlen(configMsgPart3);
    configMessageLength += strlen(configMsgPart4);
    configMessageLength += strlen(configMsgPart5);
    configMessageLength += strlen(configMsgPart6);
    configMessageLength += strlen(configMsgPart7);
    configMessageLength += (6 * strlen(macAddressUid)); // mac address is in the message 6 times
    configMessageLength += (4 * strlen(chInfix)); // channel infix is in the message 4 times
    configMessageLength += 4; // channel number (uint8) is in the message 4 times
    client.beginPublish(configTopic, configMessageLength, true);
    client.write((const unsigned char*) configMsgPart1, strlen(configMsgPart1));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) chInfix, strlen(chInfix));
    client.write(channelNumber);
    client.write((const unsigned char*) configMsgPart2, strlen(configMsgPart2));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) chInfix, strlen(chInfix));
    client.write(channelNumber);
    client.write((const unsigned char*) configMsgPart3, strlen(configMsgPart3));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) configMsgPart4, strlen(configMsgPart4));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) configMsgPart5, strlen(configMsgPart5));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) chInfix, strlen(chInfix));
    client.write(channelNumber);
    client.write((const unsigned char*) configMsgPart6, strlen(configMsgPart6));
    client.write((const unsigned char*) macAddressUid, strlen(macAddressUid));
    client.write((const unsigned char*) chInfix, strlen(chInfix));
    client.write(channelNumber);
    client.write((const unsigned char*) configMsgPart7, strlen(configMsgPart7));
    client.endPublish();
    // subscribe to command topic
    client.subscribe(commandTopic);
  }
}

// create and send state message for one channel (RGB + colortemp)
void Messagehandler::sendStateMessage(PubSubClient client, uint8 channelNumber, uint8 r, uint8 g, uint8 b, uint8 ct, effect_t fx){
  JsonDocument message;
  if(r==0 && g==0 && b==0){
    message["state"] = "OFF";
  }else{
    message["state"] = "ON";
    if(r==g && g==b){
      // if r,g,b are identical --> send brightness/colortemp
      message["brightness"] = (r+g+b)/3;
      message["color_temp"] = Colorhandler::kelvinToMireds(ct);
    }else{
      // if r,g,b are different --> send RGB
      message["color"]["r"] = r;
      message["color"]["g"] = g;
      message["color"]["b"] = b;
    }
    switch(fx){
      case colorwheel:
        message["effect"] = "colorwheel";
        break;
      case undulation:
        message["effect"] = "undulation";
        break;
      default:
        message["effect"] = "none";
        break;
    }
  }
  char output[128];
  serializeJson(message, output);
  // get channel number as a char array
  itoa(channelNumber, chNum, 10);
  // assemble state topic
  strcpy(stateTopic, topicPrefix);
  strcat(stateTopic, macAddressUid);
  strcat(stateTopic, chInfix);
  strcat(stateTopic, chNum);
  strcat(stateTopic, topicPostfixState);
  // send message
  client.publish(stateTopic,output,true);
}
