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
    _setup = true;
}

void WebServerClass::Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound)
{
    Serial.println("Setting up Webserver");
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
    nomoreregistrations = true;
}

void WebServerClass::RegisterOn(const char *path, ArRequestHandlerFunction onRequest, WebRequestMethod method)
{
    if(!IsSetup() || nomoreregistrations)
        return;

    Serial.println("Registering on-handler: " + String(method) + " " + String(path));
    _webserver->on(path, method, onRequest);
}

void WebServerClass::RegisterNotFound(ArRequestHandlerFunction onRequest)
{
    if(!IsSetup() || nomoreregistrations)
        return;

    Serial.println("Registering notfound-handler");
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

    if(!IsSetup())
        return;
    
    Serial.println("Starting webserver!");
    _webserver->begin();
}

AsyncWebServer *WebServerClass::GetServer()
{
    return _webserver;
}
