#include "wifimanager.hpp"
#include "../pmCommonLib.hpp"

void handleWifManagerRoot(AsyncWebServerRequest *request) {
  Serial.println("Webserver handle request ... " + request->url());

  String o1 = "";
  String o2 = "";

  if(_credentials.ConfigMode == "static")
    o2 = "selected";
  else
    o1 = "selected";

  String html = "<!DOCTYPE html>\
                <html>\
                <head>\
                  <title>ESP Wi-Fi Manager</title>\
                  <meta name='viewport' content='width=device-width, initial-scale=1'>\
                  <link rel='icon' href='data:,'>\
                  <style>\
                        html {\
                          font-family: Arial, Helvetica, sans-serif; \
                          display: inline-block; \
                          text-align: center;\
                        }\
                        h1 {\
                          font-size: 1.8rem; \
                          color: white;\
                        }\
                        p {\
                          font-size: 1.4rem;\
                        }\
                        .topnav { \
                          overflow: hidden; \
                          background-color: #0A1128;\
                        }\
                        body {\
                          margin: 0;\
                        }\
                        .content {\
                          padding: 5%;\
                        }\
                        .card-grid {\
                          max-width: 800px;\
                          margin: 0 auto; \
                          display: grid; \
                          grid-gap: 2rem; \
                          grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));\
                        }\
                        .card {\
                          background-color: white; \
                          box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);\
                        }\
                        .card-title { \
                          font-size: 1.2rem;\
                          font-weight: bold;\
                          color: #034078\
                        }\
                        input[type=submit] {\
                          border: none;\
                          color: #FEFCFB;\
                          background-color: #034078;\
                          padding: 15px 15px;\
                          text-align: center;\
                          text-decoration: none;\
                          display: inline-block;\
                          font-size: 16px;\
                          width: 100px;\
                          margin-right: 10px;\
                          border-radius: 4px;\
                          transition-duration: 0.4s;\
                          }\
                        input[type=submit]:hover {\
                          background-color: #1282A2;\
                        }\
                        input[type=text], input[type=number], select {\
                          width: 50%;\
                          padding: 12px 20px;\
                          margin: 18px;\
                          display: inline-block;\
                          border: 1px solid #ccc;\
                          border-radius: 4px;\
                          box-sizing: border-box;\
                        }\
                        label {\
                          font-size: 1.2rem;\
                        }\
                        .value{\
                          font-size: 1.2rem;\
                          color: #1282A2;  \
                        }\
                        .state {\
                          font-size: 1.2rem;\
                          color: #1282A2;\
                        }\
                        button {\
                          border: none;\
                          color: #FEFCFB;\
                          padding: 15px 32px;\
                          text-align: center;\
                          font-size: 16px;\
                          width: 100px;\
                          border-radius: 4px;\
                          transition-duration: 0.4s;\
                        }\
                        .button-on {\
                          background-color: #034078;\
                        }\
                        .button-on:hover {\
                          background-color: #1282A2;\
                        }\
                        .button-off {\
                          background-color: #858585;\
                        }\
                        .button-off:hover {\
                          background-color: #252524;\
                        }\
                  </style>\
                </head>\
                <body>\
                  <div class='topnav'>\
                    <h1>ESP Wi-Fi Manager</h1>\
                  </div>\
                  <div class='content'>\
                    <div class='card-grid'>\
                      <div class='card'>\
                        <form action='/' method='POST'>\
                          <p>\
                            <label for='ssid'>SSID</label>\
                            <input type='text' id ='ssid' name='ssid' value='" + _credentials.SSID + "'><br>\
                            <label for='pass'>Password</label>\
                            <input type='text' id ='pass' name='pass' value='" + _credentials.PASS + "'><br>\
                            <label for='ipmode'>Network configuration</label>\
                            <select id='ipmode' name='ipmode'>\
                              <option value='dhcp' " + o1 + ">DHCP</option>\
                              <option value='static' " + o2 + ">Static</option>\
                            </select><br>\
\
                            <label for='ip'>IP Address</label>\
                            <input type='text' id ='ip' name='ip' value='" + _credentials.IP + "'><br>\
\
                            <label for='gateway'>Gateway Address</label>\
                            <input type='text' id ='gateway' name='gateway' value='" + _credentials.Gateway + "'><br>\
                \
                            <input type ='submit' value ='Submit'>\
                          </p>\
                        </form>\
                      </div>\
                    </div>\
                  </div>\
                </body>\
                </html>";

  request->send(200, "text/html", html);

}


bool WIFIManagerClass::initWiFi() {
    if(_credentials.SSID =="" || _credentials.IP==""){
      Serial.println("Undefined SSID or IP address.");
      return false;
    }

    WiFi.mode(WIFI_STA);

    if(_credentials.ConfigMode == "static")
    {
        IPAddress localIP;
        IPAddress localGateway;

        localIP.fromString(_credentials.IP.c_str());
        localGateway.fromString(_credentials.Gateway.c_str());
        IPAddress subnet(255, 255, 255, 0);
        if (!WiFi.config(localIP, localGateway, subnet)){
          Serial.println("STA Failed to configure with static IP");
          return false;
        }
    }
    else
    {
      Serial.println("DHCP enabled");
    }

    WiFi.begin(_credentials.SSID.c_str(), _credentials.PASS.c_str());
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

    Serial.println(WiFi.localIP());
    return true;
}

void handlePOSTrequest(AsyncWebServerRequest *request)
{
    Serial.println("ServeWIFILogonPage ...");

    int params = request->params();
    for(int i=0;i<params;i++)
    {
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost())
        {
          // HTTP POST ssid value
          if (p->name() == "ssid") {
              _credentials.SSID = p->value().c_str();
              Serial.print("SSID set to: ");
              Serial.println(_credentials.SSID);
          }
          // HTTP POST pass value
          if (p->name() == "pass") {
              _credentials.PASS = p->value().c_str();
              Serial.print("Password set to: ");
              Serial.println(_credentials.PASS);
          }
          // HTTP POST ip value
          if (p->name() == "ip") {
              _credentials.IP = p->value().c_str();
              Serial.print("IP Address set to: ");
              Serial.println(_credentials.IP);
          }
          // HTTP POST gateway value
          if (p->name() == "gateway") {
              _credentials.Gateway = p->value().c_str();
              Serial.print("Gateway set to: ");
              Serial.println(_credentials.Gateway);        
          }
          // HTTP POST gateway value
          if (p->name() == "configmode") {
            _credentials.ConfigMode = p->value().c_str();
            Serial.print("ConfigMode set to: ");
            Serial.println(_credentials.ConfigMode);        
          }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }

    JsonDocument data;
    data["SSID"] = _credentials.SSID;
    data["PASS"] = _credentials.PASS;
    data["IP"] = _credentials.IP;
    data["Gateway"] = _credentials.Gateway;
    data["ConfigMode"] = _credentials.ConfigMode;
    data["DNS"] = _credentials.DNS;
    ConfigHandler.SaveConfigFile(configFilePath, data);

    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + _credentials.IP);
    delay(3000);
    ESP.restart();
}

void WIFIManagerClass::Setup(String hostname)
{
    JsonDocument doc = ConfigHandler.LoadConfigFile(configFilePath);
    
    if(doc["SSID"].is<String>())
    {
        _credentials.ConfigMode = String(doc["ConfigMode"]);
        _credentials.SSID = String(doc["SSID"]);
        _credentials.PASS = String(doc["PASS"]);
        _credentials.IP = String(doc["IP"]);
        _credentials.Gateway = String(doc["Gateway"]);
        _credentials.DNS = String(doc["DNS"]);
    }

    if(!initWiFi())  
    {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
       
        WiFi.softAP("ESP-WIFI-MANAGER", NULL, 7);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 
    }
    
    pmCommonLib.ConfigHandler.RegisterConfigPage("WIFI", handleWifManagerRoot, handlePOSTrequest);
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

WIFICreds _credentials;
