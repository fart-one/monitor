//Configuration.h

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

/////////////////////
//WiFi and MQTT def//
/////////////////////
const char* SSID = "SSID";
const char* wiFiPassword = "WiFi Password";
const char* mqttServer = "mqtt.broker.info";
const char* mqttUser = "mqtt Login";
const char* mqttPassword = "mqtt Password";
#define BUFFER_SIZE 100
