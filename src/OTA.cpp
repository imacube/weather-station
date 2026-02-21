#include "OTA.h"

/** Put post boot diagnostic code here, it will be executed during
 * the bootup or to delay this set `verifyRollbackLater` to return `true`.
 *
 * Later call `verifyRollback` to have the system to go through the
 * verification process, which leads to this function being called.
 */
bool verifyOta() __attribute__((weak));
extern "C" bool verifyOta(void) { return true; }

/** Overriding this requires manually verifying things and running
 * the commands to mark the partition as good before things reboot.
 */
extern "C" bool verifyRollbackLater() { return true; }

/** Starts OTA */
void setupOTA(const char *hostname)
{
    ArduinoOTA.setHostname(hostname);

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA
        .onStart([]()
                 {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      LOGI("Start updating %s", type); })
        .onEnd([]()
               { LOGI("End"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { LOGI("Progress: %u%%", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
      LOGI("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) LOGI("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) LOGI("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) LOGI("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) LOGI("Receive Failed");
      else if (error == OTA_END_ERROR) LOGI("End Failed"); });

    ArduinoOTA.begin();
}

void verifyRollback()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            if (verifyOta())
            {
                esp_ota_mark_app_valid_cancel_rollback();
            }
            else
            {
                LOGE("OTA verification failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
}

/**
 * OTA post boot verification
 */
bool timeToVerifyOta()
{
    LOGI("Checking OTA");
    if (millis() > 60000)
    {
        verifyRollback();
        LOGI("Successfully verified the OTA update");
        return true;
    }
    else
    {
        LOGI("Too early to verify OTA update: %d < 60000", millis());
    }
    return false;
}
