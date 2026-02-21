#include "NtpHelper.h"

void setupNTP(const char *ntp_timezone, const char *ntp_server_1, const char *ntp_server_2)
{
    /**
     * A more convenient approach to handle TimeZones with daylightOffset
     * would be to specify a environment variable with TimeZone definition including daylight adjustmnet rules.
     * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
     */
    configTzTime(ntp_timezone, ntp_server_1, ntp_server_2);

    LOGI("Syncing NTP");
    struct tm timeinfo;
    while (true)
    {
        if (getLocalTime(&timeinfo))
        {
            LOGI("%d-%d-%dT%d:%d:%d", timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            return;
        }
        LOGI("Waiting for NTP sync...");
        delay(1000);
    }
    LOGI("Failed to sync time.");
}
