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
#define OFFICE "Some office id"
#define VERSION 1
const int sleepTime = 1; // in seconds

/////////////////////
//WiFi and MQTT def//
/////////////////////
const char* SSID = "ssid";
const char* wiFiPassword = "WiFi password";
const char* mqttServer = "broker";
const char* mqttUser = "mqttlogin";
const char* mqttPassword = "mqttpswd";
#define BUFFER_SIZE 100

//////////////////
//LED definition//
/////////////////

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
      client.publish(MQTT::Publish(topic, "connecting").set_qos(1).set_retain(1));
      client.loop();
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
  if (pub.has_stream()) {
    uint8_t buf[BUFFER_SIZE];
    int read;
    while (read = pub.payload_stream()->read(buf, BUFFER_SIZE)) {
      Serial.write(buf, read);
    }
    pub.payload_stream()->stop();
    Serial.println("");
  } else {
    Serial.println(pub.payload_string());
  }
}

void mqttPublish(String message, int intDeviceId) {
  client.set_callback(mqttCallback);
  client.publish(MQTT::Publish("/test/ping/", message)
                        .set_qos(1)
                        .set_retain()
                 );
}

void setup() {
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT); 
  
  setup_wifi();
}

void loop() {
  
  int intDeviceId = ESP.getChipId();
  String deviceId = String(intDeviceId);

  StaticJsonBuffer<256> jsonBuffer;
  
  Serial.println("--------------------------------");
  
  if (!client.connected()) {
    reconnect_mqtt(deviceId);
  } else {
    JsonObject& jsonMessage = buildJson(jsonBuffer, 100, intDeviceId, VERSION, 2);
    String message;
    jsonMessage.printTo(message);

    //mqttPublish(message, intDeviceId);
  }

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

