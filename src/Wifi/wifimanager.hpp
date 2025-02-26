#include <Arduino.h>
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#elif defined(ESP32)
#include "WiFi.h"
#endif
#include "../Webserial/webserial.hpp"

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
} WIFICreds;

class WIFIManagerClass
{
    private:
        bool connecting = false;
        bool connected = false;

        WIFICreds _credentials;
        unsigned long interval = 5000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;
        
        void setupMQTT();
        bool mqttsetup = false;

    public:

        void Setup(String hostname, String SSID, String PASS);
        bool Connect();
        bool Connect(String SSID, String PASS);
        void Disconnect();
        void Loop();
        void DisplayInfo();
        bool IsConnected();
        unsigned long LastConnectionTry();
};

extern WIFIManagerClass WIFIManager;

#endif