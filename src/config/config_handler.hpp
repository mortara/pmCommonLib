#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"
#include <vector>
#include <list>
#include <utility>
#include "ESPAsyncWebServer.h"

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H


typedef std::function<String(AsyncWebServerRequest *request)> ConfigHTTPRegisterFunction;

struct ConfigPageDefinition
{
    String Name;
    String Title;
    ConfigHTTPRegisterFunction GETMethod;
    ConfigHTTPRegisterFunction POSTMethod;
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

        void RegisterConfigPage(String name, ConfigHTTPRegisterFunction onGetRequest, ConfigHTTPRegisterFunction onPostRequest, String title = "");

        // Shared HTML building blocks so all config pages look and behave the same.
        static String HtmlEscape(const String &in);
        static String FormStart(const String &page);
        static String FormEnd(const String &submitLabel = "Save");
        static String TextField(const String &name, const String &label, const String &value, const String &type = "text", const String &hint = "");
        static String SelectField(const String &name, const String &label, const std::vector<std::pair<String, String>> &options, const String &selected);
        static String Notice(const String &message, bool error = false);

        std::list<ConfigPageDefinition> ConfigPages;
};

#endif