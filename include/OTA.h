#ifndef OTA_H
#define OTA_H

#include "logging.h"

#include <ArduinoOTA.h>

#include "esp_ota_ops.h"

#ifdef CONFIG_APP_ROLLBACK_ENABLE
#undef CONFIG_APP_ROLLBACK_ENABLE
#endif
#define CONFIG_APP_ROLLBACK_ENABLE 1

extern "C" bool verifyOta(void);
extern "C" bool verifyRollbackLater(void);

void setupOTA(const char *hostname);
bool timeToVerifyOta();
void verifyRollback();

#endif