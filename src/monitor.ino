// HC-SR04: VCC - 5V, trigger - 3V
// WeMos D1 (ESP8266): D1 - trigger, D2 - echo, D0 and RST connected for deep sleep mode
// MQTT using https://github.com/Imroy/pubsubclient
// JSON using: https://github.com/bblanchon/ArduinoJson

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>
#include <ArduinoJson.h>

/////////////////////
//Occupation status//
////////////////////
#define STATE_UNKNOWN 0
#define STATE_UNOCCUPIED 10
#define STATE_OCCUPIED 20

////////////////////////
//Location definitions//
////////////////////////
#define OFFICE "MeaningfulOfficeID"
#define VERSION 1

/////////////////////
//WiFi and MQTT def//
/////////////////////
const char* SSID = "SSID";
const char* wiFiPassword = "WiFi password";
const char* mqttServer = "mqtt FQDN";
const char* mqttUser = "mqtt login";
const char* mqttPassword = "mqtt password";

//////////////////
//LED definition//
/////////////////

//////////////
//Connection//
//////////////
WiFiClient wclient;
PubSubClient client(wclient, mqttServer);

void setup_wifi() {
  flashLed(500, 3);

  Serial.print("Connecting to: [");
  Serial.print(SSID);
  Serial.println("]");

  WiFi.begin(SSID, wiFiPassword);

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("[?] Waiting for WiFi connection");
  }
  Serial.println("");
  Serial.print("[*] WiFi connected [");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  digitalWrite(BUILTIN_LED, HIGH);
}

void setup() {
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT); 
  
  setup_wifi();
}

void loop() {
  Serial.println("In loop");
  delay(1000);
}

void flashLed(int delayTime, int numberOfFlashes) {
  for (int i=0; i<numberOfFlashes; i++) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(delayTime);
    digitalWrite(BUILTIN_LED, LOW);
  }
}

