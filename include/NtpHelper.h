#ifndef NTP_HELPER_H
#define NTP_HELPER_H

#include <WiFi.h>
#include <esp_sntp.h>
#include <time.h>

#include "logging.h"

void setupNTP(const char *ntp_timezone, const char *ntp_server_1, const char *ntp_server_2);

#endif