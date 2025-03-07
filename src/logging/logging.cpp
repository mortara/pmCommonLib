#include "logging.hpp"
#include "pmCommonLib.hpp"


void pmLoggingClass::LogLn(String text, bool towebserial, bool toserial)
{
    Log(text.c_str(),true, towebserial,toserial);
}

void pmLoggingClass::Log(String text, bool newline, bool towebserial, bool toserial)
{
    Log(text.c_str(),false, towebserial,toserial);
}

void pmLoggingClass::LogLn(const char *text, bool towebserial, bool toserial)
{
    Log(text, true, towebserial, toserial);
}

void pmLoggingClass::LogLn()
{
    Log("", true);
}

void pmLoggingClass::Log(const char *text, bool newline, bool towebserial, bool toserial)
{
    #ifndef PMCOMMONNOWEBSERIAL
    if(towebserial)
    {
        if(newline)
            pmCommonLib.WebSerial.println(text);
        else
            pmCommonLib.WebSerial.print(text);
    }
    #endif

    if(toserial)
    {
        if(newline)
            Serial.println(text);
        else
            Serial.print(text);
    }
}


pmLoggingClass pmLogging;