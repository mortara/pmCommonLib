#include <Arduino.h>
#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <AsyncTCP.h>
#include <WiFi.h>
#endif
#include "config/config_handler.hpp"
#include "../Webserial/webserial.hpp"




#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#define FORMAT_LITTLEFS_IF_FAILED true

typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
    String IP;
    String Gateway;
    String DNS; 
    String ConfigMode = "dhcp";
} WIFICreds;

extern WIFICreds _credentials;


#define configFilePath "/wifi_config.txt"

class WIFIManagerClass
{
    private:
        AsyncWebServer *_WebServer;    

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
        
        
       
        bool initWiFi();

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