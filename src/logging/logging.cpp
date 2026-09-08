#include "logging.hpp"
#include "LittleFS.h"
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

    writeToLogFile(text, newline);
}

String pmLoggingClass::logFileName(int index)
{
    return "/log" + String(index) + ".txt";
}

void pmLoggingClass::rotateLogFiles()
{
    String oldest = logFileName(LOGFILE_COUNT - 1);
    if(LittleFS.exists(oldest))
        LittleFS.remove(oldest);

    for(int i = LOGFILE_COUNT - 2; i >= 0; i--)
    {
        String from = logFileName(i);
        String to = logFileName(i + 1);
        if(LittleFS.exists(from))
            LittleFS.rename(from, to);
    }
}

void pmLoggingClass::Setup()
{
    rotateLogFiles();

    File file = LittleFS.open(logFileName(0), "w");
    if(file)
        file.close();

    _currentLogFileSize = 0;
    _atLineStart = true;
    _fileLoggingReady = true;
}

void pmLoggingClass::Begin()
{
    ConfigHTTPRegisterFunction f1 = std::bind(&pmLoggingClass::handleLogsRoot, this, std::placeholders::_1);
    ConfigHTTPRegisterFunction f2 = std::bind(&pmLoggingClass::handleLogsPost, this, std::placeholders::_1);

    pmCommonLib.ConfigHandler.RegisterConfigPage("logs", f1, f2);
}

void pmLoggingClass::writeToLogFile(const char *text, bool newline)
{
    if(!_fileLoggingReady || _currentLogFileSize >= LOGFILE_MAX_SIZE)
        return;

    File file = LittleFS.open(logFileName(0), "a");
    if(!file)
        return;

    size_t written = 0;

    if(_atLineStart)
        written += file.print("[" + String(millis()) + "] ");

    written += file.print(text);

    if(newline)
    {
        written += file.println();
        _atLineStart = true;
    }
    else
    {
        _atLineStart = false;
    }

    _currentLogFileSize += written;
    file.close();
}

static String htmlEscape(const String &in)
{
    String out;
    out.reserve(in.length());

    for(size_t i = 0; i < in.length(); i++)
    {
        char c = in[i];
        if(c == '&')
            out += "&amp;";
        else if(c == '<')
            out += "&lt;";
        else if(c == '>')
            out += "&gt;";
        else
            out += c;
    }

    return out;
}

String pmLoggingClass::handleLogsRoot(AsyncWebServerRequest *request)
{
    if(request->hasParam("file"))
    {
        String requestedFile = request->getParam("file")->value();
        bool valid = false;

        for(int i = 0; i < LOGFILE_COUNT; i++)
        {
            if(requestedFile == logFileName(i).substring(1))
            {
                valid = true;
                break;
            }
        }

        if(!valid)
            return "<p>Unknown logfile.</p><p><a href='/config/logs.html'>Back to logfile list</a></p>";

        File file = LittleFS.open("/" + requestedFile, "r");
        if(!file)
            return "<p>Logfile not found.</p><p><a href='/config/logs.html'>Back to logfile list</a></p>";

        String content = file.readString();
        file.close();

        return "<p><a href='/config/logs.html'>Back to logfile list</a></p>\
                <pre style='white-space:pre-wrap;word-break:break-word;background:#111;color:#0f0;padding:12px;border-radius:5px;max-height:70vh;overflow:auto;'>" + htmlEscape(content) + "</pre>";
    }

    String html = "<p>Stored boot logfiles (log0 = current/most recent):</p><ul>";
    bool any = false;

    for(int i = 0; i < LOGFILE_COUNT; i++)
    {
        String name = logFileName(i);
        if(!LittleFS.exists(name))
            continue;

        any = true;
        File file = LittleFS.open(name, "r");
        size_t size = file ? file.size() : 0;
        if(file)
            file.close();

        html += "<li><a href='/config/logs.html?file=" + name.substring(1) + "'>" + name.substring(1) + "</a> (" + String(size) + " bytes)</li>";
    }

    if(!any)
        html += "<li>No logfiles stored yet.</li>";

    html += "</ul><form action='/config/logs.html' method='POST'><input type='hidden' name='action' value='clear'><input type='submit' value='Clear all logfiles'></form>";

    return html;
}

String pmLoggingClass::handleLogsPost(AsyncWebServerRequest *request)
{
    if(request->hasParam("action", true) && request->getParam("action", true)->value() == "clear")
    {
        for(int i = 0; i < LOGFILE_COUNT; i++)
        {
            String name = logFileName(i);
            if(LittleFS.exists(name))
                LittleFS.remove(name);
        }

        _currentLogFileSize = 0;
        _atLineStart = true;

        pmLogging.LogLn("Logfiles cleared by user");
    }

    return handleLogsRoot(request);
}


pmLoggingClass pmLogging;