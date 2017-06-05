// HC-SR04: VCC - 5V, trigger - 3V
// WeMos D1 (ESP8266): D1 - trigger, D2 - echo, D0 and RST connected for deep sleep mode
// MQTT using https://github.com/Imroy/pubsubclient
// JSON using: https://github.com/bblanchon/ArduinoJson
// config upload: https://github.com/esp8266/arduino-esp8266fs-plugin

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Configuration.h>

//PIN and constants definition
#include "Util.h"
//ENUMS for json configuration
#include "Tags.h"

//////////////
//Connection//
//////////////
WiFiClient wclient;
Configuration conf("/config.json");
PubSubClient* client;

void setup() {
  Serial.begin(9600);
  
  init_gpio();

  init_globals();
 
  connect_to_wifi();
}

void loop() {
  
  int intDeviceId = ESP.getChipId();
  String deviceId = String(intDeviceId);
  
  if (!client -> connected()) {
    reconnect_mqtt(deviceId);
  }
  if (WiFi.status() != WL_CONNECTED) {
    connect_to_wifi();
  }

  //loops through mqtt client
  client -> loop();
}

void init_gpio() {
  pinMode(BUILTIN_LED, OUTPUT); 
  pinMode(REDPIN, OUTPUT);
  digitalWrite(REDPIN, LOW);
  pinMode(BLUEPIN, OUTPUT);
  digitalWrite(BLUEPIN, LOW);
  pinMode(GREENPIN, OUTPUT);
  digitalWrite(GREENPIN, LOW);
}

void init_globals() {
  Serial.println(conf.getValue(tag[MQTT_SERVER]));
  client = new PubSubClient(wclient, conf.getValue(tag[MQTT_SERVER]).c_str());
}

void connect_to_wifi() {
  Serial.print("Connecting to: [");
  Serial.print(conf.getValue(tag[SSID]));
  Serial.println("]");
  WiFi.begin(conf.getValue(tag[SSID]).c_str(), conf.getValue(tag[WIFI_PASSWORD]).c_str());

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("[?] Waiting for WiFi connection");
  }
  
  Serial.println("");
  Serial.print("[*] WiFi connected [");
  Serial.print(WiFi.localIP());
  Serial.println("]");
}

void reconnect_mqtt(String deviceId) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connectiong to MQTT server");
    if (client -> connect(MQTT::Connect(deviceId)
                            .set_auth(conf.getValue(tag[MQTT_USER]), conf.getValue(tag[MQTT_PASSWORD]))
                            .set_keepalive(60)))
    {
      Serial.println("Connected to MQTT server");
      client -> set_callback(mqttCallback);
      String topic = "toilet/" + String(conf.getValue(tag[OFFICE_ID])) + "/" + conf.getValue(tag[BEACON_DEVICE_ID]);
      Serial.print("Subscribing to: ");
      Serial.println(topic);
      client -> subscribe(MQTT::Subscribe()
                              .add_topic(topic,1));
    } else {
      Serial.println("failed, try again in 5 secods");
      delay(5000);
    }
  }
}

void mqttCallback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload;
  if (pub.has_stream()) {
    uint8_t buf[100];
    int read;
    while (read = pub.payload_stream()->read(buf, 100)) {
      Serial.write(buf, read);
      //TODO :)
      payload = "";
    }
    pub.payload_stream()->stop();
    Serial.println("");
  } else {
    payload = pub.payload_string();
    Serial.println(pub.payload_string());
  }
  processPayload(payload);
}

void mqttPublish(String message, int intDeviceId) {
  client -> set_callback(mqttCallback);
  client -> publish(MQTT::Publish("/test/ping/", message)
                        .set_qos(1)
                        .set_retain()
                 );
}

void processPayload(String payload) {
  Serial.println("Decoding message... ");
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  int currentStatus = root["status"];
      
  Serial.print("Got status: ");
  Serial.print(currentStatus);
  if (!lastStatus) {
    lastStatus = currentStatus;
  }
  if (lastStatus != currentStatus) {
    lastStatus = currentStatus;
  }
  toggleLed();
}

void toggleLed() {
  if (lastStatus) {
    if (lastStatus == STATE_OCCUPIED) {
      setColor(COLOR_RED);
      return;
    }
    if (lastStatus == STATE_UNOCCUPIED) {
      setColor(COLOR_GREEN);
      return;
    }
  }
  setColor(COLOR_YELLOW);
}

void setColor(const int rgb_color[]) {
  Serial.println();
  Serial.print("Setting color: ");
  Serial.print(rgb_color[0]);
  Serial.print("R, ");
  Serial.print(rgb_color[1]);
  Serial.print("G, ");
  Serial.print(rgb_color[2]);
  Serial.println("B");
  //analogWrite(REDPIN, rgb_color[0]);
  //analogWrite(GREENPIN, rgb_color[1]);
  //analogWrite(BLUEPIN, rgb_color[2]);
  if (rgb_color[0] == 255) {
    digitalWrite(REDPIN, LOW);
  } else {
    digitalWrite(REDPIN, HIGH);
  }
  if (rgb_color[1] == 255) {
    digitalWrite(GREENPIN, LOW);
  } else {
    digitalWrite(GREENPIN, HIGH);
  }
  if (rgb_color[2] == 255) {
    digitalWrite(BLUEPIN, LOW);
  } else {
    digitalWrite(BLUEPIN, HIGH);
  }
}

