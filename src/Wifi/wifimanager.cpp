#include "wifimanager.hpp"
#include "../pmCommonLib.hpp"

String WIFIManagerClass::handleWifManagerRoot(AsyncWebServerRequest *request) {
    Serial.println("Webserver handle request ... ");

    String o1 = "";
    String o2 = "";

    if(_wificredentials.ConfigMode == "static")
      o2 = "selected";
    else
      o1 = "selected";

    String html = "<form action='/config/mqtt.html' method='POST'>\
                    <p>\
                      <label for='ssid'>SSID</label>\
                      <input type='text' id ='ssid' name='ssid' value='" + _wificredentials.SSID + "'><br>\
                      <label for='pass'>Password</label>\
                      <input type='text' id ='pass' name='pass' value='" + _wificredentials.PASS + "'><br>\
                      <label for='ipmode'>Network configuration</label>\
                      <select id='ipmode' name='ipmode'>\
                        <option value='dhcp' " + o1 + ">DHCP</option>\
                        <option value='static' " + o2 + ">Static</option>\
                      </select><br>\
                      <label for='ip'>IP Address</label>\
                      <input type='text' id ='ip' name='ip' value='" + _wificredentials.IP + "'><br>\
                      <label for='subnet'>Subnet</label>\
                      <input type='text' id ='subnet' name='subnet' value='" + _wificredentials.Subnet + "'><br>\
                      <label for='gateway'>Gateway Address</label>\
                      <input type='text' id ='gateway' name='gateway' value='" + _wificredentials.Gateway + "'><br>\
                      <input type ='submit' value ='Submit'>\
                    </p>\
                  </form>";

    return html;
}

String WIFIManagerClass::handlePOSTrequest(AsyncWebServerRequest *request)
{
    Serial.println("Handling POST request ...");

    int params = request->params();
    for(int i=0;i<params;i++)
    {
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost())
        {
          // HTTP POST ssid value
          if (p->name() == "ssid") {
            _wificredentials.SSID = p->value();
              Serial.print("SSID set to: ");
              Serial.println(_wificredentials.SSID);
          }
          // HTTP POST pass value
          if (p->name() == "pass") {
            _wificredentials.PASS = p->value();
              Serial.print("Password set to: ");
              Serial.println(_wificredentials.PASS);
          }
          // HTTP POST ip value
          if (p->name() == "ip") {
            _wificredentials.IP = p->value();
              Serial.print("IP Address set to: ");
              Serial.println(_wificredentials.IP);
          }
          // HTTP POST gateway value
          if (p->name() == "gateway") {
            _wificredentials.Gateway = p->value();
              Serial.print("Gateway set to: ");
              Serial.println(_wificredentials.Gateway);        
          }
          // HTTP POST configmode value
          if (p->name() == "ipmode") {
            _wificredentials.ConfigMode = p->value();
            Serial.print("ConfigMode set to: ");
            Serial.println(_wificredentials.ConfigMode);        
          }

          if (p->name() == "subnet") {
            _wificredentials.Subnet = p->value();
            Serial.print("Subnet set to: ");
            Serial.println(_wificredentials.Subnet);        
          }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }

    JsonDocument data;
    data["SSID"] = _wificredentials.SSID;
    data["PASS"] = _wificredentials.PASS;
    data["IP"] = _wificredentials.IP;
    data["Subnet"] = _wificredentials.Subnet;
    data["Gateway"] = _wificredentials.Gateway;
    data["ConfigMode"] = _wificredentials.ConfigMode;
    
    pmCommonLib.ConfigHandler.SaveConfigFile(WIFIconfigFilePath, data);

    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + _wificredentials.IP);
    delay(3000);
    ESP.restart();

    return "";
}

bool WIFIManagerClass::initWiFi() {
    if(_wificredentials.SSID == ""){
      Serial.println("Undefined SSID");
      return false;
    }

    WiFi.mode(WIFI_STA);

    if(_wificredentials.ConfigMode == "static")
    {
        IPAddress localIP;
        IPAddress localGateway;
        IPAddress subnet;

        localIP.fromString(_wificredentials.IP.c_str());
        localGateway.fromString(_wificredentials.Gateway.c_str());

        if(_wificredentials.Subnet != "")
          subnet.fromString(_wificredentials.Subnet.c_str());
        else
          subnet.fromString("255.255.255.0");

        if (!WiFi.config(localIP, localGateway, subnet)){
          Serial.println("STA Failed to configure with static IP");
          return false;
        }
    }
    else
    {
      Serial.println("DHCP enabled");
    }

    WiFi.begin(_wificredentials.SSID.c_str(), _wificredentials.PASS.c_str());
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;

    while(WiFi.status() != WL_CONNECTED) {
      currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        Serial.println("Failed to connect.");
        return false;
      }
    }

    if(_wificredentials.ConfigMode == "dhcp")
    {
      _wificredentials.IP = WiFi.localIP().toString();
      _wificredentials.Subnet = WiFi.subnetMask().toString();
      _wificredentials.Gateway = WiFi.gatewayIP().toString();
    }

    Serial.println(WiFi.localIP());
    return true;
}



void WIFIManagerClass::Setup(String hostname)
{
    Serial.println("Setting up WIFI manager");

    JsonDocument doc = pmCommonLib.ConfigHandler.LoadConfigFile(WIFIconfigFilePath);
    
    if(doc["SSID"].is<String>())
    {
        Serial.println("Reading config-file!");
        _wificredentials.ConfigMode = String(doc["ConfigMode"].as<String>());
        _wificredentials.SSID = String(doc["SSID"].as<String>());
        _wificredentials.PASS = String(doc["PASS"].as<String>());
        _wificredentials.IP = String(doc["IP"].as<String>());
        _wificredentials.Gateway = String(doc["Gateway"].as<String>());
        _wificredentials.Subnet = String(doc["Subnet"].as<String>());

        Serial.println("SSID: " + _wificredentials.SSID);
        Serial.println("PASS: " + _wificredentials.PASS);
    }

    if(!initWiFi())  
    {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
       
        WiFi.softAP("ESP-WIFI-MANAGER", NULL, 7);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 

        captiveportalactive = true;
    }
    
}

void WIFIManagerClass::Begin()
{
  ConfigHTTPRegisterFunction f1 = std::bind(&WIFIManagerClass::handleWifManagerRoot, this, std::placeholders::_1);
  ConfigHTTPRegisterFunction f2 = std::bind(&WIFIManagerClass::handlePOSTrequest, this, std::placeholders::_1);

  if(captiveportalactive)
  {
    pmCommonLib.WebServer.RegisterOn("/", f1);
    pmCommonLib.WebServer.RegisterOn("/", f2, HTTP_POST);
  }
  else
  {
    pmCommonLib.ConfigHandler.RegisterConfigPage("wifi", f1, f2);
  }
}

bool WIFIManagerClass::Connect()
{
   
    return true;
}

void WIFIManagerClass::setupMQTT()
{
    if(mqttsetup)
        return;

    pmCommonLib.WebSerial.println("Setting up Wifi MQTT client");
    
    if(!pmCommonLib.MQTTConnector.SetupSensor("SSID", "WIFI", "", "", ""))
    {
        pmCommonLib.WebSerial.println("Unable to setup WIFI MQTT client");            
        return;
    }

    pmCommonLib.MQTTConnector.SetupSensor("BSSID", "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("WIFI_RSSI", "WIFI", "signal_strength", "dB", "mdi:sine-wave");
    pmCommonLib.MQTTConnector.SetupSensor("Hostname",  "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("IP", "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("SubnetMask","WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("Gateway","WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("DNS", "WIFI", "", "", "");

    pmCommonLib.WebSerial.println("WIfi mqtt setup done!");

    mqttsetup = true;
}

void WIFIManagerClass::Disconnect()
{
    pmCommonLib.WebSerial.println("disonnecting from WiFi ..");
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
  if(WiFi.getMode() == WIFI_AP)
  {
    pmCommonLib.WebSerial.println("[*] SoftAp is running ");
    pmCommonLib.WebSerial.print("[+] ESP32 Hostname : ");
    pmCommonLib.WebSerial.println(WiFi.getHostname());
    pmCommonLib.WebSerial.print("[+] ESP32 IP : ");
    pmCommonLib.WebSerial.println(WiFi.softAPIP().toString());
  }
  else
  {
  
    pmCommonLib.WebSerial.print("[*] Network information for ");
    pmCommonLib.WebSerial.println(WiFi.SSID());

    pmCommonLib.WebSerial.println("[+] BSSID : " + WiFi.BSSIDstr());
    pmCommonLib.WebSerial.print("[+] Gateway IP : ");
    pmCommonLib.WebSerial.println(WiFi.gatewayIP().toString());
    pmCommonLib.WebSerial.print("[+] DNS IP : ");
    pmCommonLib.WebSerial.println(WiFi.dnsIP().toString());   
    pmCommonLib.WebSerial.println((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    pmCommonLib.WebSerial.print("[+] ESP32 IP : ");
    pmCommonLib.WebSerial.println(WiFi.localIP().toString());
    pmCommonLib.WebSerial.print("[+] Subnet Mask : ");
    pmCommonLib.WebSerial.println(WiFi.subnetMask().toString());
  } 
}

bool WIFIManagerClass::IsConnected()
{
    return connected;
}

unsigned long WIFIManagerClass::LastConnectionTry()
{
    return _lastConnectionTry;
}

void WIFIManagerClass::Loop()
{
    unsigned long currentMillis = millis();

    connected = WiFi.isConnected();

    if(connecting && connected)
    {
        pmCommonLib.WebSerial.println("WiFi connected!");
        connecting = false;
        DisplayInfo();
    }

    if(currentMillis - _lastMqttupdate > 30000 && connected && pmCommonLib.MQTTConnector.isActive())
    {
        if(!mqttsetup)
        {
            setupMQTT();
        }
        else
        {
            JsonDocument payload;
            payload["SSID"] = WiFi.SSID();
            payload["BSSID"] = WiFi.BSSIDstr();
            payload["WIFI_RSSI"] = String(WiFi.RSSI());
            payload["Hostname"] = String(WiFi.getHostname());
            payload["IP"] = WiFi.localIP().toString();
            payload["SubnetMask"] = WiFi.subnetMask().toString();
            payload["Gateway"] = WiFi.gatewayIP().toString();
            payload["DNS"] = WiFi.dnsIP().toString();;

            pmCommonLib.MQTTConnector.PublishMessage(payload, "WIFI");
            _lastMqttupdate = currentMillis;
        }
    }

    // if WiFi is down, try reconnecting
    /*if (!connecting && !connected && (currentMillis - _lastConnectionTry >= interval)) 
    {
        WebSerialLogger.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        connecting = true;
        _lastConnectionTry = currentMillis;
    }*/
}

WIFICreds _wificredentials;
