#include "config_handler.hpp"
#include "../pmCommonLib.hpp"

void handleConfigManagerRoot(AsyncWebServerRequest *request) {

    String content = "";
    String location = "ESP device manager";
    String pagetitle = "Main";
    String req = request->url();
    req.toLowerCase();


    Serial.println("Webserver handle request ... " + req);

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
                        html {\
                          font-family: Arial, Helvetica, sans-serif; \
                          display: inline-block; \
                          text-align: center;\
                        }\
                        h1 {\
                          font-size: 1.8rem; \
                          color: white;\
                        }\
                        p {\
                          font-size: 1.4rem;\
                        }\
                        .topnav { \
                          overflow: hidden; \
                          background-color: #0A1128;\
                        }\
                        body {\
                          margin: 0;\
                        }\
                        .content {\
                          padding: 5%;\
                        }\
                        .card-grid {\
                          max-width: 800px;\
                          margin: 0 auto; \
                          display: grid; \
                          grid-gap: 2rem; \
                          grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));\
                        }\
                        .card {\
                          background-color: white; \
                          box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);\
                        }\
                        .card-title { \
                          font-size: 1.2rem;\
                          font-weight: bold;\
                          color: #034078\
                        }\
                        input[type=submit] {\
                          border: none;\
                          color: #FEFCFB;\
                          background-color: #034078;\
                          padding: 15px 15px;\
                          text-align: center;\
                          text-decoration: none;\
                          display: inline-block;\
                          font-size: 16px;\
                          width: 100px;\
                          margin-right: 10px;\
                          border-radius: 4px;\
                          transition-duration: 0.4s;\
                          }\
                        input[type=submit]:hover {\
                          background-color: #1282A2;\
                        }\
                        input[type=text], input[type=number], select {\
                          width: 50%;\
                          padding: 12px 20px;\
                          margin: 18px;\
                          display: inline-block;\
                          border: 1px solid #ccc;\
                          border-radius: 4px;\
                          box-sizing: border-box;\
                        }\
                        label {\
                          font-size: 1.2rem;\
                        }\
                        .value{\
                          font-size: 1.2rem;\
                          color: #1282A2;  \
                        }\
                        .state {\
                          font-size: 1.2rem;\
                          color: #1282A2;\
                        }\
                        button {\
                          border: none;\
                          color: #FEFCFB;\
                          padding: 15px 32px;\
                          text-align: center;\
                          font-size: 16px;\
                          width: 100px;\
                          border-radius: 4px;\
                          transition-duration: 0.4s;\
                        }\
                        .button-on {\
                          background-color: #034078;\
                        }\
                        .button-on:hover {\
                          background-color: #1282A2;\
                        }\
                        .button-off {\
                          background-color: #858585;\
                        }\
                        .button-off:hover {\
                          background-color: #252524;\
                        }\
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

    Serial.println("Config-Manager setup complete!");
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
    #elif
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
    #elif
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
    return fileContent;
}

// Write file to LittleFS
void pmConfigHandler::writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    #if defined(ESP8266)
    File file = fs.open(path, "r");
    #elif
    File file = fs.open(path, "w");
    #endif

    
    if(!file){
      Serial.println("- failed to open file for writing");
      return;
    }
    if(file.print(message)){
      Serial.println("- file written");
    } else {
      Serial.println("- write failed");
    }
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
