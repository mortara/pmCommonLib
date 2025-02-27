#include "wifimanager.hpp"
#include "../MQTT/mqtt.hpp"

void saveWifiCallback(){
    Serial.println("[CALLBACK] saveCallback fired");
        delay(2000);
  }
  
  void configPortalTimeoutCallback(){
    Serial.println("[CALLBACK] configPortalTimeoutCallback fired");
        delay(2000);
  }
  
  void configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
  
    Serial.println(myWiFiManager->getConfigPortalSSID());
  }

  void WIFIManagerClass::Setup(String hostname)
  {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname.c_str());
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConnectTimeout(20);
   
    std::vector<const char *> menu = { "wifi", "restart", "exit" };  // Added by me...
    wifiManager.setMenu(menu);
    
    if(wifiManager.autoConnect("AutoConnectAP"))
    {
      Serial.println("WIFI Connected");
    }
  }

bool WIFIManagerClass::Connect()
{
    if(WiFi.status() == WL_CONNECTED || connecting)
    {
        //WebSerialLogger.println("Already connected or connecting");
        return true;
    }

    wifiManager.autoConnect();
    
    connecting = true;
    
    WebSerialLogger.println("Connecting to WiFi ..");
    _lastConnectionTry = millis();
   
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
    wifiManager.disconnect();
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
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

    wifiManager.process();

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
    if (!connecting && !connected && (currentMillis - _lastConnectionTry >= interval)) 
    {
        WebSerialLogger.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        connecting = true;
        _lastConnectionTry = currentMillis;
    }
}

WIFIManagerClass WIFIManager;