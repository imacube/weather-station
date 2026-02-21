#ifndef _TIMESTAMP_
#define _TIMESTAMP_

#include <Arduino.h> // for getLocalTime()
#include <time.h>    // struct tm, strftime

#include "logging.h"

void formatLocalTime(char timestamp[], size_t size_of_timestamp);

#endif