#include "webserver.hpp"

void WebServerClass::Setup(ArRequestHandlerFunction onRequest, ArRequestHandlerFunction onNotFound)
{
    _webserver = new AsyncWebServer(80);
    _webserver->on("/", onRequest);

    dnsServer.start(53, "*", WiFi.softAPIP());
    _webserver->onNotFound(onNotFound);
    _webserver->begin();
}

AsyncWebServer *WebServerClass::GetServer()
{
    return _webserver;
}

WebServerClass WebServer;