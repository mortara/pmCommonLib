#include "Arduino.h"

#ifndef LOGGING_H
#define LOGGING_H

class pmLoggingClass
{
    private:

    public:
        void Log(String text, bool newline = false, bool towebserial = true, bool toserial = true);
        void LogLn(String text, bool towebserial = true, bool toserial = true);

        void Log(const char *text, bool newline = false, bool towebserial = true, bool toserial = true);
        void LogLn(const char *text, bool towebserial = true, bool toserial = true);

        void LogLn();
};

extern pmLoggingClass pmLogging;

#endif