#include <Arduino.h>
#include "config/config_handler.hpp"
#include "settings/settings.hpp"
#include "OTA/ota_handler.hpp"
#include "WiFi/wifimanager.hpp"
#include "MQTT/mqtt.hpp"
#include "Webserial/webserial.hpp"
#include "Webserver/webserver.hpp"

#ifndef PMCOMMONLIB_H
#define PMCOMMONLIB_H

class pmCommonLibClass
{
    private:
        unsigned long previousMillis = 0;
    public:
        pmConfigHandler ConfigHandler;
        pmSettingsClass Settings;
        MQTTConnectorClass MQTTConnector;
        WebSerialLoggerClass WebSerial;
        WebServerClass WebServer;
        WIFIManagerClass WiFiManager;
        OTAHandlerClass OTAHandler;

        void Setup(bool mqtt = true, bool webserial = true, bool ota = true, bool general = true);
        void Start();
        void Loop();
};

extern pmCommonLibClass pmCommonLib;

#endif