#include "mqtt.hpp"

void default_callback(char* topic, byte* payload, unsigned int length) {
    
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }
    WebSerialLogger.println(msg);
}


void MQTTConnectorClass::Setup(String devicename, String model, String manufacturer, const char* mqttbroker, int port, String username, String password, std::function<void(char*, uint8_t*, unsigned int)> callback = NULL)
{
    WebSerialLogger.println("Initializing MQTT client. broker: " + String(mqttbroker) + ":" + String(port) + " user: " + username);


     device_id = devicename;
     _user = username;
    _pass = password;
    _manufacturer = manufacturer;
    _model = model;

    _wifiClientmqtt = new WiFiClient();

    _mqttClient = new PubSubClient(mqttbroker, port, *_wifiClientmqtt);
    _mqttClient->setBufferSize(4096);

    if(callback != NULL)
        _mqttClient->setCallback(callback);
    else
        _mqttClient->setCallback(default_callback);

    Tasks = new std::list<MQTTMessages *>();
   
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

    if(Tasks == nullptr)
    {
        Serial.println("MQTT Client has not been set up!");
        return;
    }


    if(!_active && WiFi.status() == WL_CONNECTED && now - _lastConnectAttempt > 5000UL)
    {
        Connect();
    }

    if(now - _lastMqTTLoop > 1000UL)
    {
        _lastMqTTLoop = now;
        _mqttClient->loop();
    }

    if(!Tasks->empty() && isActive())
    {
      
        MQTTMessages *bt = Tasks->front();
        
        bool ok = SendPayload(bt->payload, bt->topic, bt->Retain);

        Tasks->remove(bt);
        if(!ok)
        {
            WebSerialLogger.println("unable to publish mqtt message ...");
            Tasks->push_back(bt);
            delay(1000);
        }
        else
            delete bt;

    }
}

bool MQTTConnectorClass::Connect()
{
    WebSerialLogger.println("Connecting MQTT client...");

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
        WebSerialLogger.println("Successfully connected to MQTT broker!");
        _mqttClient->subscribe("motd");
        _active = true;
    }

    return _active;
}

bool MQTTConnectorClass::SetupSensor(String topic, String component, String deviceclass, String unit, String icon)
{
    
    if(!_active)
        return false;

    //
    String header = "homeassistant/sensor/" + device_id + "_" + component ;
    
    String config_topic = header+ "_" + topic + "/config";
	String name = device_id + "_" + component + "_" + topic;

    WebSerialLogger.println("Configuring switch " + config_topic);

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    if(unit != "")
        root["unit_of_measurement"] = unit;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["unique_id"] = name;
    root["state_topic"] = header + "/state";

    root["entity_category"] = "diagnostic";
    
    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["manufacturer"] = _manufacturer;
    devobj["model"] = _model;

    PublishMessage(root, component, true, config_topic, SENSOR);
   
    return true;
}

bool MQTTConnectorClass::SetupSwitch(String topic, String component, String deviceclass, String icon)
{
    
    if(!_active)
        return false;

    //
    String header = "homeassistant/switch/" + device_id + "_" + component ;

    String config_topic = header+ "_" + topic + "/config";
	String name = device_id + "_" + component + "_" + topic;

    WebSerialLogger.println("Configuring switch " + config_topic);

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["unique_id"] = name;
    root["state_topic"] = header + "/state";

    root["command_topic"] = header + "/" + topic;
    _mqttClient->subscribe(String(header + "/"+ topic).c_str());
    

    root["entity_category"] = "diagnostic";
    
    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["manufacturer"] = _manufacturer;
    devobj["model"] = _model;

    PublishMessage(root, component, true, config_topic, SWITCH);
   
    return true;
}

bool MQTTConnectorClass::SetupSelect(String topic, String component, String deviceclass, String icon, std::vector<String> options)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/select/" + device_id + "_" + component ;
    
    String config_topic = header+ "_" + topic + "/config";
	String name = device_id + "_" + component + "_" + topic;

    WebSerialLogger.println("Configuring select " + config_topic);

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["unique_id"] = name;
    root["state_topic"] = header + "/state";

    root["command_topic"] = header + "/" + topic;
    _mqttClient->subscribe(String(header + "/" + topic).c_str());
    

    root["entity_category"] = "diagnostic";

    JsonArray optionobj = root["options"].to<JsonArray>();
    for(int i=0;i<options.size();i++)
    {
        optionobj.add(options[i]);
    }

    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["manufacturer"] = _manufacturer;
    devobj["model"] = _model;

    PublishMessage(root, component, true, config_topic, SELECT);
   
    return true;
}

bool MQTTConnectorClass::SetupButton(String topic, String component, String deviceclass, String icon)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/button/" + device_id + "_" + component ;
    
    String config_topic = header + "_" + topic + "/config";
	String name = device_id + "_" + component + "_" + topic;

    WebSerialLogger.println("Configuring button " + config_topic);

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["unique_id"] = name;
    root["state_topic"] = header + "/state";

    root["command_topic"] = header + "/" + topic;
    _mqttClient->subscribe(String(header + "/" + topic).c_str());
    

    root["entity_category"] = "diagnostic";

    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["manufacturer"] = _manufacturer;
    devobj["model"] = _model;

    PublishMessage(root, component, true, config_topic, BUTTON);
   
    return true;
}

bool MQTTConnectorClass::SetupText(String topic, String component, String deviceclass, String icon)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/text/" + device_id + "_" + component ;
    String config_topic = header + "_" + topic + "/config";
	String name = device_id + "_" + component + "_" + topic;

    WebSerialLogger.println("Configuring text " + config_topic);

    JsonDocument root;

    if(deviceclass != "")
        root["device_class"] = deviceclass;
    
    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["unique_id"] = name;
    root["state_topic"] = header + "/state";

    root["command_topic"] = header + "/" + topic;
    _mqttClient->subscribe(String(header + "/" + topic).c_str());
    

    root["entity_category"] = "diagnostic";

    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["manufacturer"] = _manufacturer;
    devobj["model"] = _model;

    PublishMessage(root, component, true, config_topic, TEXT);
   
    return true;
}

bool MQTTConnectorClass::SendPayload(String payload, String topic, bool retain)
{
    
    if(!_active)
        return false;

    //WebSerialLogger.println("Sending mqtt message: " + payload + " to " + topic);

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

void MQTTConnectorClass::PublishMessage(JsonDocument root, String component, bool retain, String topic, MQTTClassType sensor)
{
    
    if(root == NULL)
        return;

    String typ = "sensor";
    if(sensor == SWITCH)
        typ = "switch";
    else if(sensor == SELECT)
        typ = "select";
    else if(sensor == BUTTON)
        typ = "button";

    String header = "homeassistant/" + typ + "/" + device_id + "_" + component;

    String msg;
    size_t size = serializeJson(root, msg);
    
    //WebSerialLogger.println("Sending mqtt message: " + String(msg) + " Topic: " + topic);
    if(size > 1024)
        WebSerialLogger.println("Size : " + String(size));

    if(topic == "")
        topic = header + "/state";

    MQTTMessages *bt = new MQTTMessages();
    bt->payload = msg;
    bt->topic = topic;
    bt->Retain = retain;

    //SendPayload(msg, component, retain);

    Tasks->push_back(bt);
   
    _mqttClient->loop();
}


MQTTConnectorClass MQTTConnector;