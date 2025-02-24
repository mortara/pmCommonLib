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
        AsyncWebServer *_webserver;
        DNSServer dnsServer;
       
    public:
        void Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound);
        AsyncWebServer *GetServer();

} ;

extern WebServerClass WebServer;

#endif