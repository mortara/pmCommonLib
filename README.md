# pmCommonLib

A wrapper for a collection of classes i commonly use with my ESP32/ESP8266 projects:

- https://github.com/mathieucarbou/MycilaWebSerial
- https://github.com/ESP32Async/AsyncTCP
- https://github.com/ESP32Async/ESPAsyncTCP
- https://github.com/ESP32Async/ESPAsyncWebServer
- https://github.com/hmueller01/pubsubclient3
- https://github.com/bblanchon/ArduinoJson

## Usage

Add to your PlatformIO project as usual, then include the main header file:

```
#include "pmCommonLib.hpp"
```

Then call the setup function get the lib prepared
```
pmCommonLib.Setup();
```

And the Begin functino to start-up
```
pmCommonLib.Begin();
```


## Submodules

### config_handler

A lib to load and store data on the ESPs littleFS

### settings_handler

A lib that provides a web-interface to view and edit settings

### ota_handler

A wrapper for ArduinoOTA

### mqtt_connector

A MQTT lib that allows me create homeassistant sensors

### webserial

A wrapper for MycilaWebserial

### wifimanager

wifi connection manager with captive portal

### webserver

A wrapper for ESPAsyncWebserver

### I2C

A I2C device wrapper class
