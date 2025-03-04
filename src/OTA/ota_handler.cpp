#include "ota_handler.hpp"
#include "../pmCommonLib.hpp"

void OTAHandlerClass::Setup()
{
    _setup = true;
}

void OTAHandlerClass::Start()
{
    Serial.println("Startup ota handler!");
    if(!_setup)
        return;

    ArduinoOTA.onStart(std::bind(&OTAHandlerClass::onStart, this));
    ArduinoOTA.onEnd(std::bind(&OTAHandlerClass::onEnd, this));
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) { this->onProgress(progress, total); });
    ArduinoOTA.onError([this](ota_error_t error) { this->onError(error); });
    ArduinoOTA.begin();

    pmCommonLib.WebSerial.println("OTA started");
    ota_running = true;
    ota_timer = millis();
}

bool OTAHandlerClass::IsSetup()
{
    return _setup;
}

void OTAHandlerClass::onStart()
{
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) 
    {
        type = "sketch";
    } else {  // U_FS
        type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    pmCommonLib.WebSerial.println("Start updating " + type);
    OTAOnly = true;
}

void OTAHandlerClass::onEnd()
{
    pmCommonLib.WebSerial.println("Done!");
    pmCommonLib.WebSerial.println("Rebooting");

    delay(1000);
}

void OTAHandlerClass::onProgress(unsigned int progress, unsigned int total)
{
    int perc = (progress / (total / 100));

    if(perc != last_perc)
    {
        String str = "Progress: " + String(perc) + "\r";
        pmCommonLib.WebSerial.println(str.c_str());
        last_perc = perc;
    }
}

void OTAHandlerClass::onError(ota_error_t error)
{
    Serial.printf("Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR) {
        pmCommonLib.WebSerial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
        pmCommonLib.WebSerial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
        pmCommonLib.WebSerial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
        pmCommonLib.WebSerial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
        pmCommonLib.WebSerial.println("End Failed");
    }
}

void OTAHandlerClass::Loop()
{
    if(!_setup)
        return;

    unsigned long now = millis();
   
    if(now - ota_timer > 100UL)
    {
      if(ota_running)
        ArduinoOTA.handle();
      else
      {
        if(WiFi.isConnected())
          Start();
      }
      ota_timer = now;
    }
}