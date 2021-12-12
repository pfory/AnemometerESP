//ANEMOMETER
/*
Rychlost vetru - pocitat pulzy a jednou za minutu spocitat prumer
Naraz vetru - merit pocet pulzu kazdou sekundu a odeslat ten nejvyssi
8 pulzu na otacku
*/
//Wemos D1 R2 & mini !!!!!!!SPIFSS 1M !!!!!!!!!!!!!!!!!!

#include "Configuration.h"


//#define serverHTTP
#ifdef serverHTTP
ESP8266WebServer server(80);
#endif

unsigned int volatile pulseCount            = 0;
unsigned int pulseCountLast                 = 0;
unsigned long lastSend                      = 0;


#ifdef serverHTTP
void handleRoot() {
	char temp[600];
  // DEBUG_PRINT(year());
  // DEBUG_PRINT(month());
  // DEBUG_PRINT(day());
  // DEBUG_PRINT(hour());
  // DEBUG_PRINT(minute());
  // DEBUG_PRINT(second());
  printSystemTime();
  DEBUG_PRINTLN(" Client request");
  digitalWrite(BUILTIN_LED, LOW);
  
	// snprintf ( temp, 400,
      // "<html>\
        // <head>\
          // <meta charset='UTF-8'>\
        // </head>\
        // <body>\
          // T2899BDCF02000076,%4d-%02d-%02dT%02d:%02d:%02d.000000Z,%s%d.%02d<br />\
          // Humidity,%4d-%02d-%02dT%02d:%02d:%02d.000000Z,%d.00<br />\
          // Press,%4d-%02d-%02dT%02d:%02d:%02d.000000Z,%d.00<br />\
          // DewPoint,%4d-%02d-%02dT%02d:%02d:%02d.000000Z,%s%d.%02d<br />\
        // </body>\
      // </html>",
      // year(), month(), day(), hour(), minute(), second(),
      // temperature<0 && temperature>-1 ? "-":"",
      // (int)temperature, 
      // abs((temperature - (int)temperature) * 100),
      // year(), month(), day(), hour(), minute(), second(),
      // (int)humidity,
      // year(), month(), day(), hour(), minute(), second(),
      // (int)pressure,
      // year(), month(), day(), hour(), minute(), second(),
      // dewPoint<0 && dewPoint>-1 ? "-":"",
      // (int)dewPoint, 
      // abs((dewPoint - (int)dewPoint) * 100)
	// );
	server.send ( 200, "text/html", temp );
  digitalWrite(BUILTIN_LED, HIGH);
}
#endif

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  char * pEnd;
  long int valL;
  String val =  String();
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (int i=0;i<length;i++) {
    DEBUG_PRINT((char)payload[i]);
    val += (char)payload[i];
  }
  DEBUG_PRINTLN();

  if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_restart)).c_str())==0) {
    DEBUG_PRINT("RESTART");
    ESP.restart();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_netinfo)).c_str())==0) {
    sendNetInfoMQTT();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_config_portal)).c_str())==0) {
    startConfigPortal();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_config_portal_stop)).c_str())==0) {
    stopConfigPortal();
  }
}

void ICACHE_RAM_ATTR pulseCountEvent() {
  digitalWrite(BUILTIN_LED, LOW);
  pulseCount++;
  digitalWrite(BUILTIN_LED, HIGH);
}

void setup() {
  preSetup();

  //v klidu +3V, pulz vstup stahuje k zemi pres pulldown
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), pulseCountEvent, RISING);

#ifdef timers
  //setup timers
  timer.every(SEND_DELAY, sendDataMQTT);
  timer.every(SENDSTAT_DELAY, sendStatisticMQTT);
  timer.every(CONNECT_DELAY, reconnect);
#endif

  void * a;
  reconnect(a);
  sendNetInfoMQTT();
  sendStatisticMQTT(a);
  
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, HIGH);
  
  drd.stop();

  DEBUG_PRINTLN(F("SETUP END......................."));
}

void loop() {
  timer.tick(); // tick the timer
#ifdef serverHTTP
  server.handleClient();
#endif

#ifdef ota
  ArduinoOTA.handle();
#endif

  client.loop();
  wifiManager.process();
}

bool sendDataMQTT(void *) {
  digitalWrite(BUILTIN_LED, LOW);
  DEBUG_PRINTLN(F("Data"));
  
  float pc = (float)pulseCount/((millis() - lastSend) / 1000);
  if (abs(pc - pulseCountLast) < PULSECOUNTDIF) {
    client.publish((String(mqtt_base) + "/Rychlost").c_str(), String(pc).c_str());
  }
  client.publish((String(mqtt_base) + "/Smer").c_str(), String(analogRead(analogPin)).c_str());

  pulseCountLast = pc;
  pulseCount = 0;
  lastSend = millis();
  digitalWrite(BUILTIN_LED, HIGH);
  return true;
}

bool reconnect(void *) {
  if (!client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_base, mqtt_username, mqtt_key, (String(mqtt_base) + "/LWT").c_str(), 2, true, "offline", true)) {
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_topic_restart)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_topic_netinfo)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_config_portal)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_config_portal_stop)).c_str());
      client.publish((String(mqtt_base) + "/LWT").c_str(), "online", true);
      DEBUG_PRINTLN("connected");
    } else {
      DEBUG_PRINT("disconected.");
      DEBUG_PRINT(" Wifi status:");
      DEBUG_PRINT(WiFi.status());
      DEBUG_PRINT(" Client status:");
      DEBUG_PRINTLN(client.state());
    }
  }
  return true;
}