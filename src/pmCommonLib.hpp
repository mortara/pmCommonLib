#include <Arduino.h>
#include "config/config_handler.hpp"
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

    public:
        pmConfigHandler ConfigHandler;
        MQTTConnectorClass MQTTConnector;
        WebSerialLoggerClass WebSerial;
        WebServerClass WebServer;
        WIFIManagerClass WiFiManager;
        OTAHandlerClass OTAHandler;

        void Setup(bool mqtt = true, bool webserial = true, bool ota = true);
        void Start();
        void Loop();
};

extern pmCommonLibClass pmCommonLib;

#endif