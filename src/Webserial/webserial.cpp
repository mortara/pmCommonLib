#include "webserial.hpp"

String inputString;

void recvMsg(const uint8_t *data, size_t len)
{
    Serial.println("Received Data...");
    String d = "";
    for(int i=0; i < (int)len; i++){
        d += char(data[i]);
    }

    inputString = inputString + d;

    Serial.println(d);
}

void WebSerialLoggerClass::Setup()
{
    Serial.println("Starting webserial server!");
    _setup = true;
}

void WebSerialLoggerClass::Begin(AsyncWebServer *_server)
{
    Serial.println("Starting webserial connection!");

    webSerial.begin(_server);
    webSerial.onMessage(recvMsg);

    running = true;
}

char WebSerialLoggerClass::GetInput()
{
    if(inputString == "")
        return ' ';

    char c = inputString[0];
    inputString = inputString.substring(1);
    return c;
}

bool WebSerialLoggerClass::IsSetup()
{
    return _setup;
}

bool WebSerialLoggerClass::IsRunning()
{
    return running;
}

void WebSerialLoggerClass::print(const char *text)
{
    if(running)
        webSerial.print(text);
}

void WebSerialLoggerClass::print(String text)
{
    if(running)
        webSerial.print(text);
}

void WebSerialLoggerClass::println(const char *text)
{
    if(running)
        webSerial.println(text);
}

void WebSerialLoggerClass::println(String text)
{
    if(running)
        webSerial.println(text);

}

void WebSerialLoggerClass::Loop()
{
    
}

void WebSerialLoggerClass::Flush()
{
    if(running)
        webSerial.flush();
}
