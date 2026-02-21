#include "web_server.h"

void readFile(fs::FS &fs, const char *path)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        LOGE("❌ Failed to open %s", path);
        return;
    }
    index_html = file.readString();
}

void setupWebServer()
{
    if (!LittleFS.begin())
    {
        LOGE("%s", F("❌ LittleFS mount failed"));
        return;
    }

    readFile(LittleFS, "/webpage.html");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html;charset=utf-8", index_html); });
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET");

    // Handle Web Server Events
    events.onConnect(
        [](AsyncEventSourceClient *client)
        {
            if (client->lastId())
            {
                LOGD("WebServer: Client reconnected! Last message ID that it got is: %u", client->lastId());
            }
            // send event with message "hello!", id current millis
            // and set reconnect delay to 10 second
            client->send("hello!", NULL, millis(), 10000);
        });
    server.addHandler(&events);
}