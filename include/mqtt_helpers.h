#ifndef MQTT_HELPERS_H
#define MQTT_HELPERS_H

#include <ESP32MQTTClient.h>
#include "global.h"
#include "logging.h"

void setupMqtt(const char *server, const char *hostname, const char *publish_topic);

void startMqtt();

#endif