/*
 * Tags.h - Library for defining enum tags for json Configuration
 * Should be used with Configuration library -> https://github.com/fart-one/configuration
 */
#ifndef Tags_h
#define Tags_h

#include "Arduino.h"
#include <map>


enum KEYS {
    SSID,
    WIFI_PASSWORD,
    MQTT_SERVER,
    MQTT_USER,
    MQTT_PASSWORD,
    OFFICE_ID,
    BEACON_DEVICE_ID
};

static std::map< KEYS, const char * > tag = {
  {SSID, "SSID"},
  {WIFI_PASSWORD, "wifiPassword"},
  {MQTT_SERVER, "mqttServer"},
  {MQTT_USER, "mqttUser"},
  {MQTT_PASSWORD, "mqttPassword"},
  {OFFICE_ID, "officeId"},
  {BEACON_DEVICE_ID, "beaconDeviceId"}
};

#endif
