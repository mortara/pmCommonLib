#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"
#include <vector>
#include "ESPAsyncWebServer.h"

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H


struct ConfigPageDefinition
{
    String Name;
    std::function<String(AsyncWebServerRequest *request)> GETMethod;
    std::function<String(AsyncWebServerRequest *request)> POSTMethod;
};



class pmConfigHandler
{
    private:
        bool _setup = false;
        bool initLittleFS();
        

    public:
        bool Setup();
        void Begin();
        String readFile(fs::FS &fs, const char * path);
        void writeFile(fs::FS &fs, const char * path, const char * message);

        bool SaveConfigFile(const char * name, JsonDocument data);
        JsonDocument LoadConfigFile(const char * name);

        void RegisterConfigPage(String name, std::function<String(AsyncWebServerRequest *request)> onGetRequest, std::function<String(AsyncWebServerRequest *request)> onPostRequest);

        std::list<ConfigPageDefinition> ConfigPages;
};

#endif