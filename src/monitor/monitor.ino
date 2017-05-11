// HC-SR04: VCC - 5V, trigger - 3V
// WeMos D1 (ESP8266): D1 - trigger, D2 - echo, D0 and RST connected for deep sleep mode
// MQTT using https://github.com/Imroy/pubsubclient
// JSON using: https://github.com/bblanchon/ArduinoJson

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Configuration.h>
#include <FS.h>

/////////////////////////////////
//Global variable to toggle LED//
/////////////////////////////////
int lastStatus;

/////////////////////
//Occupation status//
////////////////////
#define STATE_UNKNOWN 0
#define STATE_UNOCCUPIED 10
#define STATE_OCCUPIED 20

////////////////////////
//Location definitions//
////////////////////////
#define OFFICE "LegitOfficeLocationId"
#define VERSION 1
const int sleepTime = 1; // in seconds

/////////////////////////////
//Configuration definitions//
/////////////////////////////
const String configFile = "/config.json";
StaticJsonBuffer<256> configJsonBuffer;
// TLS
const char* fingerprint = "49 F5 0D 4C F3 3D 5F D4 7E BB D3 81 63 77 0C 30 07 2C FD AF";
#define MQTT_TLS_PORT 8883

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

/*
   MQTT const variables - they need to be here
   we read them once in setup() and use them many times in loop()
*/
const char *mqttServer;
const char *mqttUser;
const char *mqttPassword;
const char *officeId;

//////////////
//Connection//
//////////////
WiFiClient wclient;
PubSubClient client(wclient, mqttServer);
Configuration conf;

void setup_wifi() {

  //Load config from json
  
  Serial.println(conf.getWifiSSID());
  Serial.println("Reading config file");
  Serial.print("Got SSID from file:");
  Serial.print(conf.getWifiSSID());
  Serial.println();


 
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
      String topic = "toilet/" + String(officeId) + "/" + deviceId;
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
  conf = Configuration(configFile.c_str());
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
  
  /*int intDeviceId = ESP.getChipId();
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

  delay(sleepTime * 1000);*/
  Serial.println("loop");
  delay(10000);
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

