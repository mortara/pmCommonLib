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
    String Subnet;
    String Gateway;
    String DNS; 
    String ConfigMode = "dhcp";
    String Hostname = "";
} WIFICreds;

extern WIFICreds _wificredentials;


#define WIFIconfigFilePath "/wifi_config.txt"

class WIFIManagerClass
{
    private:
        AsyncWebServer *_WebServer;    

        bool connecting = false;
        bool captiveportalactive = false;
        bool connected = false;
        unsigned long interval = 10000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;

        // Timer variables
        unsigned long previousMillis = 0;
      
        #ifndef PMCOMMONNOMQTT
        void setupMQTT();
        bool mqttsetup = false;
        #endif
        
        bool initWiFi();

    public:
        void Setup(bool autoconnect = true);
        void Begin();
        bool Connect();
        void StartCaptivePortal();

        void Disconnect();
        void Loop();
        void DisplayInfo();
        bool IsConnected();
        unsigned long LastConnectionTry();
        void LoadConfig();
        void SaveConfig();
        IPAddress GetIP();

        String captivePortalRedirector(AsyncWebServerRequest *request);
        String handlePOSTrequest(AsyncWebServerRequest *request);
        String handleWifManagerRoot(AsyncWebServerRequest *request);
};

#endif