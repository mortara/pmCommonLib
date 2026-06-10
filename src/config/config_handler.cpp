#include "config_handler.hpp"
#include "../pmCommonLib.hpp"

void handleConfigManagerRoot(AsyncWebServerRequest *request) {

    String content = "";
    String location = "ESP device manager";
    String pagetitle = "Main";
    String req = request->url();
    req.toLowerCase();


    pmLogging.LogLn("Webserver handle request ... " + req);

    for (std::list<ConfigPageDefinition>::iterator it = pmCommonLib.ConfigHandler.ConfigPages.begin(); it != pmCommonLib.ConfigHandler.ConfigPages.end(); ++it){
      String page = "/config/" + it->Name + ".html";
      if(req.endsWith(page))
      {
        location += " -> " + it->Name;
        pagetitle = it->Name;
        pagetitle.toUpperCase();
        if(request->method() == HTTP_GET)
          content = it->GETMethod(request);
        else
          content = it->POSTMethod(request);

        break;
      }
    }

    if(content == "")
    {
      content = "<p>No config pages registered!</p>";
      if(!pmCommonLib.ConfigHandler.ConfigPages.empty())
      {
        content = "<div>";

        for (std::list<ConfigPageDefinition>::iterator it = pmCommonLib.ConfigHandler.ConfigPages.begin(); it != pmCommonLib.ConfigHandler.ConfigPages.end(); ++it){
          String name = String(it->Name);
          
          content += "<p><a href='/config/" + it->Name + ".html'>" + name + "</a>\r\n";
        }

        content += "</div>";
      }
    }

    String html = "<!DOCTYPE html>\
                <html>\
                <head>\
                  <title>ESP device manager</title>\
                  <meta name='viewport' content='width=device-width, initial-scale=1'>\
                  <style>\
                        *, *::before, *::after { box-sizing: border-box; }\
                        html { font-family: Arial, Helvetica, sans-serif; }\
                        body { margin: 0; background-color: #f0f2f5; color: #222; }\
                        h1 { font-size: 1.6rem; color: white; margin: 0; padding: 16px 24px; }\
                        h2 { font-size: 1.1rem; color: #034078; margin: 0 0 20px 0; }\
                        .topnav { background-color: #0A1128; border-bottom: 3px solid #1282A2; }\
                        .bottomnav { text-align: center; padding: 16px; }\
                        .bottomnav a { color: #034078; text-decoration: none; font-weight: 600; }\
                        .bottomnav a:hover { color: #1282A2; }\
                        .content { padding: 32px 5%; }\
                        .card-grid { max-width: 480px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: 1fr; }\
                        .card { background-color: white; border-radius: 8px; box-shadow: 0 2px 12px rgba(0,0,0,0.1); padding: 28px 32px; }\
                        .card a { display: block; padding: 10px 0; color: #034078; text-decoration: none; font-size: 1rem; font-weight: 600; border-bottom: 1px solid #eee; }\
                        .card a:hover { color: #1282A2; }\
                        .card p:last-child a { border-bottom: none; }\
                        .form-group { display: flex; flex-direction: column; margin-bottom: 18px; }\
                        .form-group label { font-size: 0.85rem; font-weight: 600; color: #555; text-transform: uppercase; letter-spacing: 0.04em; margin-bottom: 6px; }\
                        .form-actions { margin-top: 24px; text-align: right; }\
                        input[type=text], input[type=password], input[type=number], select { width: 100%; padding: 10px 14px; border: 1px solid #ccc; border-radius: 5px; font-size: 1rem; background-color: #fafafa; }\
                        input[type=text]:focus, input[type=password]:focus, input[type=number]:focus, select:focus { outline: none; border-color: #1282A2; background-color: #fff; box-shadow: 0 0 0 3px rgba(18,130,162,0.15); }\
                        input[type=submit], button { border: none; color: #FEFCFB; background-color: #034078; padding: 10px 28px; font-size: 0.95rem; font-weight: 600; border-radius: 5px; cursor: pointer; transition: background-color 0.2s; }\
                        input[type=submit]:hover, button:hover { background-color: #1282A2; }\
                        label { font-size: 1rem; }\
                        .card-title { font-size: 1.1rem; font-weight: bold; color: #034078; margin-bottom: 16px; }\
                        .value, .state { font-size: 1.1rem; color: #1282A2; }\
                        .button-on { background-color: #034078; }\
                        .button-on:hover { background-color: #1282A2; }\
                        .button-off { background-color: #858585; }\
                        .button-off:hover { background-color: #252524; }\
                  </style>\
                </head>\
                <body>\
                  <div class='topnav'>\
                    <h1>" + location + "</h1>\
                  </div>\
                  <div class='content'>\
                    <div class='card-grid'>\
                      <div class='card'>\
                      <h2>" + pagetitle +"</h2>\
                      " + content +"\
                      </div>\
                    </div>\
                  </div>\
                  <div class='bottomnav'>\
                    <a href='/config/'>Back</a>\
                  </div>\
                </body>\
                </html>";

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

void pmConfigHandler::RegisterConfigPage(String name, ConfigHTTPRegisterFunction onGetRequest, ConfigHTTPRegisterFunction onPostRequest)
{
    Serial.println("Registering config-page " + name);

    ConfigPageDefinition newConfig;
    newConfig.Name = name;
    newConfig.GETMethod = onGetRequest;
    newConfig.POSTMethod = onPostRequest;

    ConfigPages.push_back(newConfig);

    String page = "/config/" + name + ".html";

    pmCommonLib.WebServer.RegisterOn(page.c_str(), handleConfigManagerRoot);
    pmCommonLib.WebServer.RegisterOn(page.c_str(), handleConfigManagerRoot, HTTP_POST);
}
