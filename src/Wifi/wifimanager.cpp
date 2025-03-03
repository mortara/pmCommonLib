#include "wifimanager.hpp"
#include "../MQTT/mqtt.hpp"

// Initialize LittleFS
void WIFIManagerClass::initLittleFS() {
    if (!LittleFS.begin(true)) {
      Serial.println("An error has occurred while mounting LittleFS");
    }
    Serial.println("LittleFS mounted successfully");
  }
  
  // Read File from LittleFS
  String readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);
  
    File file = fs.open(path);
    if(!file || file.isDirectory()){
      Serial.println("- failed to open file for reading");
      return String();
    }
    
    String fileContent;
    while(file.available()){
      fileContent = file.readStringUntil('\n');
      break;     
    }
    return fileContent;
  }
  
  // Write file to LittleFS
  void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);
  
    File file = fs.open(path, FILE_WRITE);
    if(!file){
      Serial.println("- failed to open file for writing");
      return;
    }
    if(file.print(message)){
      Serial.println("- file written");
    } else {
      Serial.println("- write failed");
    }
  }

  bool WIFIManagerClass::initWiFi() {
    if(_credentials.SSID =="" || _credentials.IP==""){
      Serial.println("Undefined SSID or IP address.");
      return false;
    }
  
    WiFi.mode(WIFI_STA);
    localIP.fromString(_credentials.IP.c_str());
    localGateway.fromString(_credentials.Gateway.c_str());
  
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.config(localIP, localGateway, subnet)){
      Serial.println("STA Failed to configure");
      return false;
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

void ServeWIFILogonPage(AsyncWebServerRequest *request)
{
    int params = request->params();
    for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == String(PARAM_INPUT_1)) {
            _credentials.SSID = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(_credentials.SSID);
            // Write file to save value
            writeFile(LittleFS, ssidPath, _credentials.SSID.c_str());
        }
        // HTTP POST pass value
        if (p->name() == String(PARAM_INPUT_2)) {
            _credentials.PASS = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(_credentials.PASS);
            // Write file to save value
            writeFile(LittleFS, passPath, _credentials.PASS.c_str());
        }
        // HTTP POST ip value
        if (p->name() == String(PARAM_INPUT_3)) {
            _credentials.IP = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(_credentials.IP);
            // Write file to save value
            writeFile(LittleFS, ipPath, _credentials.IP.c_str());
        }
        // HTTP POST gateway value
        if (p->name() == String(PARAM_INPUT_4)) {
            _credentials.Gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(_credentials.Gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, _credentials.Gateway.c_str());
        }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + _credentials.IP);
    delay(3000);
    ESP.restart();
}

void WIFIManagerClass::Setup(String hostname, AsyncWebServer *server)
{
    // Serial port for debugging purposes
    Serial.begin(115200);

    initLittleFS();

    // Load values saved in LittleFS
    _credentials.SSID = readFile(LittleFS, ssidPath);
    _credentials.PASS = readFile(LittleFS, passPath);
    _credentials.IP = readFile(LittleFS, ipPath);
    _credentials.Gateway = readFile (LittleFS, gatewayPath);
    Serial.println(_credentials.SSID);
    Serial.println(_credentials.PASS);
    Serial.println(_credentials.IP);
    Serial.println(_credentials.Gateway);

    if(initWiFi()) 
    {
 
    }
    else 
    {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
        // NULL sets an open Access Point
        WiFi.softAP("ESP-WIFI-MANAGER", NULL);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 

        // Web Server Root URL
        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/wifimanager.html", "text/html");
        });
        
        server->serveStatic("/", LittleFS, "/");
        
        server->on("/", HTTP_POST, ServeWIFILogonPage);

    }
}

bool WIFIManagerClass::Connect(String SSID, String PASS)
{
   
    return Connect();
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
    
    if(!MQTTConnector.SetupSensor("SSID", "sensor", "WIFI", "", "", ""))
    {
        WebSerialLogger.println("Unable to setup WIFI MQTT client");            
        return;
    }

    MQTTConnector.SetupSensor("BSSID", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("WIFI_RSSI", "sensor", "WIFI", "signal_strength", "dB", "mdi:sine-wave");
    MQTTConnector.SetupSensor("Hostname", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("IP", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("SubnetMask", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("Gateway", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("DNS", "sensor", "WIFI", "", "", "");

    WebSerialLogger.println("WIfi mqtt setup done!");

    mqttsetup = true;
}

void WIFIManagerClass::Disconnect()
{
    WebSerialLogger.println("disonnecting from WiFi ..");
    WiFi.disconnect();
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
    WebSerialLogger.print("[*] Network information for ");
    WebSerialLogger.println(_credentials.SSID);

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
            payload["SSID"] = _credentials.SSID;
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
    if (!connecting && !connected && (currentMillis - _lastConnectionTry >= interval)) 
    {
        WebSerialLogger.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        connecting = true;
        _lastConnectionTry = currentMillis;
    }
}

WIFICreds _credentials;
WIFIManagerClass WIFIManager;