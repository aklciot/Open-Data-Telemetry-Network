# TTGO-Gateway-repeater
## Compiling and loading
This code can be compiled on a standard Arduino GUI.
### Libraries
The following libraries are required:
- AsyncTCP (can be installed fromzip file in library folder)
- ESPAsyncWebServer (can be installed fromzip file in library folder)
- EEPROM (can be installed fromzip file in library folder) (N.B. This library has been depreciated and should be replaced by Preferences. Raised as issue #2)
- Adafruit MQTT
- ESP8266 and ESP32 OLED driver for SSD1306 displays
- RH_RF95.h from RadioHead

## Setup for different gateways
* The node ID, repeater ID etc needs to be set up in `param.h`
This should be configurable by the web interface over WiFi, Issue #1 raised for this feature
* MQTT setting need to be done in the `param.h1` file
This should also be configurable over the web interface - see Issue #3

## Using the gateway
### Gateway mode

### Setup mode
