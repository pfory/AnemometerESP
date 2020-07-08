#ifndef CONFIGURATION_H
#define CONFIGURATION_H

//SW name & version
#define     VERSION                      "0.43"
#define     SW_NAME                      "Anemometer"

//#define timers
#define ota
#define verbose

#define AUTOCONNECTNAME   HOSTNAMEOTA
#define AUTOCONNECTPWD    "password"

#define ota
#ifdef ota
#include <ArduinoOTA.h>
#define HOSTNAMEOTA   "anemometer"
#endif


/*
--------------------------------------------------------------------------------------------------------------------------

Version history:

--------------------------------------------------------------------------------------------------------------------------
HW
ESP8266 Wemos D1
*/

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "DoubleResetDetector.h" // https://github.com/datacute/DoubleResetDetector
#include "Sender.h"
#include <Wire.h>

#define verbose
#ifdef verbose
  #define DEBUG_PRINT(x)         Serial.print (x)
  #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)       Serial.println (x)
  #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
  #define DEBUG_PRINTHEX(x)      Serial.print (x, HEX)
  #define PORTSPEED 115200
  #define SERIAL_BEGIN           Serial.begin(PORTSPEED);
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, y)
#endif 

static const char* const      mqtt_server                    = "192.168.1.56";
static const uint16_t         mqtt_port                      = 1883;
static const char* const      mqtt_username                  = "datel";
static const char* const      mqtt_key                       = "hanka12";
static const char* const      mqtt_base                      = "/home/Anemometer/esp11";
static const char* const      static_ip                      = "192.168.1.106";
static const char* const      static_gw                      = "192.168.1.1";
static const char* const      static_sn                      = "255.255.255.0";
static const char* const      mqtt_topic_restart             = "restart";

#define PULSECOUNTDIF                       40 //10m/s


//All of the IO pins have interrupt/pwm/I2C/one-wire support except D0.
#define STATUS_LED                          BUILTIN_LED //status LED
//SDA                                       D2 //                           GPIO4
//SCL                                       D1 //                           GPIO5
//BUILTIN_LED                               D4 //10k Pull-up, BUILTIN_LED   GPIO2

                                     
#define SEND_DELAY                          5000  //prodleva mezi poslanim dat v ms
#define SENDSTAT_DELAY                      60000 //poslani statistiky kazdou minutu

#define CFGFILE "/config.json"

                                     
#endif
