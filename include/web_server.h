#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <LittleFS.h>

#include "global.h"
#include "logging.h"

void readFile(fs::FS &fs, const char *path);

void setupWebServer();

static String index_html; // Webpage served by the weather station

#endif