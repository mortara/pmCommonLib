#include "webserver.hpp"

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


void WebServerClass::Setup()
{
    Serial.println("Setting up Webserver");
    _webserver = new AsyncWebServer(80);
    _webserver->onNotFound(notFound);
    Serial.println("webserver done");
}

void WebServerClass::Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound)
{
    Serial.println("Setting up Webserver");
    _webserver = new AsyncWebServer(80);
    _webserver->on("/", onRequest);
    _webserver->onNotFound(onNotFound);
}

bool WebServerClass::checkSetup()
{
    if(_webserver == nullptr)
    {
        Serial.println("Webserver not setup");
        return false;
    }

    return true;
}

void WebServerClass::RegisterOn(const char *path, ArRequestHandlerFunction onRequest, WebRequestMethod method)
{
    if(!checkSetup())
        return;

    _webserver->on(path, method, onRequest);
}

void WebServerClass::RegisterNotFound(ArRequestHandlerFunction onRequest)
{
    if(!checkSetup())
        return;

    _webserver->onNotFound(onRequest);
}

void WebServerClass::Begin()
{
    if(_dnsServer == nullptr)
    {
        Serial.println("Starting DNS Server");
        _dnsServer = new DNSServer();
        _dnsServer->start(53, "*", WiFi.softAPIP());
    }

    if(!checkSetup())
        return;

    _webserver->begin();
}

AsyncWebServer *WebServerClass::GetServer()
{
    return _webserver;
}

WebServerClass WebServer;