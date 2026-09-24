#include "Arduino.h"
#include "ESPAsyncWebServer.h"

#ifndef LOGGING_H
#define LOGGING_H

// Number of past boot-logs kept on flash. log0.txt is always the current
// boot, higher numbers are older boots, shifted up on every rotation.
#define LOGFILE_COUNT 5
// Cap on a single boot-log's size so runaway logging can't fill the flash.
#define LOGFILE_MAX_SIZE 32768UL

// Log text destined for the file is buffered in RAM and only flushed to
// LittleFS once one of these thresholds is hit, instead of doing a
// synchronous open+write+close on every single Log()/LogLn() call. This
// keeps the (highly variable, sometimes 100ms+) flash write latency off
// the caller's stack.
#define LOGFILE_FLUSH_THRESHOLD_BYTES 256UL
#define LOGFILE_FLUSH_INTERVAL_MS 1000UL

class pmLoggingClass
{
    private:
        bool _fileLoggingReady = false;
        bool _atLineStart = true;
        size_t _currentLogFileSize = 0;

        String _fileBuffer;
        unsigned long _lastFlushMillis = 0;

        String logFileName(int index);
        void rotateLogFiles();
        void bufferForFile(const char *text, bool newline);
        void flushLogBuffer();

    public:
        void Log(String text, bool newline = false, bool towebserial = true, bool toserial = true, bool tofile = true);
        void LogLn(String text, bool towebserial = true, bool toserial = true, bool tofile = true);

        void Log(const char *text, bool newline = false, bool towebserial = true, bool toserial = true, bool tofile = true);
        void LogLn(const char *text, bool towebserial = true, bool toserial = true, bool tofile = true);

        void LogLn();

        void Setup();
        void Begin();
        void Loop();

        String handleLogsRoot(AsyncWebServerRequest *request);
        String handleLogsPost(AsyncWebServerRequest *request);
};

extern pmLoggingClass pmLogging;

#endif