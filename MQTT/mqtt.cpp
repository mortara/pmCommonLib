#include "mqtt.hpp"

void callback(char* topic, byte* payload, unsigned int length) {
    
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }
    WebSerialLogger.println(msg);
}


void MQTTConnectorClass::Setup(String devicename, String mqttbroker, int port, String username, String password)
{
     WebSerialLogger.println("Initializing MQTT client");

     device_id = devicename;
     _user = username;
    _pass = password;

    _wifiClientmqtt = new WiFiClient();

    _mqttClient = new PubSubClient(mqttbroker.c_str(), port, *_wifiClientmqtt);
    _mqttClient->setBufferSize(4096);
    _mqttClient->setCallback(callback);
    
    Tasks = new std::list<MQTTMessages *>();
    TaskHandle_t xHandle = NULL;

    xTaskCreatePinnedToCore(
        Task1code, /* Function to implement the task */
        "Task1", /* Name of the task */
        10000,  /* Stack size in words */
        this,  /* Task input parameter */
        0,  /* Priority of the task */
        &xHandle,  /* Task handle. */
        0); /* Core where the task should run */

    _lastConnectAttempt = millis();
}

bool MQTTConnectorClass::isActive()
{
    if(WiFi.status() != WL_CONNECTED)
        _active = false;

    return _active;
}

void MQTTConnectorClass::Loop()
{
    unsigned long now = millis();

    if(!_active && WiFi.status() == WL_CONNECTED && now - _lastConnectAttempt > 5000UL)
    {
        Connect();
    }

    if(now - _lastMqTTLoop > 5000UL)
    {
        _lastMqTTLoop = now;
        _mqttClient->loop();
    }
}

bool MQTTConnectorClass::Connect()
{
    WebSerialLogger.println("Connecting mqtt client ...");

    if(_wifiClientmqtt == NULL)
    {
        
        _wifiClientmqtt = new WiFiClient();
        _mqttClient->setClient(*_wifiClientmqtt);
    }

    _lastConnectAttempt = millis();
    if(!_mqttClient->connect(device_id.c_str(), _user.c_str(), _pass.c_str()))
    {
        WebSerialLogger.println("Could not connect to MQTT broker!");
        WebSerialLogger.println("State: " + String(_mqttClient->state()));
    }
    else
    {
        _mqttClient->subscribe("motd");
        _active = true;
    }

    return _active;
}

bool MQTTConnectorClass::SetupSensor(String topic, String sensor, String component, String deviceclass, String unit, String icon)
{
    
    if(!_active)
        return false;

    //WebSerialLogger.println("Configuring sensor "+ topic);
    String header = "homeassistant/" + sensor + "/" + device_id + "_" + component + "_" + topic;

    String config_topic = header + "/config";
	String name = device_id + "_" + topic;

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    if(unit != "")
        root["unit_of_measurement"] = unit;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["uniq_id"] = name;
    root["state_topic"] = header + "/state";

    root["entity_category"] = "diagnostic";
    
    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = component;


    PublishMessage(root, component, true, config_topic);
   
    return true;
}

bool MQTTConnectorClass::SendPayload(String payload, String topic, bool retain)
{
    
    if(!_active)
        return false;
    
    if(!_mqttClient->publish(topic.c_str(), payload.c_str(), retain))
    {
        WebSerialLogger.println("Error publishing data!");
        WebSerialLogger.println("State: " + String(_mqttClient->state()));

        if(_mqttClient->state() == -3)
        {
            WebSerialLogger.println("MQTT connection lost ...");
            _mqttClient->disconnect();
            _active = false;
        }
        return false;
    }

    return true;
}

void MQTTConnectorClass::PublishMessage(JsonDocument root, String component, bool retain, String topic, String sensor)
{
    
    if(root == NULL)
        return;

    String header = "homeassistant/" + sensor + "/" + device_id + "_" + component + "_" + topic;

    String msg;
    size_t size = serializeJson(root, msg);
    
    //WebSerialLogger.println("Sending mqtt message: " + String(msg));
    if(size > 1024)
        WebSerialLogger.println("Size : " + String(size));

    if(topic == "")
        topic = header + "/state";

    MQTTMessages *bt = new MQTTMessages();
    bt->payload = msg;
    bt->topic = topic;
    bt->Retain = retain;

    while(lock)
        delay(1);

    lock = true;
    //SendPayload(msg, component, retain);

    Tasks->push_back(bt);
    lock = false;

    _mqttClient->loop();
}

void MQTTConnectorClass::Task1code(void *pvParameters) 
{
    
    // Add a delay to give the rest of the modules some time to setup
    delay(15000);
    WebSerialLogger.println("MQTT Loop starting ...");
    
    for(;;) 
    {
        delay(100);
        if(MQTTConnector.Tasks->empty() || !MQTTConnector.isActive())
        {
            delay(1000);
        }
        else
        {
            while(MQTTConnector.lock)
                delay(1);

            MQTTConnector.lock = true;
            MQTTMessages *bt = MQTTConnector.Tasks->front();
            MQTTConnector.lock = false;
            
            bool ok = MQTTConnector.SendPayload(bt->payload, bt->topic, bt->Retain);

            MQTTConnector.lock = true;
            MQTTConnector.Tasks->remove(bt);
            if(!ok)
            {
                WebSerialLogger.println("unable to publish mqtt message ...");
                MQTTConnector.Tasks->push_back(bt);
                delay(1000);
            }
            else
                delete bt;
            MQTTConnector.lock = false;
            
        }
    }
}

MQTTConnectorClass MQTTConnector;