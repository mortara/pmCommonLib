#include "webserver.hpp"
#include "pmCommonLib.hpp"

WebServerClass::~WebServerClass()
{
    // Clean up web server
    if(_webserver != nullptr)
    {
        if(_serverrunning)
            _webserver->end();
        delete _webserver;
        _webserver = nullptr;
    }
    
    // Clean up DNS server
    if(_dnsServer != nullptr)
    {
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


void WebServerClass::Setup()
{
    pmLogging.LogLn("Setting up Webserver");
    _webserver = new AsyncWebServer(80);
    _webserver->onNotFound(notFound);
    pmLogging.LogLn("webserver done");
    _setup = true;
}

void WebServerClass::Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound)
{
    pmLogging.LogLn("Setting up Webserver");
    _webserver = new AsyncWebServer(80);
    _webserver->on("/", onRequest);
    _webserver->onNotFound(onNotFound);
    _setup = true;
}

bool WebServerClass::IsSetup()
{
    return _setup;
}

void WebServerClass::StopRegistrations()
{
    _nomoreregistrations = true;
}

void WebServerClass::Reset()
{
    _webserver->reset();
}

void WebServerClass::RegisterOn(const char *path, ArRequestHandlerFunction onRequest, WebRequestMethod method)
{
    if(!IsSetup() || _nomoreregistrations)
        return;

        pmLogging.LogLn("Registering on-handler: " + String(method) + " " + String(path));

    _webserver->on(path, method, onRequest);
}

void WebServerClass::RegisterNotFound(ArRequestHandlerFunction onRequest)
{
    if(!IsSetup() || _nomoreregistrations)
        return;

    pmLogging.LogLn("Registering notfound-handler");
    _webserver->onNotFound(onRequest);
}

void WebServerClass::Begin()
{
    if(!IsSetup())
        return;
    
    StartServer();
}

void WebServerClass::StartServer()
{
    pmLogging.LogLn("StartServer ....");
    _startserver = true;
    _serverrunning = false;
}

void WebServerClass::StopServer()
{
    pmLogging.LogLn("Stopping webserver!");

    if(_serverrunning)
        _webserver->end();

    _serverrunning = false;
    _startserver = false;

    
}

AsyncWebServer *WebServerClass::GetServer()
{
    return _webserver;
}

void WebServerClass::Loop()
{
    if(WiFi.isConnected() || WiFi.softAPIP().toString() != "0.0.0.0")
    {
        if(!_serverrunning && _startserver)
        {
            pmLogging.LogLn("Starting Webserver");
            _startserver = false;
            _serverrunning = true;
            _webserver->begin();
        }

        if(_dnsServer == nullptr)
        {
            pmLogging.LogLn("Starting DNS Server");
            _dnsServer = new DNSServer();
            _dnsServer->start(53, "*", pmCommonLib.WiFiManager.GetIP());
        }
    }
}