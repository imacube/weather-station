#include "WiFiHelper.h"

/** Configures and starts WiFi.
 *
 * If `HOSTNAME` is defined then mDNS is started and hostname is set.
 */
void setupWiFi(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA); // WiFi station
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    LOGI("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}

void setHostname(const char *hostname)
{
  WiFi.setHostname(hostname);

  if (MDNS.begin(hostname))
  {
    LOGI("mDNS responder started");
  }
}
