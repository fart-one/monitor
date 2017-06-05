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
