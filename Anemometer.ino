#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <FS.h>

const char *ssid = "Datlovo";
const char *password = "Nu6kMABmseYwbCoJ7LyG";

#define AIO_SERVER      "192.168.1.56"
//#define AIO_SERVER      "178.77.238.20"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "datel"
#define AIO_KEY         "hanka12"

uint32_t pulseCount     = 0;

WiFiClient client;

//const byte ledPin       = 0;
const byte interruptPin = 2;
const byte analogPin    = A0;

unsigned long sendDelay = 60000;
unsigned long lastSend  = sendDelay * -1;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
Adafruit_MQTT_Publish verSW               = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/VersionSW");
Adafruit_MQTT_Publish hb                  = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/HeartBeat");
Adafruit_MQTT_Publish speed               = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/Rychlost");
Adafruit_MQTT_Publish vector              = Adafruit_MQTT_Publish(&mqtt, "/home/Anemometer/esp11/Smer");

//Adafruit_MQTT_Subscribe setupPulse    = Adafruit_MQTT_Subscribe(&mqtt, "/home/Anemometer/esp11/setupPulse");
Adafruit_MQTT_Subscribe restart       = Adafruit_MQTT_Subscribe(&mqtt, "/home/Anemometer/esp11/restart");

#define SERIALSPEED 115200

void MQTT_connect(void);

extern "C" {
  #include "user_interface.h"
}

float versionSW                   = 0.1;
String versionSWString            = "Anemometer v";
byte heartBeat                    = 10;

void setup() {
  Serial.begin(SERIALSPEED);
  Serial.print(versionSWString);
  Serial.println(versionSW);
  // pinMode(ledPin, OUTPUT);
  // digitalWrite(ledPin, HIGH);
  
  Serial.println(ESP.getResetReason());
  if (ESP.getResetReason()=="Software/System restart") {
    heartBeat=11;
  } else if (ESP.getResetReason()=="Power on") {
    heartBeat=12;
  } else if (ESP.getResetReason()=="External System") {
    heartBeat=13;
  } else if (ESP.getResetReason()=="Hardware Watchdog") {
    heartBeat=14;
  } else if (ESP.getResetReason()=="Exception") {
    heartBeat=15;
  } else if (ESP.getResetReason()=="Software Watchdog") {
    heartBeat=16;
  } else if (ESP.getResetReason()=="Deep-Sleep Wake") {
    heartBeat=17;
  }
  
  //Serial.println(ESP.getFlashChipRealSize);
  //Serial.println(ESP.getCpuFreqMHz);
  WiFi.begin(ssid, password);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
  
  //v klidu +3V, pulz vstup stahuje k zemi pres pulldown
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), pulseCountEvent, FALLING);
  // digitalWrite(ledPin, LOW);
  
  //mqtt.subscribe(&setupPulse);
  //mqtt.subscribe(&restart);
}

void loop() {
  Serial.println(analogRead(analogPin));
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &restart) {
      char *pNew = (char *)restart.lastread;
      uint32_t pPassw=atol(pNew); 
      if (pPassw==650419) {
        Serial.print(F("Restart ESP now!"));
        ESP.restart();
      } else {
        Serial.print(F("Wrong password."));
      }
    }
  }

  if (millis() - lastSend>=sendDelay) {
    lastSend = millis();
    if (! verSW.publish(versionSW)) {
      Serial.println("failed");
    } else {
      Serial.println("OK!");
    }
    if (! hb.publish(heartBeat++)) {
      Serial.println("failed");
    } else {
      Serial.println("OK!");
    }
    if (heartBeat>1) {
      heartBeat = 0;
    }

    //rychlost vetru
    if (! speed.publish(10)) {
      Serial.println("failed");
    } else {
      Serial.println("OK!");
    }
    
    //smer vetru
    if (! vector.publish(125)) {
      Serial.println("failed");
    } else {
      Serial.println("OK!");
    }
  }
    // if (! pulseLength.publish(pulseWidth)) {
      // Serial.println("failed");
    // } else {
      // Serial.println("OK!");
    // }
  
    // digitalWrite(ledPin, LOW);
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void pulseCountEvent() {
  if (digitalRead(interruptPin)==HIGH) { //dobezna
    attachInterrupt(digitalPinToInterrupt(interruptPin), pulseCountEvent, FALLING);
    pulseCount++;
    Serial.println(pulseCount);
  }
  if (digitalRead(interruptPin)==LOW) { //nabezna
    attachInterrupt(digitalPinToInterrupt(interruptPin), pulseCountEvent, RISING);
  } 
}