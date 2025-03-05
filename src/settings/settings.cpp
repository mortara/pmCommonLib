#include "settings.hpp"
#include "../pmCommonLib.hpp"

void pmSettingsClass::Setup()
{
    
    

    RegisterSetting("DeviceName");

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

void pmSettingsClass::RegisterSetting(String name)
{

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

        html += "<label for='"+name+"'>" + setting->Name + "</label>";
        html += "<input type='text' id ='"+name+"' name='"+name+"' value='" + setting->Value + "'><br>";
    }

    html += "<input type ='submit' value ='Submit'></p></form>";
  
    return html;
}

String pmSettingsClass::ProcessConfigPagePOST(AsyncWebServerRequest *request)
{
    return "";
}