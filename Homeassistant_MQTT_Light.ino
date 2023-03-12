// uses these libraries:
// ESP8266WiFi
// PubSubClient
// ArduinoJson
// PCA9685 (https://github.com/markusbart82/PCA9685)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <PCA9685.h>
#include "ArduinoJson.h"
#include "secrets.h"

/*
 * TODO:
 * - add multiple lamps capability with PCA9685 controller (using effects defined in home assistant MQTT Light)
 */

WiFiClient espClient;
PubSubClient client(espClient);
PCA9685 pwm0;

#define NODE_DEBUG false

#define ON 1
#define OFF 0

// GPIO configuration
#define SCL_PIN 5
#define SDA_PIN 4
//#define WW_PIN 5
//#define CW_PIN 4
//#define R_PIN 14
//#define G_PIN 12
//#define B_PIN 13

// LED strip white color temperatures in Kelvin
#define WW_TEMP 2300
#define CW_TEMP 7000

// valid PWM values go up to this number
#define PWM_RANGE 4095 // 4095 means 12 bit precision

// global variables

// module name, schema: topicPrefix namePrefix uid topicPostfix
char namePrefix[] = "ESP-dimmer-";
char macAddressUid[33]; // placeholder, filled on init after WiFi object exists
char topicPrefix[] = "homeassistant/light/";
char topicPostfixConfig[] = "/config";
char topicPostfixState[] = "/state";
char topicPostfixCommand[] = "/set";
// and the placeholders, filled on init
char clientName[20];
char configTopic[128];
char stateTopic[128];
char commandTopic[128];
char configMessage0[128];
char configMessage1[128];
char configMessage2[128];
char configMessage3[128];
char configMessage4[128];
char configMessage5[128];

// input brightness values as read from MQTT [0..255]
volatile int valueRin=255;
volatile int valueGin=255;
volatile int valueBin=255;
volatile int valueCTin=215; // in mireds
volatile int valueCTK=4650; // in kelvin

// PWM related values, dimming occurs start (= current) to end (= new values just set by incoming message)
volatile int pwmStartValues[5] = {0,0,0,0,0}; // 0..PWM_RANGE, the brightness of each channel. Ordered RGBWC.
volatile int pwmEndValues[5] = {0,0,0,PWM_RANGE/2,PWM_RANGE/2}; // 0..PWM_RANGE, the brightness of each channel. Ordered RGBWC.

// dimming related values
volatile int dimValue=100; // must be AT LEAST 1, duration of the dim in ms
volatile int dimProgress = 1; // set to dimValue to start transition, decrements by 1 every ms
volatile int dimTimer = millis(); // variable used for timing the dim

// flashing related values
volatile bool flashBool = false; // this is toggled, keeps state of  lights while flashing
volatile int flashProgress = 0; // must be set to value >0 to start flashing, decrements by 1 every 100ms
volatile int flashTimer = millis(); // variable used for timing the color flashing action

// conversion table for mapping 8bit PWM inputs to 12bit PWM outputs for smooth, linear-looking PWM
// linear
const uint16_t pwmTable[] = {0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240,256,272,288,304,320,336,352,368,384,400,416,432,448,464,480,496,512,528,544,560,576,592,608,624,640,656,672,688,704,720,736,752,768,784,800,816,832,848,864,880,896,912,928,944,960,976,992,1008,1024,1040,1056,1072,1088,1104,1120,1136,1152,1168,1184,1200,1216,1232,1248,1264,1280,1296,1312,1328,1344,1360,1376,1392,1408,1424,1440,1456,1472,1488,1504,1520,1536,1552,1568,1584,1600,1616,1632,1648,1664,1680,1696,1712,1728,1744,1760,1776,1792,1808,1824,1840,1856,1872,1888,1904,1920,1936,1952,1968,1984,2000,2016,2032,2048,2064,2080,2096,2112,2128,2144,2160,2176,2192,2208,2224,2240,2256,2272,2288,2304,2320,2336,2352,2368,2384,2400,2416,2432,2448,2464,2480,2496,2512,2528,2544,2560,2576,2592,2608,2624,2640,2656,2672,2688,2704,2720,2736,2752,2768,2784,2800,2816,2832,2848,2864,2880,2896,2912,2928,2944,2960,2976,2992,3008,3024,3040,3056,3072,3088,3104,3120,3136,3152,3168,3184,3200,3216,3232,3248,3264,3280,3296,3312,3328,3344,3360,3376,3392,3408,3424,3440,3456,3472,3488,3504,3520,3536,3552,3568,3584,3600,3616,3632,3648,3664,3680,3696,3712,3728,3744,3760,3776,3792,3808,3824,3840,3856,3872,3888,3904,3920,3936,3952,3968,3984,4000,4016,4032,4048,4064,4095};


void setup_wifi() {
  WiFi.begin(SSID, PSK);
  // this stupid mac address trickery seems to be necessary
  // because for fuck's sake there seems to be no working conversion between unsigned char[] and char[]
  unsigned char mac[6];
  WiFi.macAddress(mac);
  int macNum = mac[0]*100000 + mac[1]*10000 + mac[2]*1000 + mac[3]*100 + mac[4]*10 + mac[5];
  itoa(macNum, macAddressUid, 10);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  
  Serial.println(WiFi.localIP());
}

void setup()
{
  // configure pins
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // start I2C bus
  Wire.begin();
  
  // init serial console
  Serial.begin(115200);
  delay(100);
  Serial.println("Setup");
  
  // setup PWM
  Serial.println("...PWM");
  pwm0.begin();

  
  // init wifi and MQTT
  Serial.println("...wifi");
  setup_wifi();
  Serial.println("...mqtt");
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(receiveMessage);


  // build topics - yes I know this can be made prettier, shut up!
  Serial.println("...topics");
  strcpy(clientName, namePrefix);
  strcat(clientName, macAddressUid);

  strcpy(configTopic, topicPrefix);
  strcat(configTopic, clientName);
  strcat(configTopic, topicPostfixConfig);

  strcpy(stateTopic, topicPrefix);
  strcat(stateTopic, clientName);
  strcat(stateTopic, topicPostfixState);
  
  strcpy(commandTopic, topicPrefix);
  strcat(commandTopic, clientName);
  strcat(commandTopic, topicPostfixCommand);

// 128 Bits message length:                                                                                                                        ---->|
  strcpy(configMessage0, "{\"name\":\"esp_dimmer\",\"schema\":\"json\",\"brightness\":true,\"clrm\":true,\"min_mireds\":142,\"max_mireds\":435");
  strcpy(configMessage1, ",\"sup_clrm\":[\"color_temp\",\"rgb\"]");
  strcpy(configMessage2, ",\"dev\":{\"identifiers\":[\"");
  strcat(configMessage2, clientName);
  strcpy(configMessage3, "\"],\"name\":\"esp_dimmer\"},\"stat_t\":\"");
  strcat(configMessage3, stateTopic);
  strcpy(configMessage4, "\",\"cmd_t\":\"");
  strcat(configMessage4, commandTopic);
  strcpy(configMessage5, "\",\"uniq_id\":\"");
  strcat(configMessage5, clientName);
  strcat(configMessage5, "\"}");

  Serial.print("clientName: ");
  Serial.println(clientName);
  Serial.print("configTopic: ");
  Serial.println(configTopic);
  Serial.print("stateTopic: ");
  Serial.println(stateTopic);
  Serial.print("commandTopic: ");
  Serial.println(commandTopic);

  // turn off internal LED
  Serial.println("...initial LED values");
  digitalWrite(LED_BUILTIN, HIGH);

  // Power on default: neutral white
  pwm0.setOutput(0, 0); // red
  pwm0.setOutput(1, 0); // green
  pwm0.setOutput(2, 0); // blue
  pwm0.setOutput(3, 2047); // warm white
  pwm0.setOutput(4, 2047); // cold white
  
  Serial.println("Initialization done.");
}

void loop()
{
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.println("(re)connecting to broker...");
      client.connect(clientName,MQTT_USER,MQTT_PASS);

      delay(100);
    }
    // discovery topic for autoconfiguration
    int configMessageLength = 0;
    configMessageLength += strlen(configMessage0);
    configMessageLength += strlen(configMessage1);
    configMessageLength += strlen(configMessage2);
    configMessageLength += strlen(configMessage3);
    configMessageLength += strlen(configMessage4);
    configMessageLength += strlen(configMessage5);
    client.beginPublish(configTopic, configMessageLength, true);
    client.write((const unsigned char*)configMessage0, strlen(configMessage0));
    client.write((const unsigned char*)configMessage1, strlen(configMessage1));
    client.write((const unsigned char*)configMessage2, strlen(configMessage2));
    client.write((const unsigned char*)configMessage3, strlen(configMessage3));
    client.write((const unsigned char*)configMessage4, strlen(configMessage4));
    client.write((const unsigned char*)configMessage5, strlen(configMessage5));
    client.endPublish();
    // state topic for defined initial state
    sendCurrentState();
    // command topic subscription
    client.subscribe(commandTopic);
    
    Serial.println("All MQTT topics published/subscribed, starting normal operation.");
  }
  client.loop();
  dimLoop();
  flashLoop();
}

void sendCurrentState(){
#if(NODE_DEBUG == true)
  Serial.println("Sending current state to MQTT");
#endif
  StaticJsonDocument<128> message;
  if(valueRin==0 && valueGin==0 && valueBin==0){
    message["state"] = "OFF";
  }else{
    message["state"] = "ON";
  }
  if(valueRin==valueGin && valueGin==valueBin){
    message["brightness"] = (valueRin+valueGin+valueBin)/3;
    message["color_temp"] = valueCTin;
  }else{
    message["color"]["r"] = valueRin;
    message["color"]["g"] = valueGin;
    message["color"]["b"] = valueBin;
  }
  char output[128];
  serializeJson(message, output);
  client.publish(stateTopic,output,true);
}

void receiveMessage(char* topic, byte* payload, unsigned int length) {
#if(NODE_DEBUG == true)
  Serial.print("Received topic: ");
  Serial.println(topic);
#endif
  
  if(strcmp(topic,commandTopic)==0){
#if(NODE_DEBUG == true)
    Serial.println("parse HA message...");
#endif
    parseHA(payload, length);
#if(NODE_DEBUG == true)
    Serial.println("update LEDs...");
#endif
    updateLEDs();
#if(NODE_DEBUG == true)
    Serial.println("send current state...");
#endif
    sendCurrentState();
  }

}

// parse message from home assistant "MQTT Light" command topic
void parseHA(byte* payload, unsigned int length){
  StaticJsonDocument<128> message;
  DeserializationError error = deserializeJson(message, payload);
  if(error){
#if(NODE_DEBUG == true)
    Serial.println(error.f_str());
#endif
  }else{
    
    // flash [seconds]: time the lamp is supposed to flash in the chosen color before returning to previous color
    JsonVariant flash = message["flash"];
    if(!flash.isNull()){
      flashProgress = 10 * flash.as<int>();
    }
    
    // transition [seconds]: time the lamp should take to transition to target color
    JsonVariant transition = message["transition"];
    if(!transition.isNull()){
      dimValue = 1000 * transition.as<int>();
    }
    
    // brightness [0..255]
    JsonVariant brightness = message["brightness"];
    if(!brightness.isNull()){
      valueRin = brightness.as<int>();
      valueGin = brightness.as<int>();
      valueBin = brightness.as<int>();
    }
    
    // colors [0..255]
    JsonVariant red = message["color"]["r"];
    if(!red.isNull()){valueRin = red;}
    JsonVariant green = message["color"]["g"];
    if(!green.isNull()){valueGin = green;}
    JsonVariant blue = message["color"]["b"];
    if(!blue.isNull()){valueBin = blue;}
    
    // color temperature [mireds]
    JsonVariant colorTemp = message["color_temp"];
    if(!colorTemp.isNull()){valueCTin = colorTemp;}

    // state [ON|OFF]
    JsonVariant state = message["state"];
    if(!state.isNull()){
      // if set "ON" with all colors remaining 0, set to 255,255,255
      if(strcmp(state,"ON")==0 && valueRin==0 && valueGin==0 && valueBin==0){
        valueRin=255;
        valueGin=255;
        valueBin=255;
      }else if(strcmp(state,"OFF")==0){
        valueRin=0;
        valueGin=0;
        valueBin=0;
      }
    }
#if(NODE_DEBUG == true)
    Serial.print("new colors: ");
    Serial.print(valueRin);
    Serial.print(",");
    Serial.print(valueGin);
    Serial.print(",");
    Serial.print(valueBin);
    Serial.print(",");
    Serial.println(valueCTin);
#endif
  }
}

// Calculate RGBWC values from input values and write to PWM outputs.
// Use conversion tables for uniform brightness dimming (avoids issues with dimming at low brightness).
// Use un-colored part (minimum of RGB) with white, add colors (difference of the values) on top.
// Mix warm white and cold white according to desired color temperature.
void updateLEDs(){
#if(NODE_DEBUG == true)
  Serial.println("Updating LEDs");
#endif

  // mixing color temperature
  valueCTK = 1000000/valueCTin; // convert mireds to K
  // limit value to [WW_TEMP..CW_TEMP]
  if(valueCTK < WW_TEMP){valueCTK = WW_TEMP;}
  if(valueCTK > CW_TEMP){valueCTK = CW_TEMP;}
  // calculate values for WW and CW if they were at full brightness (still INPUT values, so 8 bit range)
  int valueCWfull = ((255 * (valueCTK - WW_TEMP)) / (CW_TEMP - WW_TEMP));
  int valueWWfull = (255 - valueCWfull);

  // mixing with white, looking up 16bit values, storing in pwmValues[]
  int whiteContent = min(valueRin, min(valueGin, valueBin));
  pwmEndValues[0] = pwmTable[(valueRin - whiteContent)];
  pwmEndValues[1] = pwmTable[(valueGin - whiteContent)];
  pwmEndValues[2] = pwmTable[(valueBin - whiteContent)];
  pwmEndValues[3] = pwmTable[valueWWfull * whiteContent / 255];
  pwmEndValues[4] = pwmTable[valueCWfull * whiteContent / 255];
  // start default transition to new color
  dimProgress = dimValue;

#if(NODE_DEBUG == true)
  Serial.print("White values: ");
  Serial.print(valueCTin);
  Serial.print(",");
  Serial.print(valueCWfull);
  Serial.print(",");
  Serial.print(valueWWfull);
  Serial.print(",");
  Serial.println(whiteContent);

  Serial.print("PWM values: ");
  Serial.print(pwmEndValues[0]);
  Serial.print(",");
  Serial.print(pwmEndValues[1]);
  Serial.print(",");
  Serial.print(pwmEndValues[2]);
  Serial.print(" / ");
  Serial.print(pwmEndValues[3]);
  Serial.print(",");
  Serial.println(pwmEndValues[4]);
#endif
}

void dimLoop(){
  if(flashProgress==0){ // no dimming while flashing
    int now = millis();
    // overflow handling, as millis() will eventually overflow
    if(now-dimTimer<0){
      dimTimer = now;
    }
    // dimming handling
    if(now-dimTimer>10){
      dimTimer = now;
      if(dimProgress > 0){
        // dimProgress works in ms, but steps are 10ms in time, so decrement by 10
        dimProgress -= 10;
        if(dimProgress < 0){dimProgress = 0;}
    
        if(dimProgress > 0){
          // dimming still in progress
#if(NODE_DEBUG == true)
          Serial.println("Dimming in progress...");
#endif

          // formula for dimming progress:
          // diff = end-start                10-5      = 5     5-10          = -5
          // value = (diff*progress)+start   (5*0.5)+5 = 7,5   (-5*0,5)+(-5) = -7,5
          int rVal = ((pwmEndValues[0]-pwmStartValues[0])*(dimValue-dimProgress)/dimValue)+pwmStartValues[0];
          int gVal = ((pwmEndValues[1]-pwmStartValues[1])*(dimValue-dimProgress)/dimValue)+pwmStartValues[1];
          int bVal = ((pwmEndValues[2]-pwmStartValues[2])*(dimValue-dimProgress)/dimValue)+pwmStartValues[2];
          int wwVal = ((pwmEndValues[3]-pwmStartValues[3])*(dimValue-dimProgress)/dimValue)+pwmStartValues[3];
          int cwVal = ((pwmEndValues[4]-pwmStartValues[4])*(dimValue-dimProgress)/dimValue)+pwmStartValues[4];
          pwm0.setOutput(0, rVal);
          pwm0.setOutput(1, gVal);
          pwm0.setOutput(2, bVal);
          pwm0.setOutput(3, wwVal);
          pwm0.setOutput(4, cwVal);
        }else{
          // dimming done, set final values and update dimming values for next tim
          pwmStartValues[0] = pwmEndValues[0];
          pwmStartValues[1] = pwmEndValues[1];
          pwmStartValues[2] = pwmEndValues[2];
          pwmStartValues[3] = pwmEndValues[3];
          pwmStartValues[4] = pwmEndValues[4];
          pwm0.setOutput(0, pwmEndValues[0]);
          pwm0.setOutput(1, pwmEndValues[1]);
          pwm0.setOutput(2, pwmEndValues[2]);
          pwm0.setOutput(3, pwmEndValues[3]);
          pwm0.setOutput(4, pwmEndValues[4]);
        }
      }else{
        // at end of dim, reset to default dim time
        dimValue = 100;
      }
    }
  }
}

void flashLoop(){
  int now = millis();
  // overflow handling, as millis() will eventually overflow
  if(now-flashTimer<0){
    flashTimer = now;
  }
  // flashing handling
  if(flashProgress>0){
    // countdown to get back to normal
    if(now-flashTimer > 100){
#if(NODE_DEBUG == true)
    Serial.println("Flashing in progress...");
#endif
      flashTimer = now;
      
      // flashing lights
      if(flashProgress%3==0){
#if(NODE_DEBUG == true)
        Serial.println("Flashing...");
#endif
        flashBool = !flashBool;
        if(flashBool){
          pwm0.setOutput(0, pwmEndValues[0]);
          pwm0.setOutput(1, pwmEndValues[1]);
          pwm0.setOutput(2, pwmEndValues[2]);
          pwm0.setOutput(3, pwmEndValues[3]);
          pwm0.setOutput(4, pwmEndValues[4]);
        }else{
          pwm0.setOutput(0, 0);
          pwm0.setOutput(1, 0);
          pwm0.setOutput(2, 0);
          pwm0.setOutput(3, 0);
          pwm0.setOutput(4, 0);
        }
      }
      
      // decrement and reset
      flashProgress--;
      if(flashProgress == 0){
        // last flash is done, reset original lamp mode/color
        pwmEndValues[0] = pwmStartValues[0];
        pwmEndValues[1] = pwmStartValues[1];
        pwmEndValues[2] = pwmStartValues[2];
        pwmEndValues[3] = pwmStartValues[3];
        pwmEndValues[4] = pwmStartValues[4];
      }
    }
  }
}
