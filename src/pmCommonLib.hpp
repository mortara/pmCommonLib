#include <Arduino.h>
#include "logging/logging.hpp"
#include "config/config_handler.hpp"
#include "settings/settings.hpp"
#include "OTA/ota_handler.hpp"
#include "WiFi/wifimanager.hpp"

#ifndef PMCOMMONNOMQTT
#include "MQTT/mqtt.hpp"
#endif

#ifndef PMCOMMONNOWEBSERIAL
#include "Webserial/webserial.hpp"
#endif

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

        #ifndef PMCOMMONNOMQTT
        MQTTConnectorClass MQTTConnector;
        #endif

        #ifndef PMCOMMONNOWEBSERIAL
        WebSerialLoggerClass WebSerial;
        #endif

        WebServerClass WebServer;
        WIFIManagerClass WiFiManager;
        OTAHandlerClass OTAHandler;

        void Setup(bool mqtt = true, bool webserial = true, bool ota = true, bool general = true);
        void Start();
        void Loop();
};

extern pmCommonLibClass pmCommonLib;

#endif