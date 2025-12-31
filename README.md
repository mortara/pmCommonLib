# pmCommonLib

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Library-orange.svg)](https://platformio.org/lib/show/pmCommonLib)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32](https://img.shields.io/badge/ESP-32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![ESP8266](https://img.shields.io/badge/ESP-8266-blue.svg)](https://www.espressif.com/en/products/socs/esp8266)

A comprehensive, modular library for ESP32 and ESP8266 projects that provides common functionality through a unified interface. This library wraps and integrates several popular ESP libraries to simplify development and reduce boilerplate code in your IoT projects.

pmCommonLib streamlines the development of ESP-based projects by providing ready-to-use modules for web interfaces, MQTT connectivity, OTA updates, WiFi management, configuration storage, and more. Perfect for building smart home devices, IoT sensors, and connected applications.

## ✨ Features

- 🌐 **WiFi Management** - Easy WiFi setup with captive portal for first-time configuration
- 🔄 **OTA Updates** - Over-the-air firmware updates for remote device management
- 📡 **MQTT Integration** - Full MQTT support with Home Assistant auto-discovery
- 🖥️ **Web Interface** - Built-in async web server with settings management
- 💾 **Configuration Storage** - Persistent storage using LittleFS filesystem
- 📊 **Web Serial** - Browser-based serial monitor for remote debugging
- 🔧 **Settings Handler** - Dynamic web-based settings configuration
- 🔌 **I2C Support** - I2C device management with built-in scanner
- 📝 **Logging System** - Comprehensive logging with multiple output options
- 🎯 **Modular Design** - Enable/disable modules as needed to save resources

## 📋 Requirements

### Platform Support
- **ESP32** - All variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- **ESP8266** - All variants (NodeMCU, Wemos D1, etc.)

### Build System
- **PlatformIO** - Recommended (tested and fully supported)
- **Arduino IDE** - Supported with manual library installation

### Framework
- **Arduino Framework** - Required

## 📦 Dependencies

This library integrates the following excellent libraries:

- [**MycilaWebSerial**](https://github.com/mathieucarbou/MycilaWebSerial) - Web-based serial monitor
- [**AsyncTCP**](https://github.com/ESP32Async/AsyncTCP) - Asynchronous TCP library for ESP32
- [**ESPAsyncTCP**](https://github.com/ESP32Async/ESPAsyncTCP) - Asynchronous TCP library for ESP8266
- [**ESPAsyncWebServer**](https://github.com/ESP32Async/ESPAsyncWebServer) - Asynchronous HTTP web server
- [**PubSubClient**](https://github.com/hmueller01/pubsubclient3) - MQTT client library
- [**ArduinoJson**](https://github.com/bblanchon/ArduinoJson) - JSON serialization and deserialization

All dependencies are automatically installed when using PlatformIO.

## 🚀 Installation

### PlatformIO (Recommended)

Add pmCommonLib to your `platformio.ini` file:

```ini
[env:mydevice]
platform = espressif32  ; or espressif8266
board = esp32dev        ; or your specific board
framework = arduino
lib_deps =
    https://github.com/mortara/pmCommonLib.git
```

### Manual Installation

1. Clone this repository into your project's `lib` folder:
```bash
cd your-project/lib
git clone https://github.com/mortara/pmCommonLib.git
```

2. Install the required dependencies manually or let PlatformIO handle them automatically.

## 🎯 Quick Start

### Basic Setup

```cpp
#include <Arduino.h>
#include "pmCommonLib.hpp"

void setup() {
    Serial.begin(115200);
    
    // Initialize the library with all modules enabled
    pmCommonLib.Setup();
    
    // Start all services
    pmCommonLib.Start();
}

void loop() {
    // Handle library tasks
    pmCommonLib.Loop();
}
```

### Advanced Setup with Selective Modules

```cpp
#include <Arduino.h>
#include "pmCommonLib.hpp"

void setup() {
    Serial.begin(115200);
    
    // Initialize with selective modules
    // Setup(mqtt, webserial, ota, general)
    pmCommonLib.Setup(true, true, true, true);
    
    // Configure WiFi credentials
    pmCommonLib.WiFiManager.SetCredentials("YourSSID", "YourPassword");
    
    // Configure MQTT if needed
    #ifndef PMCOMMONNOMQTT
    pmCommonLib.MQTTConnector.SetBroker("mqtt.example.com", "1883");
    #endif
    
    // Start all enabled services
    pmCommonLib.Start();
}

void loop() {
    pmCommonLib.Loop();
}
```

## 📚 Modules Documentation

### Config Handler

Manages persistent configuration storage using the ESP's LittleFS filesystem.

**Features:**
- Save and load JSON configuration files
- LittleFS filesystem initialization and management
- Web-based configuration page registration
- Automatic filesystem formatting on first use

**Usage:**
```cpp
// Access the config handler
pmCommonLib.ConfigHandler.Setup();

// Save configuration
JsonDocument doc;
doc["setting1"] = "value1";
doc["setting2"] = 42;
pmCommonLib.ConfigHandler.SaveConfig("/config.json", doc);

// Load configuration
JsonDocument loadedDoc;
pmCommonLib.ConfigHandler.LoadConfig("/config.json", loadedDoc);
```

**Access:** Available as `pmCommonLib.ConfigHandler`

---

### Settings Handler

Provides a web interface for viewing and editing runtime settings dynamically.

**Features:**
- Dynamic settings registration
- Web-based settings interface
- Support for text and numeric settings
- Persistent settings storage via Config Handler
- RESTful API for settings management

**Usage:**
```cpp
// Register a setting
pmSettingsStruct setting;
setting.Name = "device_name";
setting.Label = "Device Name";
setting.Value = "MyESP32";
setting.Type = pmSettingTypes::TEXTSETTING;
pmCommonLib.Settings.RegisterSetting(setting);

// Get setting value
String deviceName = pmCommonLib.Settings.GetSetting("device_name");

// Update setting
pmCommonLib.Settings.UpdateSetting("device_name", "NewName");
```

**Access:** Available as `pmCommonLib.Settings`

---

### OTA Handler

Wrapper for Arduino OTA (Over-The-Air) updates, enabling remote firmware updates.

**Features:**
- Network-based firmware updates
- Password protection support
- Update progress callbacks
- Automatic mDNS hostname setup
- Error handling and recovery

**Usage:**
```cpp
// OTA is automatically initialized with Setup()
// Access OTA handler for advanced configuration
pmCommonLib.OTAHandler.SetHostname("mydevice");
pmCommonLib.OTAHandler.SetPassword("update123");
```

**Access:** Available as `pmCommonLib.OTAHandler`

---

### MQTT Connector

MQTT client with Home Assistant auto-discovery support.

**Features:**
- MQTT publish/subscribe functionality
- Home Assistant MQTT Discovery integration
- Support for multiple entity types (sensor, switch, select, button, text)
- Automatic reconnection handling
- Message queuing and delivery
- Custom callback support

**Usage:**
```cpp
#ifndef PMCOMMONNOMQTT
// Configure MQTT connection
pmCommonLib.MQTTConnector.SetBroker("192.168.1.100", "1883");
pmCommonLib.MQTTConnector.SetCredentials("mqtt_user", "mqtt_pass");

// Create a Home Assistant sensor
pmCommonLib.MQTTConnector.RegisterSensor(
    "temperature",           // Unique ID
    "Temperature",          // Friendly name
    "°C",                   // Unit of measurement
    "temperature"           // Device class
);

// Publish sensor value
pmCommonLib.MQTTConnector.PublishSensor("temperature", "23.5");

// Subscribe to a topic
pmCommonLib.MQTTConnector.Subscribe("home/commands", [](char* topic, uint8_t* payload, unsigned int length) {
    // Handle incoming message
    String message = String((char*)payload).substring(0, length);
    Serial.println("Received: " + message);
});
#endif
```

**Access:** Available as `pmCommonLib.MQTTConnector` (when MQTT is enabled)

---

### Web Serial

Browser-based serial monitor for remote debugging without physical access.

**Features:**
- Real-time serial output in web browser
- Send commands from web interface
- No USB connection required
- Multiple simultaneous connections
- Integrates with WebServer module

**Usage:**
```cpp
#ifndef PMCOMMONNOWEBSERIAL
// WebSerial is automatically set up
// Send messages to web serial
pmCommonLib.WebSerial.println("Debug message");
pmCommonLib.WebSerial.printf("Value: %d\n", 42);

// Access at: http://<device-ip>/webserial
#endif
```

**Access:** Available as `pmCommonLib.WebSerial` (when WebSerial is enabled)

---

### WiFi Manager

WiFi connection manager with captive portal for easy first-time configuration.

**Features:**
- Automatic connection to saved networks
- Captive portal for initial setup
- Multiple WiFi credential storage
- Connection status monitoring
- Automatic reconnection
- AP mode fallback

**Usage:**
```cpp
// Set WiFi credentials programmatically
pmCommonLib.WiFiManager.SetCredentials("MyNetwork", "MyPassword");

// Start WiFi connection
pmCommonLib.WiFiManager.Begin();

// Check connection status
if (pmCommonLib.WiFiManager.IsConnected()) {
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
}

// Enable captive portal mode
pmCommonLib.WiFiManager.StartCaptivePortal("ESP_Setup");
```

**Access:** Available as `pmCommonLib.WiFiManager`

---

### Web Server

Asynchronous web server wrapper based on ESPAsyncWebServer.

**Features:**
- High-performance async web server
- Easy route registration
- JSON API support
- Static file serving from LittleFS
- CORS support
- WebSocket support

**Usage:**
```cpp
// Register custom routes
pmCommonLib.WebServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["status"] = "online";
    doc["uptime"] = millis();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
});

// Serve static files
pmCommonLib.WebServer.serveStatic("/", LittleFS, "/www/");

// Access server instance for advanced configuration
AsyncWebServer* server = pmCommonLib.WebServer.GetServer();
```

**Access:** Available as `pmCommonLib.WebServer`

---

### I2C Module

I2C device management and scanning utilities.

**Features:**
- I2C bus scanning and device detection
- Device address management
- Read/write helpers
- Multiple I2C bus support (ESP32)
- Error handling

**Usage:**
```cpp
#include "I2C/i2cscanner.hpp"
#include "I2C/i2cdevice.hpp"

// Scan I2C bus for devices
I2CScanner scanner;
scanner.Scan();

// Create I2C device instance
I2CDevice myDevice(0x48);  // Address 0x48
myDevice.begin();

// Read from device
uint8_t data = myDevice.read();

// Write to device
myDevice.write(0x01, 0xFF);
```

---

### Logging System

Comprehensive logging functionality with multiple output targets.

**Features:**
- Multiple log levels (DEBUG, INFO, WARNING, ERROR)
- Output to Serial, WebSerial, or both
- Timestamp support
- Log level filtering
- Printf-style formatting

**Usage:**
```cpp
#include "logging/logging.hpp"

// Log messages at different levels
LOG_INFO("Application started");
LOG_DEBUG("Debug value: %d", someValue);
LOG_WARNING("Warning: Low memory");
LOG_ERROR("Error occurred: %s", errorMsg);
```

## ⚙️ Configuration

### Build Flags

You can disable specific modules using build flags in `platformio.ini`:

```ini
[env:mydevice]
platform = espressif32
board = esp32dev
framework = arduino
build_flags =
    -D VERSION=1.0.0
    -D PMCOMMONNOMQTT        ; Disable MQTT module
    -D PMCOMMONNOWEBSERIAL   ; Disable WebSerial module
lib_deps =
    https://github.com/mortara/pmCommonLib.git
```

### Runtime Configuration Example

```cpp
void setup() {
    Serial.begin(115200);
    
    // Configure before calling Setup()
    pmCommonLib.ConfigHandler.Setup();
    
    // Initialize with selective features
    // Parameters: mqtt, webserial, ota, general
    pmCommonLib.Setup(
        true,   // Enable MQTT
        true,   // Enable WebSerial
        true,   // Enable OTA
        true    // Enable general features
    );
    
    // Additional module-specific configuration
    pmCommonLib.WiFiManager.SetCredentials("SSID", "Password");
    
    #ifndef PMCOMMONNOMQTT
    pmCommonLib.MQTTConnector.SetBroker("192.168.1.100", "1883");
    #endif
    
    pmCommonLib.Start();
}
```

## 📖 Examples

For reference, you can see the library's basic structure in the `src/main.cpp` file (note: this is commented out and serves as a template).

### Code Examples in This Documentation

This README provides comprehensive code examples for all modules:
- **Quick Start** section - Basic and advanced setup patterns
- **Modules Documentation** section - Usage examples for all 9 modules
- **Configuration** section - Build flags and runtime configuration

### Example Project Ideas

You can build various projects with pmCommonLib:
- Basic setup with WiFi and OTA
- MQTT sensor with Home Assistant integration
- Web-based configuration interface
- I2C device scanner and reader
- Complete IoT device template

See the Quick Start and Modules Documentation sections above for working code examples.

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Fork the repository**
   ```bash
   git clone https://github.com/mortara/pmCommonLib.git
   cd pmCommonLib
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow the existing code style
   - Comment your code where necessary
   - Update documentation if needed

4. **Test your changes**
   - Test on both ESP32 and ESP8266 if possible
   - Ensure no breaking changes to existing functionality

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add: description of your changes"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Provide a clear description of the changes
   - Reference any related issues

### Coding Standards
- Use consistent indentation (spaces, not tabs)
- Follow Arduino/ESP naming conventions
- Document public APIs with comments
- Keep functions focused and modular

## 📝 License

This project is licensed under the MIT License - see below for details:

```
MIT License

Copyright (c) 2025 Patrick Mortara

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## 👤 Author

**Patrick Mortara**
- Email: mortara@gmail.com
- Website: [www.mortara.org](https://www.mortara.org)
- GitHub: [@mortara](https://github.com/mortara)

## 🙏 Acknowledgments

This library wouldn't be possible without these excellent projects:

- [MycilaWebSerial](https://github.com/mathieucarbou/MycilaWebSerial) by Mathieu Carbou - Web-based serial communication
- [AsyncTCP](https://github.com/ESP32Async/AsyncTCP) - Asynchronous TCP for ESP32
- [ESPAsyncTCP](https://github.com/ESP32Async/ESPAsyncTCP) - Asynchronous TCP for ESP8266
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) - Powerful async web server
- [PubSubClient](https://github.com/hmueller01/pubsubclient3) by hmueller01 - MQTT functionality
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) by Benoit Blanchon - JSON handling

Special thanks to the ESP32 and ESP8266 communities for their continued support and contributions.

## ⚠️ Known Issues

For a list of known issues and planned features, please visit the [GitHub Issues](https://github.com/mortara/pmCommonLib/issues) page.

Common issues:
- Ensure sufficient flash size for LittleFS (recommended: 4MB+ flash)
- WebSerial may require page refresh on first connection
- MQTT reconnection delay is 5 seconds by default

---

**If you find this library useful, please consider giving it a ⭐ on GitHub!**
