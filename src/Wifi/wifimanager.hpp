#include <Arduino.h>
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#elif defined(ESP32)
#include "WiFi.h"
#endif
#include "../Webserial/webserial.hpp"
#include <WiFiManager.h>

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

        
        WiFiManager wifiManager;
        unsigned long interval = 5000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;
        
        void setupMQTT();
        bool mqttsetup = false;

    public:
        void Setup(String hostname);
        
        bool Connect();
 
        void Disconnect();
        void Loop();
        void DisplayInfo();
        bool IsConnected();
        unsigned long LastConnectionTry();
};

extern WIFIManagerClass WIFIManager;

#endif