
#include <Arduino.h>
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#endif

#include "ESPAsyncWebServer.h"
#include <MycilaWebSerial.h>

#ifndef WEBSERIAL_H
#define WEBSERIAL_H


class WebSerialLoggerClass
{
    private:
        
        WebSerial webSerial;
        bool running = false;

    public:
        void Start();
        void Begin(AsyncWebServer *_server);
        bool IsRunning();
        void print(const char *text);
        void print(String text = "");
        void println(const char *text);
        void println(String text = "");
        void Flush();
        void Loop();
        char GetInput();
};


extern WebSerialLoggerClass WebSerialLogger;
#endif