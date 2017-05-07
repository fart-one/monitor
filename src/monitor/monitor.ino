// HC-SR04: VCC - 5V, trigger - 3V
// WeMos D1 (ESP8266): D1 - trigger, D2 - echo, D0 and RST connected for deep sleep mode
// MQTT using https://github.com/Imroy/pubsubclient
// JSON using: https://github.com/bblanchon/ArduinoJson

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "Configuration.h"

/////////////////////////////////
//Global variable to toggle LED//
/////////////////////////////////
int lastStatus;

//////////////
//RGB PINOUT//
//////////////
const int REDPIN = D5;
const int GREENPIN = D3;
const int BLUEPIN = D7;

/////////////////////
//Colors definition//
/////////////////////
const int COLOR_YELLOW[3] = {255, 255, 0};
const int COLOR_RED[3] = {255, 0, 0};
const int COLOR_GREEN[3] = {0, 255, 0};

//////////////
//Connection//
//////////////
WiFiClient wclient;
PubSubClient client(wclient, mqttServer);

void setup_wifi() {

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
}

void reconnect_mqtt(String deviceId) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connectiong to MQTT server");
    if (client.connect(MQTT::Connect(deviceId)
                            .set_auth(mqttUser, mqttPassword)
                            .set_keepalive(60)))
    {
      Serial.println("Connected to MQTT server");
      client.set_callback(mqttCallback);
      String topic = "toilet/" + String(OFFICE) + "/" + deviceId;
      Serial.print("Subscribing to: ");
      Serial.println(topic);
      client.subscribe(MQTT::Subscribe()
                              .add_topic(topic,1));
    } else {
      Serial.print("failed, rc=");
      //Serial.print(client.state());
      Serial.println(" try again in 5 secods");
      delay(5000);
    }
  }
}

void mqttCallback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload;
  if (pub.has_stream()) {
    uint8_t buf[BUFFER_SIZE];
    int read;
    while (read = pub.payload_stream()->read(buf, BUFFER_SIZE)) {
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
  client.set_callback(mqttCallback);
  client.publish(MQTT::Publish("/test/ping/", message)
                        .set_qos(1)
                        .set_retain()
                 );
}

void processPayload(String payload) {
  Serial.println("Decoding message... ");
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  //root.printTo(Serial);
  int device = root["deviceId"];
  int currentStatus = root["status"];
  Serial.println(device);
  if (device) {
    int deviceId = ESP.getChipId();

    Serial.print("Comparing received ID: [");
    Serial.print(device);
    Serial.print("] to: [");
    Serial.print(deviceId);
    Serial.println("]");
    if (device == deviceId) {
      Serial.println("Payload should be processed by this device");
      //root.printTo(Serial);
      Serial.print("Got status: ");
      Serial.print(currentStatus);
      if (!lastStatus) {
        lastStatus = currentStatus;
        toggleLed();
      }
      if (lastStatus != currentStatus) {
        lastStatus = currentStatus;
        toggleLed();
      }
    } else {
      Serial.println("Payload missmatch");
    }
  }
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

void setup() {
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT); 
  pinMode(REDPIN, OUTPUT);
  digitalWrite(REDPIN, LOW);
  pinMode(BLUEPIN, OUTPUT);
  digitalWrite(BLUEPIN, LOW);
  pinMode(GREENPIN, OUTPUT);
  digitalWrite(GREENPIN, LOW);
  setup_wifi();
}

void loop() {
  
  int intDeviceId = ESP.getChipId();
  String deviceId = String(intDeviceId);

  StaticJsonBuffer<256> jsonBuffer;
  
  if (!client.connected()) {
    reconnect_mqtt(deviceId);
  } else {
    JsonObject& jsonMessage = buildJson(jsonBuffer, 100, intDeviceId, VERSION, 2);
    String message;
    jsonMessage.printTo(message);

    //mqttPublish(message, intDeviceId);
  }

  client.loop();

  delay(sleepTime * 1000);
}

/**
 * Creates comunication frame
 */
JsonObject& buildJson(JsonBuffer& jsonBuffer, int distance, int deviceId, int ver, int state)  {
  JsonObject& root = jsonBuffer.createObject();

  root["deviceId"] = deviceId;
  root["status"] = state;
  root["version"] = ver;
  
  JsonArray& measurements = root.createNestedArray("measurements");
  JsonObject& nested = measurements.createNestedObject();
  nested["distance"] = distance;

  return root;
}

void flashLed(int delayTime, int numberOfFlashes) {
  for (int i=0; i<numberOfFlashes; i++) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(delayTime);
    digitalWrite(BUILTIN_LED, LOW);
  }
}

