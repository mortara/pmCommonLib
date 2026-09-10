#include "wifimanager.hpp"
#include "../pmCommonLib.hpp"

String WIFIManagerClass::captivePortalRedirector(AsyncWebServerRequest *request)
{
    request->redirect("/config/wifi.html");
    return "";
}

String WIFIManagerClass::handleWifManagerRoot(AsyncWebServerRequest *request) {
    pmLogging.LogLn("Webserver handle wifi request ... ");

    String o1 = "";
    String o2 = "";

    if(_wificredentials.ConfigMode == "static")
      o2 = "selected";
    else
      o1 = "selected";

    String html = "<form action='/config/wifi.html' method='POST'>\
                    <p>\
                      <label for='ssid'>SSID</label>\
                      <input type='text' id ='ssid' name='ssid' value='" + _wificredentials.SSID + "'><br>\
                      <label for='pass'>Password</label>\
                      <input type='text' id ='pass' name='pass' value='" + _wificredentials.PASS + "'><br>\
                      <label for='pass'>Hostname</label>\
                      <input type='text' id ='hostname' name='hostname' value='" + _wificredentials.Hostname + "'><br>\
                      <label for='ipmode'>Network configuration</label>\
                      <select id='ipmode' name='ipmode'>\
                        <option value='dhcp' " + o1 + ">DHCP</option>\
                        <option value='static' " + o2 + ">Static</option>\
                      </select><br>\
                      <label for='ip'>IP address</label>\
                      <input type='text' id ='ip' name='ip' value='" + _wificredentials.IP + "'><br>\
                      <label for='subnet'>Subnet</label>\
                      <input type='text' id ='subnet' name='subnet' value='" + _wificredentials.Subnet + "'><br>\
                      <label for='gateway'>Gateway</label>\
                      <input type='text' id ='gateway' name='gateway' value='" + _wificredentials.Gateway + "'><br>\
                      <label for='dns'>DNS server</label>\
                      <input type='text' id ='dns' name='dns' value='" + _wificredentials.DNS + "'><br>\
                      <input type ='submit' value ='Submit'>\
                    </p>\
                  </form>";

    return html;
}

String WIFIManagerClass::handlePOSTrequest(AsyncWebServerRequest *request)
{
   pmLogging.LogLn("Handling POST request ...");

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

          // HTTP POST dns value
          if (p->name() == "dns") {
            _wificredentials.DNS = p->value();
              Serial.print("DNS server set to: ");
              Serial.println(_wificredentials.DNS);
          }

          if (p->name() == "hostname") {
            _wificredentials.Hostname = p->value();
              Serial.print("Host name set to: ");
              Serial.println(_wificredentials.Hostname);
          }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
    }

    SaveConfig();
    Disconnect();
    Connect();
    return "";
}

bool WIFIManagerClass::initWiFi() 
{
    if(_wificredentials.SSID == ""){
      pmLogging.LogLn("Undefined SSID");
      return false;
    }

    if(_wificredentials.ConfigMode == "static")
    {
        IPAddress localIP;
        IPAddress localGateway;
        IPAddress subnet;
        IPAddress dns;

        dns.fromString(_wificredentials.DNS.c_str());
        localIP.fromString(_wificredentials.IP.c_str());
        localGateway.fromString(_wificredentials.Gateway.c_str());

        if(_wificredentials.Subnet != "")
          subnet.fromString(_wificredentials.Subnet.c_str());
        else
          subnet.fromString("255.255.255.0");

        
        if (!WiFi.config(localIP, localGateway, subnet, dns)){
          pmLogging.LogLn("STA Failed to configure with static IP");
          return false;
        }
    }
    else
    {
      pmLogging.LogLn("DHCP enabled");
    }

    // Hostname must be set before WiFi.begin()/WiFi.mode() to reliably take
    // effect for the DHCP client's hostname option.
    if(_wificredentials.Hostname != "")
        WiFi.setHostname(_wificredentials.Hostname.c_str());

    // Keep the AP alive while we retry STA from within the captive portal,
    // otherwise switching mode here would tear the portal down mid-retry.
    WiFi.mode(captiveportalactive ? WIFI_AP_STA : WIFI_STA);
    // tryConnect must be true — false only configures the STA credentials
    // without ever calling esp_wifi_connect(), so the device would sit
    // "configured" but never actually attempt to join the network.
    WiFi.begin(_wificredentials.SSID.c_str(), _wificredentials.PASS.c_str(), 0, __null, true);

    connecting = true;
    _lastConnectionTry = millis();
    _connectionAttempts = 0;

    return true;
}

void WIFIManagerClass::Setup(bool autoconnect)
{
  pmLogging.LogLn("Setting up WIFI manager");
  LoadConfig();

  if(autoconnect)
    Connect();

  if(_wificredentials.Hostname == "")
    _wificredentials.Hostname = WiFi.getHostname();

  ConfigHTTPRegisterFunction f1 = std::bind(&WIFIManagerClass::handleWifManagerRoot, this, std::placeholders::_1);
  ConfigHTTPRegisterFunction f2 = std::bind(&WIFIManagerClass::handlePOSTrequest, this, std::placeholders::_1);

  pmCommonLib.ConfigHandler.RegisterConfigPage("wifi", f1, f2);
}

void WIFIManagerClass::Begin()
{
  // Avoid kicking off a second, redundant connection attempt if Setup()
  // (autoconnect) already has one in flight.
  if(!connecting && !connected)
    Connect();
}

void WIFIManagerClass::LoadConfig()
{
  JsonDocument doc = pmCommonLib.ConfigHandler.LoadConfigFile(WIFIconfigFilePath);

  if(doc["SSID"].is<String>())
  {
      pmLogging.LogLn("Reading config-file!");
      _wificredentials.ConfigMode = doc["ConfigMode"].as<String>();
      _wificredentials.SSID = doc["SSID"].as<String>();
      _wificredentials.PASS = doc["PASS"].as<String>();
      _wificredentials.IP = doc["IP"].as<String>();
      _wificredentials.Gateway = doc["Gateway"].as<String>();
      _wificredentials.Subnet = doc["Subnet"].as<String>();
      _wificredentials.DNS = doc["dns"].as<String>();
      _wificredentials.Hostname = doc["Hostname"].as<String>();

      pmLogging.LogLn("SSID: " + _wificredentials.SSID);
      pmLogging.LogLn("PASS: " + _wificredentials.PASS);
  }
}

void WIFIManagerClass::SaveConfig()
{
  JsonDocument data;
  data["SSID"] = _wificredentials.SSID;
  data["PASS"] = _wificredentials.PASS;
  data["IP"] = _wificredentials.IP;
  data["Subnet"] = _wificredentials.Subnet;
  data["Gateway"] = _wificredentials.Gateway;
  data["ConfigMode"] = _wificredentials.ConfigMode;
  data["Hostname"] = _wificredentials.Hostname;
  data["dns"] = _wificredentials.DNS;
  pmCommonLib.ConfigHandler.SaveConfigFile(WIFIconfigFilePath, data);
}

void WIFIManagerClass::StartCaptivePortal()
{
  // Connect to Wi-Fi network with SSID and password
  pmLogging.LogLn("Setting AP (Access Point)");

  // AP_STA (rather than plain AP) so the device can keep retrying the
  // stored STA credentials in the background while the portal is up.
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP-WIFI-MANAGER-" + _wificredentials.Hostname, "", 7);

  IPAddress IP = WiFi.softAPIP();
  pmLogging.LogLn("AP IP address: " + IP.toString());

  ConfigHTTPRegisterFunction f1 = std::bind(&WIFIManagerClass::captivePortalRedirector, this, std::placeholders::_1);
  
  pmCommonLib.WebServer.RegisterOn("/", f1);
  //pmCommonLib.WebServer.StopRegistrations();
  captiveportalactive = true;
}



bool WIFIManagerClass::Connect()
{
   // No stored credentials (e.g. a freshly flashed device) — initWiFi()
   // would just log "Undefined SSID" and bail out without ever setting
   // `connecting`, so Loop()'s connect-timeout path that falls back to the
   // captive portal would never trigger. Start the portal directly instead.
   if(_wificredentials.SSID == "")
   {
       if(!captiveportalactive)
           StartCaptivePortal();
       return false;
   }

   // A full initWiFi() (fresh WiFi.begin() with the current credentials) is
   // more reliable than WiFi.reconnect(), and also (re)establishes the
   // connecting/_lastConnectionTry bookkeeping Loop() relies on.
   return initWiFi();
}

#ifndef PMCOMMONNOMQTT
void WIFIManagerClass::setupMQTT()
{
    if(mqttsetup)
        return;

    pmLogging.LogLn("Setting up Wifi MQTT client");
    
    if(!pmCommonLib.MQTTConnector.SetupSensor("SSID", "WIFI", "", "", ""))
    {
        pmLogging.LogLn("Unable to setup WIFI MQTT client");            
        return;
    }

    pmCommonLib.MQTTConnector.SetupSensor("BSSID", "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("WIFI_RSSI", "WIFI", "signal_strength", "dB", "mdi:sine-wave");
    pmCommonLib.MQTTConnector.SetupSensor("Hostname",  "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("IP", "WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("SubnetMask","WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("Gateway","WIFI", "", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("DNS", "WIFI", "", "", "");

    pmLogging.LogLn("WIfi mqtt setup done!");

    mqttsetup = true;
}
#endif

void WIFIManagerClass::Disconnect()
{
    pmLogging.LogLn("disonnecting from WiFi ..");

    connecting = false;
    connected = false;
    _connectionAttempts = 0;

    WiFi.disconnect();
}

void WIFIManagerClass::DisplayInfo(){
  pmLogging.LogLn("Dumping WIFI info ");
  if(WiFi.getMode() == WIFI_AP)
  {
    pmLogging.LogLn("[*] SoftAp is running ");
    pmLogging.Log("[+] ESP32 Hostname : ");
    pmLogging.LogLn(WiFi.getHostname());
    pmLogging.Log("[+] ESP32 IP : ");
    pmLogging.LogLn(WiFi.softAPIP().toString());
  }
  else
  {
  
    pmLogging.Log("[*] Network information for ");
    pmLogging.LogLn(WiFi.SSID());

    pmLogging.LogLn("[+] BSSID : " + WiFi.BSSIDstr());
    pmLogging.Log("[+] Gateway IP : ");
    pmLogging.LogLn(WiFi.gatewayIP().toString());
    pmLogging.Log("[+] DNS IP : ");
    pmLogging.LogLn(WiFi.dnsIP().toString());   
    pmLogging.LogLn((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    pmLogging.Log("[+] ESP32 IP : ");
    pmLogging.LogLn(WiFi.localIP().toString());
    pmLogging.Log("[+] Subnet Mask : ");
    pmLogging.LogLn(WiFi.subnetMask().toString());
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

IPAddress WIFIManagerClass::GetIP()
{
    if(!WiFi.isConnected())
      return IPAddress();

    if(WiFi.getMode() == WIFI_AP)
      return WiFi.softAPIP();

    return WiFi.localIP();
}

void WIFIManagerClass::Loop()
{
    unsigned long currentMillis = millis();

    connected = WiFi.isConnected();

    if(connecting && connected)
    {
        pmLogging.LogLn("WiFi connected!");
        
        if(_wificredentials.ConfigMode == "dhcp")
        {
          pmLogging.LogLn("getting dhcp parameters");
          _wificredentials.IP = WiFi.localIP().toString();
          _wificredentials.Subnet = WiFi.subnetMask().toString();
          _wificredentials.Gateway = WiFi.gatewayIP().toString();
          _wificredentials.DNS = WiFi.dnsIP().toString();
        }

        connecting = false;
        _connectionAttempts = 0;

        if(captiveportalactive)
        {
            pmLogging.LogLn("Reconnected to configured WiFi network, stopping captive portal");
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            captiveportalactive = false;
        }

        DisplayInfo();
    }

    if(connecting && !connected && currentMillis - _lastConnectionTry > WIFI_CONNECT_TIMEOUT_MS)
    {
        _connectionAttempts++;

        if(_connectionAttempts < WIFI_MAX_CONNECT_ATTEMPTS)
        {
            pmLogging.LogLn("WiFi connect attempt " + String(_connectionAttempts) + " of " + String(WIFI_MAX_CONNECT_ATTEMPTS) + " timed out, retrying ...");
            _lastConnectionTry = currentMillis;
            WiFi.disconnect();
            WiFi.begin(_wificredentials.SSID.c_str(), _wificredentials.PASS.c_str(), 0, __null, true);
        }
        else
        {
            pmLogging.LogLn("WiFi could not be connected after " + String(_connectionAttempts) + " attempts!");
            pmLogging.LogLn("Starting captive Portal");
            Disconnect();
            _lastPortalRetry = currentMillis;
            StartCaptivePortal();
        }
    }

    if(captiveportalactive && !connecting && !connected && currentMillis - _lastPortalRetry > WIFI_PORTAL_RETRY_INTERVAL_MS)
    {
        pmLogging.LogLn("Retrying configured WiFi network while captive portal is active ...");
        _lastPortalRetry = currentMillis;
        Connect();
    }

    #ifndef PMCOMMONNOMQTT
    if(currentMillis - _lastMqttupdate > 30000 && connected && pmCommonLib.MQTTConnector.isActive())
    {
        if(!mqttsetup)
        {
            setupMQTT();
        }
        else
        {
           pmLogging.LogLn("Sending WIFI status to MQTT ");
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
    #endif
}
