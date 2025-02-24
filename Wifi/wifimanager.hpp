#include <Arduino.h>
#include <WiFi.h>
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
        unsigned long interval = 15000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;
        TaskHandle_t Task1;
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