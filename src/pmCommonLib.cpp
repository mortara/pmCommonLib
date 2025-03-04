#include "pmCommonLib.hpp"


void pmCommonLibClass::Setup(bool mqtt, bool webserial, bool ota)
{

    ConfigHandler.Setup();

    WebServer.Setup();

    WiFiManager.Setup("");

    if(webserial)
    {
        WebSerial.Setup();
    }

    if(mqtt)
    {
        MQTTConnector.Setup(NULL);
    }

    if(ota)
    {
        OTAHandler.Setup();
    }
}

void pmCommonLibClass::Start()
{
    WebServer.Begin();

    ConfigHandler.Begin();

    if(WebSerial.IsSetup())
        WebSerial.Begin(WebServer.GetServer());
}

void pmCommonLibClass::Loop()
{
    unsigned long currentMillis = millis();
    if((currentMillis - previousMillis) < 100UL)
        return;

    previousMillis = currentMillis;

    if(OTAHandler.IsSetup())
        OTAHandler.Loop();
    
    if(WebSerial.IsSetup())
        WebSerial.Loop();

    if(MQTTConnector.IsSetup())
        MQTTConnector.Loop();
    

    WiFiManager.Loop();
    
}


pmCommonLibClass pmCommonLib;