#include <Arduino.h>
#include <vector>
#include <list>
#include "ArduinoJson.h"
#include "ESPAsyncWebServer.h"


#ifndef SETTINGS_H
#define SETTINGS_H

struct pmSettingsStruct
{
    String Name; 
    String Value; 
};

class pmSettingsClass
{
    private:
        bool _setup = false;
        
    public:
        std::list<pmSettingsStruct> Settings;

        void Setup();
        void Begin();
        bool IsSetup();
        void RegisterSetting(String name);

        String GetSettingValue(String name);

        String ServeConfigPageHTML(AsyncWebServerRequest *request);
        String ProcessConfigPagePOST(AsyncWebServerRequest *request); 
    };
        

#endif