#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ArduinoJson.h>

//SW name & version
#define     VERSION                      "0.84"
#define     SW_NAME                      "Anemometer"

#define timers
#define ota
#define verbose

/*
--------------------------------------------------------------------------------------------------------------------------

Version history:

--------------------------------------------------------------------------------------------------------------------------
HW
ESP8266 Wemos D1
*/

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
//#define BUILTIN_LED                         1

const byte interruptPin = D2;
const byte analogPin    = A0;
                                     
#define SEND_DELAY                          5000  //prodleva mezi poslanim dat v ms
#define CONNECT_DELAY                       5000 //ms

#include <fce.h>

#endif
