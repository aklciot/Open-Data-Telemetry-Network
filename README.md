<img src="https://github.com/aklciot/TTGO-Gateway-repeater/blob/master/InnovateAuckland_Medium.png" align="middle" height="75"/>

# Multi Mode LoRa Gateway-Repeater
## Arduino ESP32 based LoRa to Wifi MQTT Gateway

INNOVATE AUCKLAND
Copyright (c) 2019 Innovate Auckland. All rights reserved.
Included libraries are copyright to their respective owners
   
## Functions and features.
This gateway is designed to create a telemetry connection for situations where standard 2.4 Ghz WiFi and mains power is available. 
<br>Please contact us if you require the solar powered version or don’t have access to a suitable Wifi connection.
<li>Receives open LoRa massages and publishes as comma delimited text to MQTT over a standard Wifi connection
<li>Includes and embedded webserver and application for simple setup using mobile or PC
<li>Three main modes of operation, normal gateway mode, repeater mode to extend range and monitor for network testing and setup. 
<li>Includes built in reset watchdog
<li>Detects and automatically reconnects on connection failure 
<li>Regularly sends the gateway status including name, type, version, uptime, battery level over MQTT using the keyword GWSTATUS.
<li>Supports onsite network testing/setup with automatic gateway status replies over LoRa and MQTT triggered by a gateway message sent in monitor mode. Note this function requires a multimode gateway in monitor mode.

## Contents:
This page describes the Innovate Auckland Telemetry Gateway. On this page you will find details on where to buy supported hardware, details on the gateway operation, source code and the operating manual for setup and advanced features.

The gateway sends data from the sensor to the internet. To make this work the gateway needs to know your WIFI name and password to make a connection. The gateway uses very little data, about 20MB per month, as much as listening to a few music tracks. It also contains no personal data and has no microphone or camera. The gateway does not accept any inbound connections making it very resistant to hacking, all internet connections are outbound only. Power utilisation is about 150mA, less than 1 watt (1/10th of a standard LED light bulb).

## Important:  This opensource software is free for non-commercial use only.
Please review and respect the licences for included libraires.
For any commercial applications, please contact us.


## Compiling and loading
This code can be compiled on a standard Arduino GUI. Your IDE will need the ESP32 boards added.  See the Arduino support pages for details on how to complete this step.

UPDATE:  Sample code for a simple LoRa node that works with this gateway is now included

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

There are four operation modes.

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
