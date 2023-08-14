//  ******************************************************************************************************
//  ********************************* INNOVATE AUCKLAND, LoRa MQTT Gateway *******************************
//  ******************************************************************************************************
//  LoRa Wifi MQTT gateway.  Includes remote send LoRa test message and system reset functions via MQTT subscription
//  To configure WiFi and switch modes hold button A during startup to enter AP setup mode.
//  Browser connect to http://192.168.4.1/home to enter your wifi settings, or select operation modes:
//  monitor or repeater and set LoRa spreading factor.
//  USB Serial connect at 115200(n,8,1) to view the serial diagnostics.
//
// IMPORTANT: Take care to correctly Configure Hardware Version for your board, lines 144-175.
//
//  Version WD-2.3.5 27-Feb-2020, Added MQTT reconnect to failed publish gateway loop
//  Version WD-2.3.4 Reduced RSSI value in Monitor mode
//  Version WD-2.3.3 Reduced RSSI value in Repeater mode
//  Version WD-2.3.2 Added Iwi mode
//  Version WD-2.3.1 Reordered library load to free some memory
//  Version WD-2.3.0 Printed some dots, cleared some memory
//  Version WD-2.2.9 Moved serial prints to from SVRAM to PROGMEM (F())
//  Version WD-2.2.8 Added MQTT publish to while loop function
//  Version WD-2.2.7 Added clear buffer memory prior to publish
//  Version WD-2.2.6 Added OLED display burn-in saver
//  Version WD-2.2.5 Added new watchdog
//  Version WD-2.2.4 Added Uptime and version to LoRa send test message
//  Version WD-2.2.3 Disable watchdog function until it can be made stable
//  Version WD-2.2.2 Added auto reply message in gateway mode, keyword "TTMon" from a monitor node
//  Version WD-2.2.1 Increased watchdog timeout to 1 minute
//  Version WD-2.2.0 Reconfigured gateway loop to prioritise LoRa receive
//  version WD-2.1.8 Added MQTT failed publish counter and max trys reset
//  version JW-2.1.7 Moved changable parameters to a separate file
//  version AC-2.1.6 Added repeater loop'd message detection. Thanks Aldo!
//  version WD-2.1.5 Enabled watchdog reset to Gateway and Repeater mode.
//  version WD-2.1.4 Added Gateway ID to serial diagnostics.
//  version WD-2.1.3 Added remote config for setup-mode and LoRa spreading factor over MQTT control subscribe.
//  version WD-2.1.1 Modified uptime seconds to minutes.
//  version WD-2.1.0 added serial print WiFi MAC address to start-up.
//  version WD-2.0.9 added support for TTGO T-Beam v1
//  version WD-2.0.8 added channel spreading factor to AP setup mode
//  version WD-2.0.7 added Project ID to gateway status message
//  version WD-2.0.6 added support for TTGO version 1.6
//  version WD-2.0.5 Moved location for lat-long to SRAM
//  version WD-2.0 added SRAM setup mode via WiFi
//  version WD-0.8 Increased connect retries
//  version WD-0.7 Added ESP Watchdog
//  version WD-0.6 Added reset counter during wifi and mqtt connection
//  version WD-0.5 Added ESP.restart(); to mqtt RESET command.
//  version WD-0.4 Added test message button
//  version WD-0.3 Updated serial diagnostics
//  version WD-0.2 Updated serial diagnostics
//  version WD-0.1 Initial build
//  ESP Tutorial - Saving Persistent Variables to NV-SRAM
//  https://www.youtube.com/watch?v=EpfBo6r5hzA
//  ESP Async Server tutorial
//  https://techtutorialsx.com/2017/07/26/esp32-arduino-setting-a-soft-ap/
//  *******************************************************************************************************

/*
The Non-Profit Open Software License version 3.0 (NPOSL-3.0)

Copyright (c) 2019 Innovate Auckland

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

This software may not be used for commercial purposes.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.  


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/



// -- End of File --
