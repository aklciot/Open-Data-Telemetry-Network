<img src="https://github.com/aklciot/TTGO-Gateway-repeater/blob/master/InnovateAuckland_Medium.png" width="200" height="200" />
# TTGO-Gateway-repeater
## Compiling and loading
This code can be compiled on a standard Arduino GUI.
### Libraries
The following libraries are required:
- AsyncTCP (can be installed fromzip file in library folder)
- ESPAsyncWebServer (can be installed from zip file in library folder)
- EEPROM (can be installed from zip file in library folder) (N.B. This library has been depreciated and should be replaced by Preferences. Raised as [issue #2](https://github.com/aklciot/TTGO-Gateway-repeater/issues/2))
- Adafruit MQTT
- ESP8266 and ESP32 OLED driver for SSD1306 displays
- RH_RF95.h from RadioHead

## Setup for different gateways
* The node ID, repeater ID etc needs to be set up in `param.h`
This should be configurable by the web interface over WiFi, [Issue #1](https://github.com/aklciot/TTGO-Gateway-repeater/issues/1) raised for this feature
* MQTT setting need to be done in the `param.h1` file
This should also be configurable over the web interface - see [Issue #3](https://github.com/aklciot/TTGO-Gateway-repeater/issues/3)

## Using the gateway
### Gateway mode

### Setup mode

## Change log
- Version WD-2.3.2 Added Iwi mode
- Version WD-2.3.1 Reordered library load to free some memory
- Version WD-2.3.0 Printed some dots, cleared some memory
- Version WD-2.2.9 Moved serial prints to from SVRAM to PROGMEM (F())
- Version WD-2.2.8 Added MQTT publish to while loop function
- Version WD-2.2.7 Added clear buffer memory prior to publish
- Version WD-2.2.6 Added OLED display burn-in saver
- Version WD-2.2.5 Added new watchdog
- Version WD-2.2.4 Added Uptime and - Version to LoRa send test message
- Version WD-2.2.3 Disable watchdog function until it can be made stable
- Version WD-2.2.2 Added auto reply message in gateway mode, keyword "TTMon" from a monitor node
- Version WD-2.2.1 Increased watchdog timeout to 1 minute
- Version WD-2.2.0 Reconfigured gateway loop to prioritise LoRa receive
- Version WD-2.1.8 Added MQTT failed publish counter and max trys reset
- Version JW-2.1.7 Moved changable parameters to a separate file
- Version AC-2.1.6 Added repeater loop'd message detection. Thanks Aldo!
- Version WD-2.1.5 Enabled watchdog reset to Gateway and Repeater mode.
- Version WD-2.1.4 Added Gateway ID to serial diagnostics.
- Version WD-2.1.3 Added remote config for setup-mode and LoRa spreading factor over MQTT control subscribe.
- Version WD-2.1.1 Modified uptime seconds to minutes.
- Version WD-2.1.0 added serial print WiFi MAC address to start-up.
- Version WD-2.0.9 added support for TTGO T-Beam v1
- Version WD-2.0.8 added channel spreading factor to AP setup mode
- Version WD-2.0.7 added Project ID to gateway status message
- Version WD-2.0.6 added support for TTGO - Version 1.6
- Version WD-2.0.5 Moved location for lat-long to SRAM
- Version WD-2.0 added SRAM setup mode via WiFi
- Version WD-1.8 Removed reply message
- Version WD-1.7 Added reset counter watchdog during wifi and mqtt connection
- Version WD-1.4 Disabled watchdog during connection to aviod watchdog loop lockout
- Version WD-1.3 Added power fail status
- Version WD-1.2 Added non blocking 5 min delay for status
- Version WD-1.1 Added location to health status message
- Version WD-1.0 Added health status queue
- Version WD-0.9 Updated health status message to include uptime
- Version WD-0.8 Added gateway health message publish
- Version WD-0.7 Added gateway location message values
- Version WD-0.6 Added RSSI message value
- Version WD-0.5 Added reset counter during wifi and mqtt connection
- Version WD-0.4 Added ESP.restart(); to mqtt RESET command.
- Version WD-0.3 Added test message button
- Version WD-0.2 Updated serial diagnostics
- Version WD-0.1 Initial build
