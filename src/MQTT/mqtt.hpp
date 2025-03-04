#include <Arduino.h>
#include "PubSubClient.h"
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#elif defined(ESP32)
#include "WiFi.h"
#endif
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <list>
#include <vector>

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H

enum MQTTClassType {
    SENSOR,
    SWITCH,
    SELECT,
    BUTTON,
    TEXT
  };

struct MQTTMessages
{
    String payload;
    String component;
    String topic;
    bool Retain = false;
};

typedef struct {
    String DeviceName;
    String ManuFacturer;
    String Model;
    String Broker;  
    String Port; //stations name
    String User;
    String Pass;
    bool changed = false;
    std::function<void(char*, uint8_t*, unsigned int)> callback;
} MQTTCreds;

extern MQTTCreds _mqttcredentials;

#define MQTTconfigFilePath "/mqtt_config.txt"

class MQTTConnectorClass
{
    private:
        WiFiClient *_wifiClientmqtt = nullptr;
        PubSubClient *_mqttClient = nullptr;
        bool _active = false;
        bool _reconnectneeded = false;
        unsigned long _lastConnectAttempt;
        unsigned long _lastMqTTLoop = 0;
        
        bool _setup = false;

    public:
        void Setup(std::function<void(char*, uint8_t*, unsigned int)> callback = NULL);
        void Loop();
        void PublishMessage(JsonDocument msg, String component, bool retain = false, String topic = "", MQTTClassType type = SENSOR);
        bool SendPayload(String msg, String topic, bool retain = false);
        bool isActive();
        bool IsSetup();
        bool SetupSensor(String topic, String component, String deviceclass = "", String unit = "", String icon = "");
        bool SetupSwitch(String topic, String component, String deviceclass, String icon);
        bool SetupSelect(String topic, String component, String deviceclass, String icon, std::vector<String> options);
        bool SetupButton(String topic, String component, String deviceclass, String icon);
        bool SetupText(String topic, String component, String deviceclass, String icon);
        bool Connect();

        std::list<MQTTMessages *>* Tasks = nullptr;
      
};

#endif