#include "settings.hpp"
#include "../pmCommonLib.hpp"

void pmSettingsClass::Setup()
{
    
    
    String dname = WiFi.getHostname();
    RegisterSetting("DeviceName",dname);

    #if !defined(ESP8266)
    String clockrate = String(getCpuFrequencyMhz());
    RegisterSetting("CPU frequency",clockrate);
    #endif
    
    JsonDocument doc = pmCommonLib.ConfigHandler.LoadConfigFile(settingsconfigFilePath);
    if(doc["devicename"].is<String>())
    {
        for (std::list<pmSettingsStruct>::iterator setting = Settings.begin(); setting != Settings.end(); ++setting)
        {
            String sname = setting->Name;
            sname.toLowerCase();
            if(doc[sname].is<String>())
                setting->Value = doc[sname].as<String>();

            if(sname == "devicename")
            {
                Serial.println("New device name: " + setting->Value);
                WiFi.setHostname(setting->Value.c_str());
            }

            #if !defined(ESP8266)
            if(sname == "cpu frequency")
            {
                Serial.println("cpu frequency changed: " + setting->Value);
                long intval = setting->Value.toInt();
                setCpuFrequencyMhz(intval);
            }
            #endif
        }
    }

    _setup = true;
}

bool pmSettingsClass::IsSetup()
{
    return _setup;
}

void pmSettingsClass::Begin()
{
    if(!_setup)
    {
        Serial.println("SettingsClass has not been setup yet!");
        return;
    }

    ConfigHTTPRegisterFunction f1 = std::bind(&pmSettingsClass::ServeConfigPageHTML, this, std::placeholders::_1);
    ConfigHTTPRegisterFunction f2 = std::bind(&pmSettingsClass::ProcessConfigPagePOST, this, std::placeholders::_1);

    pmCommonLib.ConfigHandler.RegisterConfigPage("general", f1, f2);
}

void pmSettingsClass::RegisterSetting(String name, String defaultvalue, String label, pmSettingTypes type)
{
    Serial.println("registering new setting: " + name);

    pmSettingsStruct newsetting;
    newsetting.Name = name;
    newsetting.Value = defaultvalue;
    newsetting.Label = label;
    newsetting.Type = type;

    Settings.push_back(newsetting);
}

String pmSettingsClass::GetSettingValue(String name)
{
    name.toLowerCase();
    for (std::list<pmSettingsStruct>::iterator setting = Settings.begin(); setting != Settings.end(); ++setting)
    {
        String sname = setting->Name;
        sname.toLowerCase();
        if(sname == name)
            return String(setting->Value);
    }

    return "";
}

String pmSettingsClass::ServeConfigPageHTML(AsyncWebServerRequest *request)
{
    Serial.println("Webserver handle request ... ");
  
    
    String html = "<form action='/config/general.html' method='POST'><p>";

    for (std::list<pmSettingsStruct>::iterator setting = Settings.begin(); setting != Settings.end(); ++setting)
    {
        String name = setting->Name;
        name.toLowerCase();
        String label = setting->Label;
        if(label == "")
            label = setting->Name;

        html += "<label for='"+name+"'>" + label + "</label>";
        html += "<input type='text' id ='"+name+"' name='"+name+"' value='" + setting->Value + "'><br>";
    }

    html += "<input type ='submit' value ='Submit'></p></form>";
  
    return html;
}

String pmSettingsClass::ProcessConfigPagePOST(AsyncWebServerRequest *request)
{
    Serial.println("Handling Settings POST request ...");
    JsonDocument data;

    int params = request->params();

    for (std::list<pmSettingsStruct>::iterator setting = Settings.begin(); setting != Settings.end(); ++setting)
    {
        String sname = setting->Name;
        sname.toLowerCase();

        String value = String(setting->Value);

        for(int i=0;i<params;i++)
        {
            const AsyncWebParameter* p = request->getParam(i);
            String pname = p->name();
            pname.toLowerCase();
            if(p->isPost())
            {
                if (pname == sname) {
                    data[sname] = p->value();
                    Serial.println(setting->Name + " set to: " + p->value());
                    setting->Value = p->value();
                }
            }
        }

        if(value != setting->Value)
        {
            if(sname == "devicename")
            {
                Serial.println("New device name: " + setting->Value);
                WiFi.setHostname(setting->Value.c_str());
            }

            #if !defined(ESP8266)
            if(sname == "cpu frequency")
            {
                Serial.println("cpu frequency changed: " + setting->Value);
                long intval = setting->Value.toInt();
                setCpuFrequencyMhz(intval);
            }
            #endif
        }
    }

    pmCommonLib.ConfigHandler.SaveConfigFile(settingsconfigFilePath, data);

    
    return ServeConfigPageHTML(request);
}