#include "Arduino.h"
#include "ESPAsyncWebServer.h"

#ifndef LOGGING_H
#define LOGGING_H

// Number of past boot-logs kept on flash. log0.txt is always the current
// boot, higher numbers are older boots, shifted up on every rotation.
#define LOGFILE_COUNT 5
// Cap on a single boot-log's size so runaway logging can't fill the flash.
#define LOGFILE_MAX_SIZE 32768UL

class pmLoggingClass
{
    private:
        bool _fileLoggingReady = false;
        bool _atLineStart = true;
        size_t _currentLogFileSize = 0;

        String logFileName(int index);
        void rotateLogFiles();
        void writeToLogFile(const char *text, bool newline);

    public:
        void Log(String text, bool newline = false, bool towebserial = true, bool toserial = true);
        void LogLn(String text, bool towebserial = true, bool toserial = true);

        void Log(const char *text, bool newline = false, bool towebserial = true, bool toserial = true);
        void LogLn(const char *text, bool towebserial = true, bool toserial = true);

        void LogLn();

        void Setup();
        void Begin();

        String handleLogsRoot(AsyncWebServerRequest *request);
        String handleLogsPost(AsyncWebServerRequest *request);
};

extern pmLoggingClass pmLogging;

#endif