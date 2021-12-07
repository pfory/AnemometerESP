#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <DoubleResetDetector.h>  //https://github.com/khoih-prog/ESP_DoubleResetDetector
#include <PubSubClient.h>
#include <Wire.h>
#include <timer.h>

//SW name & version
#define     VERSION                      "0.81"
#define     SW_NAME                      "Anemometer"

//#define timers
#define ota
#define verbose


#define CFGFILE "/config.json"


#ifdef serverHTTP
#include <ESP8266WebServer.h>
#endif

#ifdef time
#include <TimeLib.h>
#include <Timezone.h>
#endif
/*
--------------------------------------------------------------------------------------------------------------------------

Version history:

--------------------------------------------------------------------------------------------------------------------------
HW
ESP8266 Wemos D1
*/

#define CONFIG_PORTAL_TIMEOUT 60 //jak dlouho zustane v rezimu AP nez se cip resetuje
#define CONNECT_TIMEOUT 5 //jak dlouho se ceka na spojeni nez se aktivuje config portal

static const char* const      mqtt_server                    = "192.168.1.56";
static const uint16_t         mqtt_port                      = 1883;
static const char* const      mqtt_username                  = "datel";
static const char* const      mqtt_key                       = "hanka12";
static const char* const      mqtt_base                      = "/home/Anemometer/esp11";
static const char* const      mqtt_topic_restart             = "restart";
static const char* const      mqtt_topic_netinfo             = "netinfo";
static const char* const      mqtt_config_portal             = "config";
static const char* const      mqtt_config_portal_stop        = "disconfig";


#define PULSECOUNTDIF                       40 //10m/s

const byte interruptPin = D2;
const byte analogPin    = A0;
                                     
#define SEND_DELAY                          5000  //prodleva mezi poslanim dat v ms
#define SENDSTAT_DELAY                      60000 //poslani statistiky kazdou minutu
#define CONNECT_DELAY                       5000 //ms

#include <fce.h>

#endif
