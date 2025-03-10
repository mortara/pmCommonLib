#include <Arduino.h>
#include <DNSServer.h>
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#endif

#include "ESPAsyncWebServer.h"

#ifndef WEBSERVER_H
#define WEBSERVER_H

class WebServerClass
{
    private:
        AsyncWebServer *_webserver = nullptr;
        DNSServer *_dnsServer = nullptr;
      
        bool _setup = false;
    public:
        void Setup();
        void Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound);
        bool IsSetup();
        void Reset();
        AsyncWebServer *GetServer();

        void Begin();
        void RegisterOn(const char *path, ArRequestHandlerFunction onRequest, WebRequestMethod method = HTTP_GET);
        void RegisterNotFound(ArRequestHandlerFunction onRequest);
} ;

#endif