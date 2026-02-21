#include "main.h"

/** Network Stuff */
#include "WiFiHelper.h"
WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, LOG_KERN);

/** NTP & Timetamps */
#include "NtpHelper.h"
#include "Timestamp.h"

/** OTA */
#include "OTA.h"
bool otaVerified = false;

/** MQTT */
#include "mqtt_helpers.h"
ESP32MQTTClient mqttClient;

/** Webserver */
#include "web_server.h"
AsyncWebServer server(80);
AsyncEventSource events("/events");

/** Lightning */
#include "lightning.h"
SparkFun_AS3935 lightning;

/** SparkFun_Weather_Meter_Kit_Arduino_Library */
#include "weather_sensors.h"
Adafruit_DS248x ds248x;
uint8_t temperatureProbeAddr[8]; // 1-wire address of the probe
BME280 bme280;
SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);

/** Sensor check frequency */
static uint32_t loopDelay = 100;                  // Delay at the end of each loop()
static unsigned long nextSensorCheck = 0;         // When the sensors were last checked
static unsigned long sensorCheckFrequency = 2000; // How often to check the sensors

int syslog_method(const char *fmt, va_list args)
{
  int result = 0;
#ifdef ENABLE_SERIAL
  result = vprintf(fmt, args);
#endif

  char logBuff[1024];
  result = snprintf(logBuff, sizeof(logBuff), "heap(%d) version(%s) ", esp_get_free_heap_size(), VERSION);
  result = vsnprintf(logBuff + strlen(logBuff), sizeof(logBuff) - strlen(logBuff), fmt, args);
  if (result < 0)
  {
    syslog.logf(LOG_ERR, "vsnprintf encoding error: %d", result);
  }
  else if (result > sizeof(logBuff))
  {
    syslog.logf(LOG_ERR, "log buffer too small %d > %d; log message: %s", result, sizeof(logBuff), logBuff);
  }
  else
  {
#ifdef ENABLE_SYSLOG
    syslog.log(logBuff);
#endif

#ifdef ENABLE_SERIAL_SYSLOG
    Serial.printf(logBuff);
#endif
  }

  return result;
}

void setup()
{
  esp_log_level_set("*", ESP_LOG_VERBOSE);
#ifdef ENABLE_CUSTOM_LOGGING
  esp_log_set_vprintf(syslog_method);
#endif

  const int timeoutSec = 120;          // timeout in seconds
  esp_task_wdt_init(timeoutSec, true); // true = panic on timeout
  esp_task_wdt_add(nullptr);           // add the Arduino "loop" task

  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  /** WiFi setup */
  setupWiFi(WIFI_SSID, WIFI_PASSWORD);
  setHostname(HOSTNAME);

  /** Post WiFi */
  setupOTA(HOSTNAME);
  setupNTP(NTP_TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
  setupMqtt(MQTT_SERVER, HOSTNAME, MQTT_PUBLISH_TOPIC);
  startMqtt();
  setupWebServer();
  server.begin();

  /** Bring sensors up */
  setupLightning(spiCS, lightningInt, lightning);
  setupWeatherSensors(bme280, weatherMeterKit, ds248x, temperatureProbeAddr);

  char tempSensorAddress[64];
  snprintf(tempSensorAddress, sizeof(tempSensorAddress), "Temperature sensor address: ");
  for (int i = 0; i < 8; i++)
  {
    snprintf(tempSensorAddress + strlen(tempSensorAddress), sizeof(tempSensorAddress) - strlen(tempSensorAddress), "%d ", temperatureProbeAddr[i], HEX);
  }
  LOGI("%s", tempSensorAddress);

  LOGI("Setup complete");
}

void loop()
{
  esp_task_wdt_reset();
  ArduinoOTA.handle();
  if (!otaVerified)
  {
    otaVerified = timeToVerifyOta();
  }

  char *lightningEvent = check_lightning(lightningInt, lightning);
  if (lightningEvent == NULL)
  {
    LOGV("lightningEvent is NULL");
  }
  else if (strlen(lightningEvent) == 0)
  {
    LOGE("Empty response from lightning event sensor");
  }
  else
  {
    LOGI("Sending %s", lightningEvent);
    LOGV("Publishing JSON to mqtt");
    mqttClient.publish(MQTT_PUBLISH_TOPIC, lightningEvent, 1, true);
    LOGV("Sending lightningEvent to web server events");
    events.send(lightningEvent, "sensors", millis());
  }
  free(lightningEvent);

  /** If the next sensor check is calculated to be in the future, sleep and return */
  if (nextSensorCheck > millis())
  {
    delay(loopDelay);
    return;
  }
  else
  {
    nextSensorCheck = millis() + sensorCheckFrequency;
  }

  char *weatherSensorReadings = sendSensorReadings(bme280, weatherMeterKit, ds248x, temperatureProbeAddr);

  LOGV("Test if weatherSensorReadings is NULL");
  LOGV("Testing weatherSensorReadings length");
  if (weatherSensorReadings == NULL)
  {
    LOGD("weatherSensorReadings is NULL");
  }
  else if (strlen(weatherSensorReadings) == 0)
  {
    LOGE("Empty response from weather station sensors");
  }
  else
  {
    LOGD("Sending %s", weatherSensorReadings);
    LOGV("Publishing JSON to mqtt");
    mqttClient.publish(MQTT_PUBLISH_TOPIC, weatherSensorReadings, 1, true);
    LOGV("Sending weatherSensorReadings to web server events");
    events.send(weatherSensorReadings, "sensors", millis());
  }
  free(weatherSensorReadings);

  delay(loopDelay);
}
