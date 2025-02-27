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
#include <vector>

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H

enum MQTTClassType {
    SENSOR,
    SWITCH,
    SELECT,
    BUTTON
  };

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
        WiFiClient *_wifiClientmqtt = nullptr;
        PubSubClient *_mqttClient = nullptr;
        bool _active = false;
        String device_id = "randomesp32device";
        unsigned long _lastConnectAttempt;
        unsigned long _lastMqTTLoop = 0;
        String _user = "";
        String _pass = "";
        String _manufacturer = "";
        String _model = "";
        
    public:
        void Setup(String devicename, String model, String manufacturer, const char* mqttbroker, int port, String username, String password, std::function<void(char*, uint8_t*, unsigned int)> callback);
        void Loop();
        void PublishMessage(JsonDocument msg, String component, bool retain = false, String topic = "", MQTTClassType type = SENSOR);
        bool SendPayload(String msg, String topic, bool retain = false);
        bool isActive();
        bool SetupSensor(String topic, String component, String deviceclass = "", String unit = "", String icon = "");
        bool SetupSwitch(String topic, String component, String deviceclass, String icon);
        bool SetupSelect(String topic, String component, String deviceclass, String icon, std::vector<String> options);
        bool SetupButton(String topic, String component, String deviceclass, String icon);
        bool Connect();

        std::list<MQTTMessages *>* Tasks = nullptr;
        volatile bool lock;   
        
};

extern MQTTConnectorClass MQTTConnector;

#endif