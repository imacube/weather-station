#include "mqtt_helpers.h"

void setupMqtt(const char *server, const char *hostname, const char *publish_topic)
{
    mqttClient.enableDebuggingMessages();

    mqttClient.setURI(server);
    mqttClient.setMqttClientName(hostname);
    mqttClient.setAutoReconnect(true);
}

void startMqtt()
{
    LOGI("%s", F("🚀 Starting MQTT background task"));
    mqttClient.loopStart();
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    // if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    // {
    //     mqttClient.subscribe(subscribeTopic, [](const std::string &payload)
    //                          { LOGI("%s: %s", subscribeTopic, payload.c_str()); });

    //     mqttClient.subscribe("bar/#", [](const std::string &topic, const std::string &payload)
    //                          { LOGI("%s: %s", topic.c_str(), payload.c_str()); });
    // }
}

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
    mqttClient.onEventCallback(event);
    return ESP_OK;
}
#else  // IDF CHECK
void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    mqttClient.onEventCallback(event);
}
#endif // IDF CHECK
