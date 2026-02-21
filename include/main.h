#ifndef MAIN_H
#define MAIN_H

#include "secret.h"
#include "global.h"
#include "logging.h"

/** Network stuff */
#include <WiFiUdp.h>
#include <Syslog.h>

/** Watchdog Timer */
#include <esp_task_wdt.h>

/** Network transmision of data */
#include <cJSON.h>

#define ENABLE_CUSTOM_LOGGING
#define ENABLE_SYSLOG
#define ENABLE_SERIAL_SYSLOG
// #define ENABLE_SERIAL

#endif