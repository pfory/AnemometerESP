/*
Rychlost vetru - pocitat pulzy a jednou za minutu spocitat prumer
Naraz vetru - merit pocet pulzu kazdou sekundu a odeslat ten nejvyssi
8 pulzu na otacku
*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//#include <FS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h> 

//for LED status
#include <Ticker.h>
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}
  
  
//#define verbose
#ifdef verbose
 #define DEBUG_PRINT(x)         Serial.print (x)
 #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)       Serial.println (x)
 #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
 #define PORTSPEED 115200
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTF(x, y)
#endif 
  

#define AIO_SERVER      "192.168.1.56"
//#define AIO_SERVER      "178.77.238.20"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "datel"
#define AIO_KEY         "hanka12"

unsigned int volatile pulseCount     = 0;
unsigned long volatile pulseLength   = 0;

WiFiClient client;

const byte interruptPin = D2;
const byte analogPin    = A0;
//const byte ledPin       = D4;

unsigned long sendDelay = 5000;
unsigned long lastSend  = sendDelay * -1;

unsigned long milisLastRunMinOld          = 0;


Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
Adafruit_MQTT_Publish verSW               = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/VersionSW");
Adafruit_MQTT_Publish hb                  = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/HeartBeat");
Adafruit_MQTT_Publish speed               = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/Rychlost");
Adafruit_MQTT_Publish vector              = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/Smer");

Adafruit_MQTT_Subscribe restart       = Adafruit_MQTT_Subscribe(&mqtt, "/home/Anemometer/esp11/restart");

IPAddress _ip           = IPAddress(192, 168, 1, 106);
IPAddress _gw           = IPAddress(192, 168, 1, 1);
IPAddress _sn           = IPAddress(255, 255, 255, 0);


void MQTT_connect(void);

extern "C" {
  #include "user_interface.h"
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

float versionSW                   = 0.18;
String versionSWString            = "Anemometer v";
uint32_t heartBeat                = 0;

void setup() {
#ifdef verbose
  Serial.begin(PORTSPEED);
#endif
  DEBUG_PRINTLN();
  DEBUG_PRINT(versionSWString);
  DEBUG_PRINTLN(versionSW);
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);
  
  DEBUG_PRINTLN(ESP.getResetReason());
  if (ESP.getResetReason()=="Software/System restart") {
    heartBeat=1;
  } else if (ESP.getResetReason()=="Power on") {
    heartBeat=2;
  } else if (ESP.getResetReason()=="External System") {
    heartBeat=3;
  } else if (ESP.getResetReason()=="Hardware Watchdog") {
    heartBeat=4;
  } else if (ESP.getResetReason()=="Exception") {
    heartBeat=5;
  } else if (ESP.getResetReason()=="Software Watchdog") {
    heartBeat=6;
  } else if (ESP.getResetReason()=="Deep-Sleep Wake") {
    heartBeat=7;
  }

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  wifiManager.setConnectTimeout(600); //5min

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  
  //Serial.println(ESP.getFlashChipRealSize);
  //Serial.println(ESP.getCpuFreqMHz);
  //WiFi.begin(ssid, password);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  
  if (!wifiManager.autoConnect("Anemometer", "password")) {
    DEBUG_PRINTLN("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

	DEBUG_PRINTLN("");
	DEBUG_PRINT("Connected to ");
	DEBUG_PRINT("IP address: ");
	DEBUG_PRINTLN(WiFi.localIP());
  
  //v klidu +3V, pulz vstup stahuje k zemi pres pulldown
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), pulseCountEvent, RISING);
  
  //OTA
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("anemometer");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    // String type;
    // if (ArduinoOTA.getCommand() == U_FLASH)
      // type = "sketch";
    // else // U_SPIFFS
      // type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //DEBUG_PRINTLN("Start updating " + type);
    DEBUG_PRINTLN("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
   DEBUG_PRINTLN("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();

  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);
}

void loop() {
  if (millis() - lastSend >= sendDelay) {
    //Serial.println(analogRead(analogPin));
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    noInterrupts();
    MQTT_connect();

    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
      if (subscription == &restart) {
        char *pNew = (char *)restart.lastread;
        uint32_t pPassw=atol(pNew); 
        if (pPassw==650419) {
          DEBUG_PRINT(F("Restart ESP now!"));
          ESP.restart();
        } else {
          DEBUG_PRINT(F("Wrong password."));
        }
      }
    }

  //if (false) {
  
    //pocet pulsu
    DEBUG_PRINTLN(pulseCount);
    if (! speed.publish((float)pulseCount/((millis() - lastSend) / 1000))) {
      DEBUG_PRINT(F("Send pulseCount failed!"));
    } else {
      DEBUG_PRINT(F("Send pulseCount OK!"));
    }
    
    //smer vetru
    if (! vector.publish(analogRead(analogPin))) {
      DEBUG_PRINT(F("Send vector failed!"));
    } else {
      DEBUG_PRINT(F("Send vector OK!"));
    }
    lastSend = millis();
    pulseCount = 0;
    
    interrupts();
  }
  
  if (millis() - milisLastRunMinOld > 60000) {
    milisLastRunMinOld = millis();
    if (! hb.publish(heartBeat)) {
      DEBUG_PRINTLN("Send HB failed");
    } else {
      DEBUG_PRINTLN("Send HB OK!");
    }
    heartBeat++;
    if (! verSW.publish(versionSW)) {
      DEBUG_PRINT(F("Send verSW failed!"));
    } else {
      DEBUG_PRINT(F("Send verSW OK!"));
    }
  }
  
  ArduinoOTA.handle();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  DEBUG_PRINT("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    DEBUG_PRINTLN(mqtt.connectErrorString(ret));
    DEBUG_PRINTLN("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  DEBUG_PRINTLN("MQTT Connected!");
}

void pulseCountEvent() {
  digitalWrite(BUILTIN_LED, LOW);
  pulseCount++;
  digitalWrite(BUILTIN_LED, HIGH);
}
