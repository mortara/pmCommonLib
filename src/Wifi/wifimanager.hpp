#include <Arduino.h>
#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <AsyncTCP.h>
#include <WiFi.h>
#endif
#include "../Webserial/webserial.hpp"
#include "LittleFS.h"

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
    String IP;
    String Gateway;
} WIFICreds;

extern WIFICreds _credentials;

#define PARAM_INPUT_1 "ssid"
#define PARAM_INPUT_2 "pass"
#define PARAM_INPUT_3 "ip"
#define PARAM_INPUT_4 "gateway"
#define ssidPath "/ssid.txt"
#define passPath "/pass.txt"
#define ipPath "/ip.txt"
#define gatewayPath "/gateway.txt"


class WIFIManagerClass
{
    private:
        
        bool connecting = false;
        bool connected = false;
     

        unsigned long interval = 10000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;

        // Timer variables
        unsigned long previousMillis = 0;
      
        void setupMQTT();
        bool mqttsetup = false;

        
        IPAddress localIP;
        //IPAddress localIP(192, 168, 1, 200); // hardcoded
        String processor(const String& var);
        // Set your Gateway IP address
        IPAddress localGateway;
        //IPAddress localGateway(192, 168, 1, 1); //hardcoded
        
        void initLittleFS();
       
        bool initWiFi();

    public:

        void Setup(String hostname, AsyncWebServer *webserver);
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