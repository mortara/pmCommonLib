#include <Arduino.h>
#include "PubSubClient.h"
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#elif defined(ESP32)
#include "WiFi.h"
#endif
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "../Webserial/webserial.hpp"
#include <list>

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H


struct MQTTMessages
{
    String payload;
    String component;
    String topic;
    bool Retain = false;
};

class MQTTConnectorClass
{
    private:
        WiFiClient *_wifiClientmqtt = NULL;
        PubSubClient *_mqttClient;
        bool _active = false;
        String device_id = "randomesp32device";
        unsigned long _lastConnectAttempt;
        unsigned long _lastMqTTLoop = 0;
        String _user = "";
        String _pass = "";
        
    public:
        void Setup(String devicename, const char* mqttbroker, int port, String username, String password);
        void Loop();
        void PublishMessage(JsonDocument msg, String component, bool retain = false, String topic = "", String sensor = "sensor");
        bool SendPayload(String msg, String topic, bool retain = false);
        bool isActive();
        bool SetupSensor(String topic, String sensor, String component, String deviceclass = "", String unit = "", String icon = "");
        bool Connect();

        std::list<MQTTMessages *>* Tasks;
        volatile bool lock;   
        static void Task1code(void *pvParameters);
};

extern MQTTConnectorClass MQTTConnector;

#endif