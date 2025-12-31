#include "mqtt.hpp"
#include "../pmCommonLib.hpp"

MQTTConnectorClass::~MQTTConnectorClass()
{
    // Clean up dynamically allocated WiFi client
    if(_wifiClientmqtt != nullptr)
    {
        delete _wifiClientmqtt;
        _wifiClientmqtt = nullptr;
    }
    
    // Clean up MQTT client
    if(_mqttClient != nullptr)
    {
        delete _mqttClient;
        _mqttClient = nullptr;
    }
    
    // Clean up task list and all pending messages
    if(Tasks != nullptr)
    {
        // Delete all pending messages
        for(auto it = Tasks->begin(); it != Tasks->end(); ++it)
        {
            delete *it;
        }
        Tasks->clear();
        delete Tasks;
        Tasks = nullptr;
    }
}

String MQTTConnectorClass::mqtt_config_page(AsyncWebServerRequest *request) {
    Serial.println("Webserver handle request ... ");
  
    
    String html = "<form action='/config/mqtt.html' method='POST'>\
                    <p>\
                        <label for='broker'>Broker</label>\
                        <input type='text' id ='broker' name='broker' value='" + _mqttcredentials.Broker + "'><br>\
                        <label for='port'>Port</label>\
                        <input type='text' id ='port' name='port' value='" + _mqttcredentials.Port + "'><br>\
                        <label for='user'>User</label>\
                        <input type='text' id ='user' name='user' value='" + _mqttcredentials.User + "'><br>\
                        <label for='pass'>Password</label>\
                        <input type='text' id ='pass' name='pass' value='" + _mqttcredentials.Pass + "'><br>\
                        <label for='devicename'>Device name</label>\
                        <input type='text' id ='devicename' name='devicename' value='" + _mqttcredentials.DeviceName + "'><br>\
                        <label for='manufacturer'>Manufacturer</label>\
                        <input type='text' id ='manufacturer' name='manufacturer' value='" + _mqttcredentials.ManuFacturer + "'><br>\
                        <label for='model'>Model</label>\
                        <input type='text' id ='model' name='model' value='" + _mqttcredentials.Model + "'><br>\
                        <input type ='submit' value ='Submit'>\
                    </p>\
                    </form>";
  
    return html;
  }

  String MQTTConnectorClass::mqtt_config_page_POST(AsyncWebServerRequest *request)
  {
      Serial.println("Handling POST request ...");
  
      int params = request->params();
      for(int i=0;i<params;i++)
      {
          const AsyncWebParameter* p = request->getParam(i);
          if(p->isPost())
          {
            
            if (p->name() == "broker") {
                _mqttcredentials.Broker = p->value();
                Serial.print("Broker set to: ");
                Serial.println(_mqttcredentials.Broker);
            }
            
            if (p->name() == "port") {
                _mqttcredentials.Port = p->value();
                Serial.print("Port set to: ");
                Serial.println(_mqttcredentials.Port);
            }
            
            if (p->name() == "user") {
                _mqttcredentials.User = p->value();
                Serial.print("User set to: ");
                Serial.println(_mqttcredentials.User);
            }
            
            if (p->name() == "pass") {
                _mqttcredentials.Pass = p->value();
                Serial.print("Password set to: ");
                Serial.println(_mqttcredentials.Pass);        
            }

            if (p->name() == "devicename") {
                _mqttcredentials.DeviceName = p->value();
                Serial.print("DeviceName set to: ");
                Serial.println(_mqttcredentials.DeviceName);        
            }

            if (p->name() == "manufacturer") {
                _mqttcredentials.ManuFacturer = p->value();
                Serial.print("ManuFacturer set to: ");
                Serial.println(_mqttcredentials.ManuFacturer);        
            }

            if (p->name() == "model") {
                _mqttcredentials.Model = p->value();
                Serial.print("Model set to: ");
                Serial.println(_mqttcredentials.Model);        
            }
          }
      }

      _mqttcredentials.changed = true;
  
      JsonDocument data;
      data["Broker"] = _mqttcredentials.Broker;
      data["Port"] = _mqttcredentials.Port;
      data["User"] = _mqttcredentials.User;
      data["Pass"] = _mqttcredentials.Pass;
      data["DeviceName"] = _mqttcredentials.DeviceName;
      data["ManuFacturer"] = _mqttcredentials.ManuFacturer;
      data["Model"] = _mqttcredentials.Model;
      data["AllowChanges"] = true;
      pmCommonLib.ConfigHandler.SaveConfigFile(MQTTconfigFilePath, data);
  
      
      return mqtt_config_page(request);
  }

void MQTTConnectorClass::default_callback(char* topic, uint8_t* payload, unsigned int length) {
    
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }
    pmLogging.LogLn(msg);
}

void MQTTConnectorClass::Setup(std::function<void(char*, uint8_t*, unsigned int)> callback)
{
    JsonDocument doc = pmCommonLib.ConfigHandler.LoadConfigFile(MQTTconfigFilePath);
    
    if(doc["Broker"].is<String>())
    {
        pmLogging.LogLn("Reading MQTT config-file!");
        _mqttcredentials.Broker = doc["Broker"].as<String>();
        _mqttcredentials.Port = doc["Port"].as<String>();
        _mqttcredentials.User = doc["User"].as<String>();
        _mqttcredentials.Pass = doc["Pass"].as<String>();
        _mqttcredentials.DeviceName = doc["DeviceName"].as<String>();
        _mqttcredentials.ManuFacturer = doc["ManuFacturer"].as<String>();
        _mqttcredentials.Model = doc["Model"].as<String>();

        pmLogging.LogLn("Broker: " + _mqttcredentials.Broker);
        pmLogging.LogLn("Port: " + _mqttcredentials.Port);
    }
    else
    {
        _mqttcredentials.Broker = "dummy";
        _mqttcredentials.Port = "1883";
        _mqttcredentials.User = "";
        _mqttcredentials.Pass = "";
        _mqttcredentials.DeviceName = WiFi.getHostname();
        _mqttcredentials.ManuFacturer = "Patrick Mortara";
        _mqttcredentials.Model = "v1";
        
    }

    _mqttcredentials.callback = callback;

    _wifiClientmqtt = new WiFiClient();
    _mqttClient = new PubSubClient(_mqttcredentials.Broker.c_str(), (uint16_t)_mqttcredentials.Port.toInt(), *_wifiClientmqtt);
    _mqttClient->setBufferSize(4096);

    if(callback != NULL)
        _mqttClient->setCallback(_mqttcredentials.callback);
    else
    {
        MQTTCallBackFunction f1 = std::bind(&MQTTConnectorClass::default_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        _mqttClient->setCallback(f1);
    }

    Tasks = new std::list<MQTTMessages *>();
    _lastConnectAttempt = millis();
    _setup = true;
}

void MQTTConnectorClass::Begin()
{
    if(pmCommonLib.WebServer.IsSetup())
    {
        ConfigHTTPRegisterFunction f1 = std::bind(&MQTTConnectorClass::mqtt_config_page, this, std::placeholders::_1);
        ConfigHTTPRegisterFunction f2 = std::bind(&MQTTConnectorClass::mqtt_config_page_POST, this, std::placeholders::_1);
        pmCommonLib.ConfigHandler.RegisterConfigPage("mqtt", f1, f2);
    }
}

void MQTTConnectorClass::ConfigureCallback(std::function<void(char*, uint8_t*, unsigned int)> callback)
{
    _mqttcredentials.callback = callback;
    _mqttClient->setCallback(_mqttcredentials.callback);
}

bool MQTTConnectorClass::IsSetup()
{
    return _setup;
}

void MQTTConnectorClass::saveConfig()
{
    JsonDocument data;
    data["Broker"] = _mqttcredentials.Broker;
    data["Port"] = _mqttcredentials.Port;
    data["User"] = _mqttcredentials.User;
    data["Pass"] = _mqttcredentials.Pass;
    data["DeviceName"] = _mqttcredentials.DeviceName;
    data["ManuFacturer"] = _mqttcredentials.ManuFacturer;
    data["Model"] = _mqttcredentials.Model;
    data["AllowChanges"] = _mqttcredentials.AllowChanges;
    pmCommonLib.ConfigHandler.SaveConfigFile(MQTTconfigFilePath, data);
}

void MQTTConnectorClass::ConfigureDevice(String devicename, String manufacturer, String model)
{
    _mqttcredentials.DeviceName = devicename;
    _mqttcredentials.ManuFacturer = manufacturer;
    _mqttcredentials.Model = model;

    saveConfig();
}

void MQTTConnectorClass::Configure(String broker, int port, String user, String password, String devicename, String manufacturer, String model, bool allowchanges)
{
    _mqttcredentials.Broker = broker;
    _mqttcredentials.Port = String(port);
    _mqttcredentials.User = user;
    _mqttcredentials.Pass = password;
    _mqttcredentials.DeviceName = devicename;
    _mqttcredentials.ManuFacturer = manufacturer;
    _mqttcredentials.Model = model;
    _mqttcredentials.AllowChanges = allowchanges;

    saveConfig();
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
        pmLogging.LogLn("MQTT Client has not been set up!");
        return;
    }

    if(_mqttcredentials.changed)
    {
        pmLogging.LogLn("MQTT credentials changed. reconnecting ....");
        _mqttClient->setServer(_mqttcredentials.Broker.c_str(), (uint16_t)_mqttcredentials.Port.toInt());
        _active = false;
        _mqttcredentials.changed = false;
    }

    if(!_active && WiFi.status() == WL_CONNECTED && now - _lastConnectAttempt > 5000UL)
    {
        pmLogging.LogLn("MQTT connecting!");
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
        Tasks->pop_front();
        
        bool ok = SendPayload(bt->payload, bt->topic, bt->Retain);

        if(!ok)
        {
            pmLogging.LogLn("unable to publish mqtt message ...");
            Tasks->push_back(bt);
            delay(1000);
        }
        else
        {
            delete bt;
        }

    }
}

bool MQTTConnectorClass::Connect()
{
    _lastConnectAttempt = millis();

    if(_mqttcredentials.Broker == "dummy")
        return false;

    pmLogging.LogLn("Connecting MQTT client...");

    if(_wifiClientmqtt == NULL)
    {
        _wifiClientmqtt = new WiFiClient();
        _mqttClient->setClient(*_wifiClientmqtt);
    }

    
    if(!_mqttClient->connect(_mqttcredentials.DeviceName.c_str(), _mqttcredentials.User.c_str(), _mqttcredentials.Pass.c_str()))
    {
        pmLogging.LogLn("Could not connect to MQTT broker!");
        pmLogging.LogLn("State: " + String(_mqttClient->state()));
    }
    else
    {
        pmLogging.LogLn("Successfully connected to MQTT broker!");
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
    String header = "homeassistant/sensor/" + _mqttcredentials.DeviceName + "_" + component ;
    
    String config_topic = header+ "_" + topic + "/config";
	String name = _mqttcredentials.DeviceName + "_" + component + "_" + topic;

    pmLogging.LogLn("Configuring sensor " + config_topic);

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
    deviceids.add(_mqttcredentials.DeviceName);

    devobj["name"] = _mqttcredentials.DeviceName;
    devobj["manufacturer"] = _mqttcredentials.ManuFacturer;
    devobj["model"] = _mqttcredentials.Model;

    PublishMessage(root, component, true, config_topic, SENSOR);
   
    return true;
}

bool MQTTConnectorClass::SetupSwitch(String topic, String component, String deviceclass, String icon)
{
    
    if(!_active)
        return false;

    //
    String header = "homeassistant/switch/" + _mqttcredentials.DeviceName + "_" + component ;

    String config_topic = header+ "_" + topic + "/config";
	String name = _mqttcredentials.DeviceName + "_" + component + "_" + topic;

    pmLogging.LogLn("Configuring switch " + config_topic);

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
    deviceids.add(_mqttcredentials.DeviceName);

    devobj["name"] = _mqttcredentials.DeviceName;
    devobj["manufacturer"] = _mqttcredentials.ManuFacturer;
    devobj["model"] = _mqttcredentials.Model;

    PublishMessage(root, component, true, config_topic, SWITCH);
   
    return true;
}

bool MQTTConnectorClass::SetupSelect(String topic, String component, String deviceclass, String icon, std::vector<String> options)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/select/" + _mqttcredentials.DeviceName + "_" + component ;
    
    String config_topic = header+ "_" + topic + "/config";
	String name = _mqttcredentials.DeviceName + "_" + component + "_" + topic;

    pmLogging.LogLn("Configuring select " + config_topic);

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
    for(std::size_t i=0;i<options.size();i++)
    {
        optionobj.add(options[i]);
    }

    JsonObject devobj = root["dev"].to<JsonObject>();
    JsonArray deviceids = devobj["ids"].to<JsonArray>();
    deviceids.add(_mqttcredentials.DeviceName);

    devobj["name"] = _mqttcredentials.DeviceName;
    devobj["manufacturer"] = _mqttcredentials.ManuFacturer;
    devobj["model"] = _mqttcredentials.Model;

    PublishMessage(root, component, true, config_topic, SELECT);
   
    return true;
}

bool MQTTConnectorClass::SetupButton(String topic, String component, String deviceclass, String icon)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/button/" + _mqttcredentials.DeviceName + "_" + component ;
    
    String config_topic = header + "_" + topic + "/config";
	String name = _mqttcredentials.DeviceName + "_" + component + "_" + topic;

    pmLogging.LogLn("Configuring button " + config_topic);

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
    deviceids.add(_mqttcredentials.DeviceName);

    devobj["name"] = _mqttcredentials.DeviceName;
    devobj["manufacturer"] = _mqttcredentials.ManuFacturer;
    devobj["model"] = _mqttcredentials.Model;

    PublishMessage(root, component, true, config_topic, BUTTON);
   
    return true;
}

bool MQTTConnectorClass::SetupText(String topic, String component, String deviceclass, String icon)
{
    if(!_active)
        return false;

    //
    String header = "homeassistant/text/" + _mqttcredentials.DeviceName + "_" + component ;
    String config_topic = header + "_" + topic + "/config";
	String name = _mqttcredentials.DeviceName + "_" + component + "_" + topic;

    pmLogging.LogLn("Configuring text " + config_topic);

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
    deviceids.add(_mqttcredentials.DeviceName);

    devobj["name"] = _mqttcredentials.DeviceName;
    devobj["manufacturer"] = _mqttcredentials.ManuFacturer;
    devobj["model"] = _mqttcredentials.Model;

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
        pmLogging.LogLn("Error publishing data!");
        pmLogging.LogLn("State: " + String(_mqttClient->state()));

        if(_mqttClient->state() == -3)
        {
            pmLogging.LogLn("MQTT connection lost ...");
            _mqttClient->disconnect();
            _active = false;
        }
        return false;
    }

    return true;
}

void MQTTConnectorClass::PublishMessage(JsonDocument root, String component, bool retain, String topic, MQTTClassType sensor)
{
    
    if(root.isNull())
        return;

    String typ = "sensor";
    if(sensor == SWITCH)
        typ = "switch";
    else if(sensor == SELECT)
        typ = "select";
    else if(sensor == BUTTON)
        typ = "button";

    String header = "homeassistant/" + typ + "/" + _mqttcredentials.DeviceName + "_" + component;

    String msg;
    size_t size = serializeJson(root, msg);
    
    //WebSerialLogger.println("Sending mqtt message: " + String(msg) + " Topic: " + topic);
    if(size > 1024)
        pmLogging.LogLn("Size : " + String(size));

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
