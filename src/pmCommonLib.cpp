#include "pmCommonLib.hpp"


void pmCommonLibClass::Setup(bool mqtt, bool webserial, bool ota, bool general)
{

    ConfigHandler.Setup();

    if(general)
    {
        Settings.Setup();
    }

    WebServer.Setup();

    WiFiManager.Setup("");

    #ifndef PMCOMMONNOWEBSERIAL
    if(webserial)
    {
        WebSerial.Setup();
    }
    #endif

    #ifndef PMCOMMONNOMQTT
    if(mqtt)
    {
        MQTTConnector.Setup(NULL);
    }
    #endif

    if(ota)
    {
        OTAHandler.Setup();
    }
}

void pmCommonLibClass::Start()
{
    WebServer.Begin();

    if(Settings.IsSetup())
        Settings.Begin();

    WiFiManager.Begin();

    #ifndef PMCOMMONNOMQTT
    if(MQTTConnector.IsSetup())
        MQTTConnector.Begin();
    #endif

    ConfigHandler.Begin();
    
    #ifndef PMCOMMONNOWEBSERIAL
    if(WebSerial.IsSetup())
        WebSerial.Begin(WebServer.GetServer());
    #endif
}

void pmCommonLibClass::Loop()
{
    unsigned long currentMillis = millis();
    if((currentMillis - previousMillis) < 100UL)
        return;

    previousMillis = currentMillis;

    if(OTAHandler.IsSetup())
        OTAHandler.Loop();
    
    #ifndef PMCOMMONNOWEBSERIAL
    if(WebSerial.IsSetup())
        WebSerial.Loop();
    #endif

    #ifndef PMCOMMONNOMQTT
    if(MQTTConnector.IsSetup())
        MQTTConnector.Loop();
    #endif

    WiFiManager.Loop();
    
}


pmCommonLibClass pmCommonLib;