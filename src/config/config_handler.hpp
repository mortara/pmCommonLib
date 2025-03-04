#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H

class pmConfigHandler
{
    private:
        bool _setup = false;
        bool initLittleFS();
    public:
        String readFile(fs::FS &fs, const char * path);
        void writeFile(fs::FS &fs, const char * path, const char * message);

        bool SaveConfigFile(const char * name, JsonDocument data);
        JsonDocument LoadConfigFile(const char * name);
};

extern pmConfigHandler ConfigHandler;
#endif