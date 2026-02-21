#ifndef GLOBAL_H
#define GLOBAL_H

#include <AsyncTCP.h> // Adds non-blocking asynchronous operations

/** MQTT Client */
#include <ESP32MQTTClient.h>
extern ESP32MQTTClient mqttClient;

/** Web server used for weather website */
#include <ESPAsyncWebServer.h>
extern AsyncWebServer server;
extern AsyncEventSource events;

#define VERSION "0.1.0"

#endif