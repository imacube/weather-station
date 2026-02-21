#ifndef LOGGING_H
#define LOGGING_H

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE // Override the log level setting
#include <esp_log.h>

#define LOGE(format, ...) ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, __FUNCTION__, format, ##__VA_ARGS__)
#define LOGW(format, ...) ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN __FUNCTION__, format, ##__VA_ARGS__)
#define LOGI(format, ...) ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, __FUNCTION__, format, ##__VA_ARGS__)
#define LOGD(format, ...) ESP_LOG_LEVEL_LOCAL(ESP_LOG_DEBUG, __FUNCTION__, format, ##__VA_ARGS__)
#define LOGV(format, ...) ESP_LOG_LEVEL_LOCAL(ESP_LOG_VERBOSE, __FUNCTION__, format, ##__VA_ARGS__)

#endif