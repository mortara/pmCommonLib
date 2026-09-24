#include "config_handler.hpp"
#include "../pmCommonLib.hpp"

static const char CONFIG_CSS[] PROGMEM = R"CSS(
*,*::before,*::after{box-sizing:border-box}
html{font-family:system-ui,-apple-system,Arial,sans-serif}
body{margin:0;background:#f0f2f5;color:#222}
.topnav{background:#0A1128;border-bottom:3px solid #1282A2}
.topnav h1{font-size:1.3rem;color:#fff;margin:0;padding:14px 24px 6px}
.tabs{display:flex;flex-wrap:wrap;gap:4px;padding:0 16px}
.tabs a{color:#cfd8e3;text-decoration:none;padding:8px 16px;border-radius:6px 6px 0 0;font-size:.95rem}
.tabs a:hover{color:#fff}
.tabs a.active{background:#f0f2f5;color:#034078;font-weight:600}
.content{padding:24px 16px}
.card{max-width:520px;margin:0 auto;background:#fff;border-radius:8px;box-shadow:0 2px 12px rgba(0,0,0,.1);padding:24px 28px}
h2{font-size:1.1rem;color:#034078;margin:0 0 20px}
.form-group{display:flex;flex-direction:column;margin-bottom:16px}
.form-group label{font-size:.8rem;font-weight:600;color:#555;text-transform:uppercase;letter-spacing:.04em;margin-bottom:6px}
.hint{font-size:.8rem;color:#777;margin-top:4px}
input[type=text],input[type=password],input[type=number],select{width:100%;padding:10px 14px;border:1px solid #ccc;border-radius:5px;font-size:1rem;background:#fafafa}
input:focus,select:focus{outline:none;border-color:#1282A2;background:#fff;box-shadow:0 0 0 3px rgba(18,130,162,.15)}
.form-actions{margin-top:24px;text-align:right}
input[type=submit],button,.btn{border:none;color:#fff;background:#034078;padding:10px 28px;font-size:.95rem;font-weight:600;border-radius:5px;cursor:pointer}
input[type=submit]:hover,button:hover,.btn:hover{background:#1282A2}
.btn-danger{background:#a33}.btn-danger:hover{background:#c44}
.notice{padding:10px 14px;border-radius:5px;margin-bottom:18px;background:#e3f4ea;color:#1b5e3a;border:1px solid #b7dfc7}
.notice.error{background:#fdeaea;color:#8a1f1f;border-color:#f0bcbc}
.list{list-style:none;margin:0 0 20px;padding:0}
.list li{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #eee}
.list a{color:#034078;text-decoration:none;font-weight:600}
.muted{color:#777;font-size:.9rem}
pre.log{white-space:pre-wrap;word-break:break-word;background:#111;color:#0f0;padding:12px;border-radius:5px;max-height:70vh;overflow:auto;font-size:.85rem}
)CSS";

String pmConfigHandler::HtmlEscape(const String &in)
{
    String out;
    out.reserve(in.length());

    for(size_t i = 0; i < in.length(); i++)
    {
        char c = in[i];
        if(c == '&')       out += "&amp;";
        else if(c == '<')  out += "&lt;";
        else if(c == '>')  out += "&gt;";
        else if(c == '"')  out += "&quot;";
        else if(c == '\'') out += "&#39;";
        else               out += c;
    }

    return out;
}

String pmConfigHandler::FormStart(const String &page)
{
    return "<form action='/config/" + page + ".html' method='POST'>";
}

String pmConfigHandler::FormEnd(const String &submitLabel)
{
    return "<div class='form-actions'><input type='submit' value='" + HtmlEscape(submitLabel) + "'></div></form>";
}

String pmConfigHandler::TextField(const String &name, const String &label, const String &value, const String &type, const String &hint)
{
    String html = "<div class='form-group'><label for='" + name + "'>" + HtmlEscape(label) + "</label>";
    html += "<input type='" + type + "' id='" + name + "' name='" + name + "' value='" + HtmlEscape(value) + "'>";
    if(hint != "")
        html += "<span class='hint'>" + HtmlEscape(hint) + "</span>";
    html += "</div>";
    return html;
}

String pmConfigHandler::SelectField(const String &name, const String &label, const std::vector<std::pair<String, String>> &options, const String &selected)
{
    String html = "<div class='form-group'><label for='" + name + "'>" + HtmlEscape(label) + "</label>";
    html += "<select id='" + name + "' name='" + name + "'>";
    for(const auto &o : options)
        html += "<option value='" + HtmlEscape(o.first) + "'" + (o.first == selected ? " selected" : "") + ">" + HtmlEscape(o.second) + "</option>";
    html += "</select></div>";
    return html;
}

String pmConfigHandler::Notice(const String &message, bool error)
{
    return String("<div class='notice") + (error ? " error" : "") + "'>" + HtmlEscape(message) + "</div>";
}

static void handleConfigCSS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/css", FPSTR(CONFIG_CSS));
    response->addHeader("Cache-Control", "max-age=86400");
    request->send(response);
}

void handleConfigManagerRoot(AsyncWebServerRequest *request) {

    String req = request->url();
    req.toLowerCase();

    pmLogging.LogLn("Webserver handle request ... " + req);

    std::list<ConfigPageDefinition> &pages = pmCommonLib.ConfigHandler.ConfigPages;

    if(pages.empty())
    {
        request->send(200, "text/plain", "No config pages registered!");
        return;
    }

    ConfigPageDefinition *current = nullptr;
    for (auto it = pages.begin(); it != pages.end(); ++it){
      if(req.endsWith("/config/" + it->Name + ".html"))
      {
        current = &(*it);
        break;
      }
    }

    // /config/ itself has no content of its own: send the user to the first page
    // ("general" if present) instead of showing a page that only lists links.
    if(current == nullptr)
    {
        String target = pages.front().Name;
        for (auto it = pages.begin(); it != pages.end(); ++it)
            if(it->Name == "general")
                target = it->Name;

        request->redirect("/config/" + target + ".html");
        return;
    }

    String content = (request->method() == HTTP_GET) ? current->GETMethod(request) : current->POSTMethod(request);

    String device = WiFi.getHostname() ? String(WiFi.getHostname()) : String("ESP");
    String tabs;
    for (auto it = pages.begin(); it != pages.end(); ++it){
      String tabtitle = it->Title != "" ? it->Title : it->Name;
      tabs += "<a href='/config/" + it->Name + ".html'" + (&(*it) == current ? " class='active'" : "") + ">" + pmConfigHandler::HtmlEscape(tabtitle) + "</a>";
    }

    String title = current->Title != "" ? current->Title : current->Name;

    String html = "<!DOCTYPE html><html><head><title>" + pmConfigHandler::HtmlEscape(device) + " - " + pmConfigHandler::HtmlEscape(title) + "</title>"
                  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                  "<link rel='icon' href='data:,'>"
                  "<link rel='stylesheet' href='/config/style.css'></head><body>"
                  "<div class='topnav'><h1>" + pmConfigHandler::HtmlEscape(device) + "</h1><nav class='tabs'>" + tabs + "</nav></div>"
                  "<div class='content'><div class='card'><h2>" + pmConfigHandler::HtmlEscape(title) + "</h2>" + content + "</div></div>"
                  "</body></html>";

    request->send(200, "text/html", html);
}

bool pmConfigHandler::Setup()
{
    if(!initLittleFS())
      return false;

    pmLogging.LogLn("Config-Manager setup complete!");
    _setup = true;
    return true;
}


void pmConfigHandler::Begin()
{
    if(!_setup)
      return;

    pmCommonLib.WebServer.RegisterOn("/config/style.css", handleConfigCSS);
    pmCommonLib.WebServer.RegisterOn("/config/", handleConfigManagerRoot);
    pmCommonLib.WebServer.RegisterOn("/config/", handleConfigManagerRoot, HTTP_POST);
}


bool pmConfigHandler::initLittleFS()
{
    if(_setup)
        return true;
      
    #if defined(ESP8266)
    if (!LittleFS.begin()) {
      Serial.println("An error has occurred while mounting LittleFS");
      return false;
    }
    #else
    if (!LittleFS.begin(true)) {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
    }
    #endif

    Serial.println("LittleFS mounted successfully");
    
    return true;
}
  
  // Read File from LittleFS
String pmConfigHandler::readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    #if defined(ESP8266)
    File file = fs.open(path, "r");
    #else
    File file = fs.open(path);
    #endif
    if(!file || file.isDirectory()){
      Serial.println("- failed to open file for reading");
      return String();
    }
    
    String fileContent;
    while(file.available()){
      fileContent = file.readStringUntil('\n');
      break;     
    }
    file.close();
    return fileContent;
}

// Write file to LittleFS
void pmConfigHandler::writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, "w");
    
    if(!file){
      Serial.println("- failed to open file for writing");
      return;
    }
    if(file.print(message)){
      Serial.println("- file written");
    } else {
      Serial.println("- write failed");
    }
    file.close();
}

bool pmConfigHandler::SaveConfigFile(const char * name, JsonDocument data)
{
    if(!_setup)
        return false;

    String filetext;
    size_t size = serializeJson(data, filetext);

    if(size == 0)
      return false;

    writeFile(LittleFS, name, filetext.c_str());
    
    return true;
}

JsonDocument pmConfigHandler::LoadConfigFile(const char * name)
{
    JsonDocument doc;

    if(!_setup)
        return doc;

    String file = readFile(LittleFS, name);
    
    deserializeJson(doc, file);

    return doc;
}

void pmConfigHandler::RegisterConfigPage(String name, ConfigHTTPRegisterFunction onGetRequest, ConfigHTTPRegisterFunction onPostRequest, String title)
{
    Serial.println("Registering config-page " + name);

    ConfigPageDefinition newConfig;
    newConfig.Name = name;
    newConfig.Title = title;
    newConfig.GETMethod = onGetRequest;
    newConfig.POSTMethod = onPostRequest;

    ConfigPages.push_back(newConfig);

    String page = "/config/" + name + ".html";

    pmCommonLib.WebServer.RegisterOn(page.c_str(), handleConfigManagerRoot);
    pmCommonLib.WebServer.RegisterOn(page.c_str(), handleConfigManagerRoot, HTTP_POST);
}
