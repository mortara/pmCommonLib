#include "pmCommonLib.hpp"


void pmCommonLibClass::Setup(bool mqtt, bool webserial, bool ota)
{
    WebServer.Setup();

    if(webserial)
    {
        WebSerial.Setup();
    }

    ConfigHandler.Setup();

    WiFiManager.Setup("");
    
    if(mqtt)
    {
        MQTTConnector.Setup("","","","",0,"","",NULL);
    }

    if(ota)
    {
        OTAHandler.Setup();
    }
}

void pmCommonLibClass::Start()
{
    WebServer.Begin();

    if(WebSerial.IsSetup())
        WebSerial.Begin(WebServer.GetServer());
}

void pmCommonLibClass::Loop()
{
    if(OTAHandler.IsSetup())
        OTAHandler.Loop();
    
    if(WebSerial.IsSetup())
        WebSerial.Loop();

    if(MQTTConnector.IsSetup())
        MQTTConnector.Loop();
    

    WiFiManager.Loop();
    
}


pmCommonLibClass pmCommonLib;