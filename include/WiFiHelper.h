#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include <WiFi.h>
#include <ESPmDNS.h>

#include "logging.h"

void setupWiFi(const char *ssid, const char *password);

void setHostname(const char *hostname);

#endif