#include "config_handler.hpp"



bool pmConfigHandler::initLittleFS()
{
    if(_setup)
        return true;

    if (!LittleFS.begin(true)) {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
    }
    Serial.println("LittleFS mounted successfully");
    _setup = true;
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
    if(!initLittleFS())
       return false;

    String filetext;
    size_t size = serializeJson(data, filetext);

    writeFile(LittleFS, name, filetext.c_str());
    
    return true;
  }

  JsonDocument pmConfigHandler::LoadConfigFile(const char * name)
  {
    JsonDocument doc;

    if(!initLittleFS())
       return doc;

    String file = readFile(LittleFS, name);
    
    deserializeJson(doc, file);

    return doc;
  }

  pmConfigHandler ConfigHandler;