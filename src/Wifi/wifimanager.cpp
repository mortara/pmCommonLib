#include "wifimanager.hpp"
#include "../MQTT/mqtt.hpp"
#include "../Webserver/webserver.hpp"



void handleWifManagerIndex(AsyncWebServerRequest *request) {
    
  Serial.println("Webserver handle request ... " + request->url());



  String SSID = _credentials.SSID;
  String PASS = _credentials.PASS;



  String html = "<!DOCTYPE html>\
                <html>\
                <head>\
                  <title>ESP Wi-Fi Manager</title>\
                  <meta name='viewport' content='width=device-width, initial-scale=1'>\
                  <link rel='icon' href='data:,'>\
                  <link rel='stylesheet' type='text/css' href='style.css'>\
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
                            <input type='text' id ='ssid' name='ssid' value='" + SSID + "'><br>\
                            <label for='pass'>Password</label>\
                            <input type='text' id ='pass' name='pass' value='" + PASS + "'><br>\
                            <label for='ipmode'>Network configuration</label>\
                            <select id='ipmode' name='ipmode'>\
                              <option value='dhcp'>DHCP</option>\
                              <option value='static'>Static</option>\
                            </select><br>\
\
                            <label for='ip'>IP Address</label>\
                            <input type='text' id ='ip' name='ip' value='192.168.1.200'><br>\
\
                            <label for='gateway'>Gateway Address</label>\
                            <input type='text' id ='gateway' name='gateway' value='192.168.1.1'><br>\
                            \
                            <label for='dns'>DSN Server</label>\
                            <input type='text' id ='dns' name='dns' value='192.168.1.1'><br>\
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

void handleWifManagerStyle(AsyncWebServerRequest *request) {
    
  Serial.println("Webserver style.css request ...");


  String html = "html {\
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
}";

  request->send(200, "text/css", html);

}

void handleWifManagerRoot(AsyncWebServerRequest *request) 
{
    String req = request->url();
    req.toLowerCase();

    Serial.println("handling request " + req);

    if(req.endsWith("style.css"))
    {
        handleWifManagerStyle(request);
        return;
    }

    handleWifManagerIndex(request);
}

  

  bool WIFIManagerClass::initWiFi() {
    if(_credentials.SSID =="" || _credentials.IP==""){
      Serial.println("Undefined SSID or IP address.");
      return false;
    }
  
    WiFi.mode(WIFI_STA);

    if(_credentials.ConfigMode == "static")
    {
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

  String WIFIManagerClass::processor(const String& var) {
    if(var == "STATE") {
     
    }
    return String();
  }

  

void handlePOSTrequest(AsyncWebServerRequest *request)
{
  Serial.println("ServeWIFILogonPage ...");

    int params = request->params();
    for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
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
    
    if(doc.containsKey("SSID"))
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
        // NULL sets an open Access Point
        
        WiFi.softAP("ESP-WIFI-MANAGER", NULL, 7);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 

        WebServer.RegisterOn("/", handleWifManagerRoot);
        WebServer.RegisterOn("/", handlePOSTrequest, HTTP_POST);

    }
    else
    {
        WebServer.RegisterOn("/config", handleWifManagerRoot);
        WebServer.RegisterOn("/config", handlePOSTrequest, HTTP_POST);

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

    WebSerialLogger.println("Setting up Wifi MQTT client");
    
    if(!MQTTConnector.SetupSensor("SSID", "WIFI", "", "", ""))
    {
        WebSerialLogger.println("Unable to setup WIFI MQTT client");            
        return;
    }

    MQTTConnector.SetupSensor("BSSID", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("WIFI_RSSI", "WIFI", "signal_strength", "dB", "mdi:sine-wave");
    MQTTConnector.SetupSensor("Hostname",  "WIFI", "", "", "");
    MQTTConnector.SetupSensor("IP", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("SubnetMask","WIFI", "", "", "");
    MQTTConnector.SetupSensor("Gateway","WIFI", "", "", "");
    MQTTConnector.SetupSensor("DNS", "WIFI", "", "", "");

    WebSerialLogger.println("WIfi mqtt setup done!");

    mqttsetup = true;
}

void WIFIManagerClass::Disconnect()
{
    WebSerialLogger.println("disonnecting from WiFi ..");
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
  if(WiFi.getMode() == WIFI_AP)
  {
    WebSerialLogger.println("[*] SoftAp is running ");
    WebSerialLogger.print("[+] ESP32 Hostname : ");
    WebSerialLogger.println(WiFi.getHostname());
    WebSerialLogger.print("[+] ESP32 IP : ");
    WebSerialLogger.println(WiFi.softAPIP().toString());
  }
  else
  {
  
    WebSerialLogger.print("[*] Network information for ");
    WebSerialLogger.println(WiFi.SSID());

    WebSerialLogger.println("[+] BSSID : " + WiFi.BSSIDstr());
    WebSerialLogger.print("[+] Gateway IP : ");
    WebSerialLogger.println(WiFi.gatewayIP().toString());
    WebSerialLogger.print("[+] DNS IP : ");
    WebSerialLogger.println(WiFi.dnsIP().toString());   
    WebSerialLogger.println((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    WebSerialLogger.print("[+] ESP32 IP : ");
    WebSerialLogger.println(WiFi.localIP().toString());
    WebSerialLogger.print("[+] Subnet Mask : ");
    WebSerialLogger.println(WiFi.subnetMask().toString());
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
        WebSerialLogger.println("WiFi connected!");
        connecting = false;
        DisplayInfo();
    }

    if(currentMillis - _lastMqttupdate > 30000 && connected && MQTTConnector.isActive())
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

            MQTTConnector.PublishMessage(payload, "WIFI");
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
WIFIManagerClass WIFIManager;