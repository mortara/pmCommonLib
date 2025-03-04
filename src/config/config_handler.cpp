#include "config_handler.hpp"
#include "../pmCommonLib.hpp"

void handleConfigManagerRoot(AsyncWebServerRequest *request) {

    String req = request->url();
    req.toLowerCase();


    Serial.println("Webserver handle request ... " + req);

    for (std::list<ConfigPageDefinition>::iterator it = pmCommonLib.ConfigHandler.ConfigPages.begin(); it != pmCommonLib.ConfigHandler.ConfigPages.end(); ++it){
      if(req.startsWith("/config/" + it->Name))
      {
          if(request->method() == HTTP_POST)
          {
            it->POSTMethod(request);
          }
          else
          {
            it->GETMethod(request);
          }
          return;
      }
    }

    String config_options = "No config pages registered!";

    if(!pmCommonLib.ConfigHandler.ConfigPages.empty())
        config_options = "<ul>";

    for (std::list<ConfigPageDefinition>::iterator it = pmCommonLib.ConfigHandler.ConfigPages.begin(); it != pmCommonLib.ConfigHandler.ConfigPages.end(); ++it){
        config_options += "<li><a href='/config/" + it->Name + "'>" + it->Name + "</a></ul><br>\r\n";
    }

    if(!pmCommonLib.ConfigHandler.ConfigPages.empty())
        config_options = "</ul>";

    String html = "<!DOCTYPE html>\
                <html>\
                <head>\
                  <title>ESP Wi-Fi Manager</title>\
                  <meta name='viewport' content='width=device-width, initial-scale=1'>\
                  <link rel='icon' href='data:,'>\
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
                    <h1>ESP Wi-Fi Manager</h1>\
                  </div>\
                  <div class='content'>\
                    <div class='card-grid'>\
                      <div class='card'>\
                      " + config_options +"\
                      </div>\
                    </div>\
                  </div>\
                </body>\
                </html>";

  request->send(200, "text/html", html);

}

bool pmConfigHandler::Setup()
{
    if(!initLittleFS())
      return false;

    pmCommonLib.WebServer.RegisterOn("/config/", handleConfigManagerRoot);
    pmCommonLib.WebServer.RegisterOn("/config/", handleConfigManagerRoot, HTTP_POST);
    _setup = true;
    return true;
}

bool pmConfigHandler::initLittleFS()
{
    if(_setup)
        return true;

    if (!LittleFS.begin(true)) {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
    }
    Serial.println("LittleFS mounted successfully");
    
    return true;
}
  
  // Read File from LittleFS
String pmConfigHandler::readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
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

    File file = fs.open(path, FILE_WRITE);
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

void pmConfigHandler::RegisterConfigPage(String name, ArRequestHandlerFunction onGetRequest, ArRequestHandlerFunction onPostRequest)
{
    ConfigPageDefinition newConfig;
    newConfig.Name = name;
    newConfig.GETMethod = onGetRequest;
    newConfig.POSTMethod = onPostRequest;

    ConfigPages.push_back(newConfig);
}

pmConfigHandler ConfigHandler;