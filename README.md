<img src="https://github.com/aklciot/TTGO-Gateway-repeater/blob/master/InnovateAuckland_Medium.png" align="middle" height="75"/>

# TTGO Multi Mode LoRa Gateway-Repeater
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

## Getting the Gateway Hardware
The gateway software runs on three variants of TTGO LoRa Wifi ESP32 device that is compatible with Arduino ESP32.
<br>NOTE:  Ensure you select the correct frequency, (915Mhz version for New Zealand use) or region.
Edit the gateway code to match the hardware gateway device you have.

#### Type One; LILYGO TTGO 915MHz SX1276 ESP32 LoRa 0.96
2Pcs LILYGO TTGO 915MHz SX1276 ESP32 LoRa 0.96 Inch Blue OLED Display bluetooth WIFI Module IOT Development Board
https://www.banggood.com/2Pcs-LILYGO-TTGO-915MHz-SX1276-ESP32-LoRa-0_96-Inch-Blue-OLED-Display-bluetooth-WIFI-Module-IOT-Development-Board-p-1545115.html?rmmds=search&cur_warehouse=CN
<br>
#### Type Two;  TTGO T-Beam; TTGO LoRa32 V2.1 ESP32LILYGO® 
TTGO LoRa32 V2.1 ESP32 433MHz/868MHz/915MHz OLED 0.96 Inch SD Card bluetooth WIFI Wireless Module SMA IP5306 - 915MHZ
https://www.banggood.com/LILYGO-TTGO-LoRa32-V2_1-ESP32-433MHz868MHz915MHz-OLED-0_96-Inch-SD-Card-bluetooth-WIFI-Wireless-Module-SMA-IP5306-p-1545144.html?rmmds=search&ID=510803&cur_warehouse=CN
<br>
#### Type three;  TTGO T-Beam
TTGO T-Beam ESP32 433/868/915/923Mhz WiFi Wireless Bluetooth Module ESP 32 GPS NEO-6M SMA 18650 Battery Holder with OLED (915MHZ)<br>
https://www.amazon.com/TTGO-T-Beam-Wireless-Bluetooth-Battery/dp/B07WVWCMKZ

## Gateway Modes of operation:

The gateway sends data from the sensor to the internet.  To make this work the gateway needs to know your WiFi name and password to make a connection. The gateway uses very little data, about 50MB per month, as much as listening to a few music tracks.  It also contains no personal data, has no microphone or camera and is hack resistant. This is because the gateway has a very low attack surface, it does not listen for any internet traffic (its’ all outbound only).

### Setup Mode:
 This mode allows the end user to enter the WiFi connection credentials and select the mode of operation.

### Gateway Mode:
 This is the main operation mode.  Gateway mode receives sensor data over LoRa and sends it over the internet to the data processing service.

### Repeater Mode:
This mode converts the gateway in to a LoRa to LoRa signal repeater that extends the range of the network and allows sensors to be put in locations where the gateway cannot otherwise make a connection.

### Monitor Mode:
The signal monitor mode primary function is during network setup. The monitor provides unique features to assist in location decisions including signal strength and auto reply functions. This mode also provides local only (iwi) mode, this is an important feature identified by Mana Whenua ensures data remains in the local area.


## Change log
- Version WD-2.3.5, 27-Feb-2020, Added MQTT reconnect to failed publish gateway loop
- Version WD-2.3.4 Reduced RSSI value in Monitor mode
- Version WD-2.3.3 Reduced RSSI value in Repeater mode- Version WD-2.3.2 Added Iwi mode
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
