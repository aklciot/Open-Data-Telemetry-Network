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



// Paramaters that are likely to change are stored in a separate file.
// This source code file should not change between rebuilds
#include "param.h"
int RPDelay = 2000;  // Delay to avoid TX message collision. MIN 2 SEC, MAX 6. Use a different a delay time for chained repeaters (min 2secs)

// *************************************************************************************************//
// ********************************** Do not edit below this line **********************************//
// *************************************************************************************************//

#define Version "2.3.5"


// ******************** Required Libraries **********************//
#include <RH_RF95.h>      // LoRa radio, 100k
#include "SSD1306.h"      // OLED Display
#include "EEPROM.h"
#include <AsyncTCP.h>     //40k
#include "ESPAsyncWebServer.h"  // 30k

#include <WiFi.h>         // esp32 Wifi driver

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"  // 30k
#include "esp_system.h"


// *************** Flash Variables class definition *******************//
// this class holds variable and functions to store data in eeprom
class FLASHvariables
{
    // variables:
  public:
    char ProjectName[50];
    char ssid[25];
    char password[20];
    char Latstr[12]; //Latitude
    char Lonstr[12]; //Longitude
    uint8_t varsGood;
    uint8_t varsSF; //Spreading Factor
    uint8_t varsWEP;

    //methods
    void save();
    void get();
    void initialise();
} myVars;  // this is the instantiated object and name





/******************************** Setup Board Connections **************************************/
//  #define SCK     5    // GPIO5  -- SX1278's SCK
//  #define MISO    19   // GPIO19 -- SX1278's MISO
//  #define MOSI    27   // GPIO27 -- SX1278's MOSI
//  #define SS      18   // GPIO18 -- SX1278's CS
//  #define RST     14   // GPIO14 -- SX1278's RESET
//  #define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
//  #define BAND    868E6



/****************************** Configure Hardware Version *******************************/
// TTGO version 1.0
/*
  #define RFM95_RST 14
  #define RFM95_CS  18
  #define RFM95_INT 26
  #define LED_BUILTIN 2
  #define BUTTON_A 0                // Setup AP initialise button
  #define Type "TTV1"
  SSD1306 display(0x3c, 4, 15);     // TTGO VER 1.0 // OLED Display SDA = pin 4, SCL = pin 15, RESET = pin 16
*/
/*
  //TTGO Version 1.6
  #define RFM95_RST     23
  #define RFM95_CS      18
  #define RFM95_INT     26
  #define LED_BUILTIN   25            //TTGO V1:2, TTGO V2:25, TBeam:14
  #define BUTTON_A      2             //setup AP initialise button
  #define Type "TTV2"
  SSD1306 display(0x3c, 21, 22);      //TTGO VER 2.1.6 and TBeam OLED Display SDA = pin 21, SCL = pin 22, RESET = pin 16
*/

//TTGO TBeam Version 1.0
#define RFM95_RST 23
#define RFM95_CS  18
#define RFM95_INT 26
#define LED_BUILTIN 14            //TTGO V1:2, TTGO V2:25, TBeam:14
#define BUTTON_A 39               //setup AP initialise button
#define Type "TTV3"
SSD1306 display(0x3c, 21, 22);    //TTGO VER 2.1.6 and TBeam OLED Display SDA = pin 21, SCL = pin 22, RESET = pin 16

/*******************************************************************************************/


#define RF95_FREQ 915.          // Set LoRa radio frequency


/****************************** Load drivers ***************************************/

// wd load singleton instance of the radio driver and called it "rf95"
RH_RF95 rf95(RFM95_CS, RFM95_INT);

WiFiClient client;

// wd load mqtt driver called mqtt
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

/***************************** MQTT Queue Config **********************************/

// Notice MQTT paths for Adafruit AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish pubmessage = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC "/Gateway/"GWID);
Adafruit_MQTT_Publish pubstatus = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC "/Status/"GWID);
Adafruit_MQTT_Subscribe control = Adafruit_MQTT_Subscribe(&mqtt, MQTT_TOPIC "/Control/"GWID);



/**************************** SETUP SECTION ***************************/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(200);

  // digitalWrite(BUTTON_A, LOW);      // set Button A low
  pinMode(BUTTON_A, INPUT_PULLUP);

  // Initialising the the OLED display
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);      // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);     // while OLED is running, must set GPIO16 in high、
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // get SRAM data
  EEPROM.begin(sizeof(myVars));
  EEPROM.get(0, myVars);
  delay(1000);
  Serial.println();
  Serial.println(F("<<<--------------- INNOVATE AUCKLAND ----------------->>>"));
  Serial.println(F("                   LoRa MQTT Gateway                     "));
  Serial.println();
  Serial.print(F("Gateway name = ")); Serial.print(GWID);
  Serial.print(F(", Version: ")); Serial.println(Version);
  Serial.print(F("Project name = ")); Serial.println(myVars.ProjectName);
  Serial.print(F("Wifi Name = ")); Serial.println(myVars.ssid);
  Serial.print(F("Wifi password = ")); Serial.println(myVars.password);
  Serial.print(F("varsGood value = ")); Serial.println(myVars.varsGood);
  Serial.print(F("WEP value = ")); Serial.println(myVars.varsWEP);
  Serial.print(F("Spreading Factor value = ")); Serial.println(myVars.varsSF);
  Serial.print(F("LoRa TX delay: ")); Serial.print(RPDelay); Serial.println(F(" mSec"));
  Serial.print(F("WiFi.macAddress: ")); Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.println(F("<<<-------------------------------------------------->>>"));
  delay(2000);
  Serial.println();
  display.display();
  display.clear();
  display.drawString(5, 25, "INNOVATE AUCKLAND");
  display.display();
  delay(4000);
  display.clear();
  display.drawString(0, 0, "LoRa MQTT Gateway");
  display.drawString(0, 10, "Gateway ID: " GWID);
  display.drawString(0, 20, "Version: "Version);
  display.display();
  delay(4000);
  display.drawString(0, 35, "STARTING..");
  Serial.println(F("STARTING..  "));
  display.display();
  delay(2000);

  // wd Set SRAM varsGood value to 180 trigger APSetup mode on reboot
  if (! digitalRead(BUTTON_A))
  {
    Serial.println(F("Button A:  "));
    Serial.println(F("Setting AP Mode true  "));
    myVars.varsGood = 180;
    delay(1000);
    myVars.save();
    display.clear();
    display.drawString(15, 25, "AP MODE");
    //delay(1000);
    //display.drawString(0, 35, "Restarting...");
    display.display();
    delay(3000);
    APSetup();
    //ESP.restart();
  }

  // manual radio reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // wd initialise radio
  while (!rf95.init()) {
    Serial.println(F("LoRa radio init failed"));
    while (1);
  }

  Serial.println();
  Serial.println();
  Serial.println(F("********************************"));
  if (myVars.varsSF == 200)
  {
    //wd config for long range. Put just after radio init
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: LowDataRate=On, Agc=On
    };
    rf95.setModemRegisters(&modem_config);
    Serial.println(F("* LoRa radio set to long range *"));
  }
  else {
    Serial.println(F("* LoRa radio set to default *"));
  }
  Serial.println(F("********************************"));

  Serial.println();
  Serial.println(F("LoRa radio init OK!"));

  // Set radio frequency and power level. Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println(F("setFrequency failed"));
    while (1);
  }
  Serial.print(F("Set Freq to: "));
  Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);

  display.clear();
  display.display();
  Serial.println(F("Finished Setup"));
  Serial.println();
  // wd start Gateway Mode or
  if (myVars.varsGood == 190)
  {
    mqtt.subscribe(&control); // wd mqtt subsribe topic
    Serial.println(F("Good eeprom variables found, starting Gateway mode... "));
    display.clear();
    display.drawString(25, 25, "Gateway Mode");
    display.display();
    delay(3000);
    GWConnect();
    delay(1000);
    gatewayStatus();
    delay(1000);
    Serial.println(F("Starting Gateway"));
    Serial.println();
    Gateway();
  }

  // wd start LoRa Monitor Mode
  if (myVars.varsGood == 200)
  {
    Serial.println(F("Good eeprom variables found, starting Monitor mode... "));
    display.clear();
    display.drawString(25, 25, "Monitor Mode");
    display.display();
    delay(3000);
    Monitor();
  }
  // wd start LoRa Repeater Mode
  if (myVars.varsGood == 250)
  {
    Serial.println(F("Good eeprom variables found, starting Repeater mode... "));
    display.clear();
    display.drawString(25, 25, "Repeater Mode");
    display.display();
    delay(3000);
    Repeater();
  }
  else
    // wd start APSetup Mode
  {
    Serial.println(F("Starting AP mode... "));
    display.clear();
    display.drawString(25, 25, "Setup Mode");
    display.display();
    delay(3000);
    APSetup();
  }
}


// ****** wd variables told hold gateway values *********
char Gbuffer[100];
char* autoReply = "TTMon";             // Monitor Keyword for auto reply
char* Statstr = "OK";                  // holds status string value used to indicate power fail if implemented
char Uptime[6];                        // holds uptime string value
char Rstr[4];                          // holds RSSI string value
int RS;                                // holds RSSI value
double Mills;                          // double for uptime value
int period = 605000;                   // 10min delay for status message publish
unsigned long time_now = 0;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];  // keep outside of loop to avoid klingon
byte sendLen;
int value = 0;

// ****** wd variables told hold repeater values *********
char Rbuffer[50];
char Bstr[4];           // Battery Voltage
char* Found;            // Checks for this repeater ID to avoid endless loops and monitor message keyword.

int counter1 = 0; // Wifi connect reset counter 1
int counter2 = 0; // MQTT connect reset counter 2
int counter3 = 0; // MQTT Publish reset counter 3


int keyIndex = 0;             // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
char Sbuffer[50];             // Buffer for LoRa send test message


/******************************* ESP Watchdog ***********************************/

const int wdtTimeout = 60000;  //time in ms to trigger the watchdog, 1 minute.
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("Watchdog triggered, reboot\n");
  esp_restart();
}




/************************** Main Gateway Loop ******************************/
void Gateway() {

  // Enable watchdog
  Serial.println();
  Serial.println(F("Enable Watchdog"));
  Serial.println(F("Starting Gateway listen"));
  Serial.println();
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, true);  //set time in us, reboot true
  timerAlarmEnable(timer);

  int i = 0; //used for oled saver

  while (myVars.varsGood == 190) {
    //Serial.println();
    timerWrite(timer, 0); //reset timer (feed watchdog)
    delay(10);
    //Serial.println(F("Reset Watchdog"));
    //Serial.println();

    // This section prevents OLED display burn when running for long periods
    i++;
    if (i <= 40) {
      display.clear();
      display.drawString(0, 45, "Ready...");
      display.display();
    }
    if (i > 41 && i <= 80) {
      display.clear();
      display.drawString(40, 45, "Ready...");
      display.display();
    }
    if (i > 81 && i <= 120) {
      display.clear();
      display.drawString(80, 45, "Ready...");
      display.display();
    }
    if (i > 120) {
      display.clear();
      display.display();
      i = 0;
    }

    // This is the LoRa Radio receive section
    // Serial.println(F("1. Check radio is available"));
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      Serial.println();
      Serial.println(F("2. Check for LoRa message"));
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println();
        Serial.print(F("3a. LoRa data received: "));
        Serial.println((char*)buf);
        Serial.print(F("Length: "));  Serial.println(len);
        Serial.println();
        display.clear();
        display.drawString(0, 0, "LoRa Message:");
        display.drawString(0, 15, (char*)buf);
        display.display();

        Found = strstr((char*)buf, autoReply); // check for message from a monitor node and auto reply

        if (Found != NULL) {
          Serial.println(F("Monitor message received, sending auto reply"));
          Serial.println();
          display.clear();
          display.drawString(0, 0, "Monitor Message:");
          display.drawString(0, 15, (char*)Rbuffer);
          display.drawString(0, 35, "RSSI:");
          display.drawString(50 , 35 , Rstr);
          display.display();
          Serial.println(F("5. Send Gateway status"));
          Serial.print(F("TX delay: ")); Serial.print(RPDelay); Serial.println(F(" mSec"));
          // Send buffer and send length to radio
          delay(RPDelay);       // delay to avoid message collision
          sendTest();           // Send LoRa status
          delay(100);
          gatewayStatus();      // Send MQTT status
          Found = '\0'; //clear "Found" char value
        }

        // Create MQTT Message...
        memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
        delay(10);
        RS = rf95.lastRssi();
        dtostrf(RS, 1, 0, Rstr);
        sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s", GWID, buf, Rstr, myVars.Latstr, myVars.Lonstr, myVars.ProjectName);

        // Publish MQTT Message...
        Serial.println(F("3c. Start Publish. "));

        Serial.print(F("Publishing MQTT Message: ")); Serial.println(Gbuffer);
        Serial.print(F("Check MQTT Connection. "));
        GWConnect();
        Serial.println();
        while (! pubmessage.publish(Gbuffer)) {
          Serial.println(F("Publish failed, retry in 5secs"));
          timerWrite(timer, 0); //reset timer (feed watchdog)
          delay(10);
          Serial.println(F("MQTT Disconnect"));
          mqtt.disconnect();
          delay(5000);  // wait 2 seconds
          Serial.println(F("MQTT Reconnect"));
          mqtt.connect();
          counter3++;
          Serial.print(F("MQTT Publish Counter: ")); Serial.println(counter3);
          if (counter3 == 5) {
            Serial.println(F("MQTT Publish Fails. Restarting!"));
            Serial.println();
            display.drawString(0, 15, "Max Publish Fail");
            delay(3000);  // wait 2 seconds and reset
            display.drawString(0, 25, "Restarting");
            display.display();
            delay(1000);
            ESP.restart();
          }
        }
        memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
        delay(10);
        Serial.println(F("Publish Message OK! "));
        counter3 = 0;
        display.drawString(0, 35, "Publish OK");
        display.display();
        delay(1500);
        Serial.println();

        digitalWrite(LED_BUILTIN, LOW);
      }
      else
      {
        Serial.println(F("recv failed"));
      }
    }

    // check for messages on the mqtt subscribe topic

    //Serial.println(F("4. Check for MQTT subscripton messages"));
    mqttSubscribe();
    //Serial.println();


    // Send Gateway status message
    if ((! digitalRead(BUTTON_A)) || (millis() > time_now + period))
    {
      time_now = millis();
      Serial.println(F("5. Send Gateway status"));
      sendTest();       // Send LoRa status
      delay(100);
      gatewayStatus();  // Send MQTT status
    }
    // Print out some dots...
    uint8_t dot;
    dot++;
    //Serial.print(F("."));
    if (dot == 80) {
      // Serial.println(F("."));
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      dot = 0;
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}


/********************** Connect Section *******************************/

void GWConnect()
{
  // MQTT Connection section
  int8_t ret;
  if (mqtt.connected()) {
    //Serial.println(F("Connection OK."));
    delay(10);
    return;
  }
  else {
    // Wifi Connection section
    Serial.println(F("Not connected, connecting."));
    while (WiFi.status() != WL_CONNECTED) {
      counter1++;
      Serial.print(F("WiFi Connect Counter: "));
      Serial.println(counter1);
      if (counter1 == 10) {
        counter1 = 0;
        Serial.println(F("Maximum connect attempts, resetting"));
        display.clear();
        display.drawString(0 , 0 , "Wifi Connect Error");
        display.drawString(0 , 15 , "Resetting");
        display.display();
        delay(10000);
        ESP.restart();
        delay(1000);
      }
      Serial.print(F("Attempting to connect to SSID: "));
      Serial.println(myVars.ssid);
      display.clear();
      display.drawString(0 , 0 , "Connecting to:");
      display.drawString(0 , 15 , myVars.ssid);
      display.display();

      // Connect to WPA/WPA2 network. Remove pass value if using open or WEP network:
      if (myVars.varsWEP != 1) {
        status = WiFi.begin(myVars.ssid, myVars.password);
      }
      else {
        status = WiFi.begin(myVars.ssid); //WEP mode, no password
      }
      uint8_t timeout = 5;      // wait 5 seconds for connection:
      while (timeout && (WiFi.status() != WL_CONNECTED)) {
        timeout--;
        delay(500);
        Serial.print(F("."));
      }
    }
    Serial.println();
    Serial.println(F("WiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    Serial.println();

    // MQTT Connection section
    Serial.print(F("Connecting to MQTT... "));
    Serial.println(MQTT_SERVER);
    display.clear();
    display.drawString(0 , 0 , "Connecting to MQTT... ");
    display.drawString(0 , 15 , MQTT_SERVER);
    display.display();

    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      if (counter2 == 5) {
        counter2 = 0;
        Serial.println(F("MQTT connect error, resetting"));
        display.clear();
        display.drawString(0 , 0 , "MQTT Connect Error");
        display.drawString(0 , 15 , "Resetting");
        display.display();
        delay(3000);
        //digitalWrite(resetPin, LOW);
        ESP.restart();
        delay(1000);
      }
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println(F("Retrying MQTT connection in 4 seconds..."));
      mqtt.disconnect();
      delay(4000);  // wait 4 seconds
      counter2++;
      Serial.print(F("MQTT Counter: ")); Serial.println(counter2);
    }
    Serial.println(F("MQTT Connected!"));
    Serial.println();
    display.clear();
    display.drawString(15, 15, "MQTT Connected!");
    display.display();
    delay(2000);
  }
}


/************************** Start of Gateway Status Function ******************************/
void gatewayStatus() {
  Serial.println();
  memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
  delay(10);
  Serial.println(F("Publish Gateway Status: "));
  Mills = (millis() / 1000) / 60;
  dtostrf(Mills, 1, 0, Uptime);
  sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s,%s,%s", GWID, Statstr, Version, Type, Uptime, myVars.Latstr, myVars.Lonstr, myVars.ProjectName);
  //sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s", GWID, Statstr, Version, Type, Uptime, myVars.ProjectName);
  //Serial.println();
  Serial.print(F("Publishing MQTT Gateway Status Message: ")); Serial.println(Gbuffer);
  Serial.print(F("Check MQTT Connection. "));
  GWConnect();
  Serial.println();
  while (! pubstatus.publish(Gbuffer)) {
    Serial.println(F("Publish failed, retry in 5secs"));
    timerWrite(timer, 0); //reset timer (feed watchdog)
    delay(5000);  // wait 2 seconds
    counter3++;
    Serial.print(F("MQTT Publish Counter: ")); Serial.println(counter3);
    if (counter3 == 5) {
      Serial.println(F("MQTT Publish Fails. Restarting!"));
      Serial.println();
      display.drawString(0, 15, "Max Publish Fail");
      delay(3000);  // wait 2 seconds and reset
      display.drawString(0, 25, "Restarting");
      display.display();
      delay(1000);
      ESP.restart();
    }
  }
  memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
  delay(10);
  Serial.println(F("Publish Message OK! "));
  counter3 = 0;
  display.drawString(0, 35, "Publish OK");
  display.display();
  delay(1500);
  Serial.println();
  display.clear();
  display.drawString(0, 15, "Publish Status");
  display.drawString(0, 25, GWID);
  display.display();
  delay(2000);
}


/******************************* MQTT Publish Section *******************************/
/*
  void mqttPublish() {
  Serial.print(F("Publishing MQTT Message: ")); Serial.println(Gbuffer);
  Serial.print(F("Check MQTT Connection. "));
  GWConnect();
  Serial.println();
  while (! pubmessage.publish(Gbuffer)) {
    Serial.println(F("Publish failed, retry in 5secs"));
    timerWrite(timer, 0); //reset timer (feed watchdog)
    delay(10);
    delay(5000);  // wait 2 seconds
    counter3++;
    Serial.print(F("MQTT Publish Counter: ")); Serial.println(counter3);
    if (counter3 == 5) {
      Serial.println(F("MQTT Publish Fails. Restarting!"));
      Serial.println();
      display.drawString(0, 15, "Max Publish Fail");
      delay(3000);  // wait 2 seconds and reset
      display.drawString(0, 25, "Restarting");
      display.display();
      delay(1000);
      ESP.restart();
    }
  }
  memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
  delay(10);
  Serial.println(F("Publish Message OK! "));
  counter3 = 0;
  display.drawString(0, 35, "Publish OK");
  display.display();
  delay(1500);
  Serial.println();
  }
*/






/********************** AP Setup Webserver Section *******************************/

String WiFiName;
String WiFiPassword;
String ProjectName;
String LatStr;
String LongStr;
String GuestWiFi;


// HTML Content
String HTMLHead = "<!DOCTYPE html><html><head>";
String HTMLStyle_1 = "<style>body {background-color: #131c41;width: 85%;height: 85%; color: #ffffff; font-family: Arial, Helvetica, sans-serif; padding: 7px;} div{font-weight: normal;font-size: 18px; color: #ffffff; font-family: Arial, Helvetica, sans-serif; padding: 7px;}";
String HTMLStyle_2 = "a:link, a:visited {font-weight: normal;font-size: 12px;color: #ffffff;padding: 14px 25px;text-align: center;text-decoration: none;display: inline-block;}a:hover, a:active {background-color: #ff6f00;color: #000000;}";
String HTMLStyle_3 = "form{border: 2px solid #5d699e;padding: 12px 20px;}input[type=text] {width: 100%;padding: 12px 20px;margin: 8px 0;box-sizing: border-box;} p{font-weight: normal;font-size: 12px;color: #cccccc;}</style></head><body>";
String HTMLText_1 = "<h1>LoRa Gateway Setup</h1><h3>Enter your wifi name, WiFi password and project name.</h3><br>IMPORTANT: Your data is identified by the project name. You must use the same project name for all gateways to access your sensor data.<br>";
String HTMLForm_1 = "<div class='input'><form action='update' method='get' ><br>Wifi Name:<br><input type='text' name='WiFi_Name'><br>Wifi Password:<br><input type='text' name='WiFi_Password'><br>Project Name:<br><input type='text' name='Project_Name'><br><input type='checkbox' name='Guest_WiFi' value='1'>Use Guest Wifi, no password<br><input type='submit' value='Submit'></form></div>";
String HTMLForm_2 = "<div class='input'><form action='update' method='get' ><br>Wifi Name:<br><input type='text' name='WiFi_Name'><br>Wifi Password:<br><input type='text' name='WiFi_Password'><br>Project Name:<br><input type='text' name='Project_Name'><br>Gateway Location:<br>Lattitude:<br><input type='text' name='Lat_Str' value='-36.83'><br><br>Longitude:<br><input type='text' name='Long_Str' value='174.83'><br><input type='checkbox' name='Guest_WiFi' value='1'>Use Guest Wifi, no password<br><input type='submit' value='Submit'></form></div>";
String HTMLNav = "<p><a href='/monitor'>MONITOR MODE</a>|<a href='/repeater'>REPEATER MODE</a>|<a href='/SetSF'>ADVANCED</a>|<a href='/save'>EXIT SETUP</a>";
String HTMLFoot = "<br><br><p>Gateway Version: 2.3.2, Copyright (c) Innovate Auckland<br>W.Davies 2019</p></body></html>";

String HTMLHome = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + HTMLText_1 + HTMLForm_2 + HTMLNav + HTMLFoot);

AsyncWebServer server(80);
void APSetup()
{
  //*** Variables for APSetup mode and GET Parameters ******
  /*
    String WiFiName;
    String WiFiPassword;
    String ProjectName;
    String LatStr;
    String LongStr;
    String GuestWiFi;
    // HTML Content
    String HTMLHead = "<!DOCTYPE html><html><head>";
    String HTMLStyle_1 = "<style>body {background-color: #131c41;width: 85%;height: 85%; color: #ffffff; font-family: Arial, Helvetica, sans-serif; padding: 7px;} div{font-weight: normal;font-size: 18px; color: #ffffff; font-family: Arial, Helvetica, sans-serif; padding: 7px;}";
    String HTMLStyle_2 = "a:link, a:visited {font-weight: normal;font-size: 12px;color: #ffffff;padding: 14px 25px;text-align: center;text-decoration: none;display: inline-block;}a:hover, a:active {background-color: #ff6f00;color: #000000;}";
    String HTMLStyle_3 = "form{border: 2px solid #5d699e;padding: 12px 20px;}input[type=text] {width: 100%;padding: 12px 20px;margin: 8px 0;box-sizing: border-box;} p{font-weight: normal;font-size: 12px;color: #cccccc;}</style></head><body>";
    String HTMLText_1 = "<h1>LoRa Gateway Setup</h1><h3>Enter your wifi name, WiFi password and project name.</h3><br>IMPORTANT: Your data is identified by the project name. You must use the same project name for all gateways to access your sensor data.<br>";
    String HTMLForm_1 = "<div class='input'><form action='update' method='get' ><br>Wifi Name:<br><input type='text' name='WiFi_Name'><br>Wifi Password:<br><input type='text' name='WiFi_Password'><br>Project Name:<br><input type='text' name='Project_Name'><br><input type='checkbox' name='Guest_WiFi' value='1'>Use Guest Wifi, no password<br><input type='submit' value='Submit'></form></div>";
    String HTMLForm_2 = "<div class='input'><form action='update' method='get' ><br>Wifi Name:<br><input type='text' name='WiFi_Name'><br>Wifi Password:<br><input type='text' name='WiFi_Password'><br>Project Name:<br><input type='text' name='Project_Name'><br>Gateway Location:<br>Lattitude:<br><input type='text' name='Lat_Str' value='-36.83'><br><br>Longitude:<br><input type='text' name='Long_Str' value='174.83'><br><input type='checkbox' name='Guest_WiFi' value='1'>Use Guest Wifi, no password<br><input type='submit' value='Submit'></form></div>";
    String HTMLNav = "<p><a href='/monitor'>MONITOR MODE</a>|<a href='/repeater'>REPEATER MODE</a>|<a href='/SetSF'>ADVANCED</a>|<a href='/save'>EXIT SETUP</a>";
    String HTMLFoot = "<br><br><p>Gateway Version: 2.3.0, Copyright (c) Innovate Auckland<br>W.Davies 2019</p></body></html>";

    String HTMLHome = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + HTMLText_1 + HTMLForm_2 + HTMLNav + HTMLFoot);

  */

  const char *APssid = "GWSetup_1";
  const char *APpassword = "testpassword"; //not used
  display.clear();
  display.drawString(25, 5, "SETUP MODE");
  display.drawString(0, 30, "Wifi connect to:");
  display.drawString(0, 45, APssid);
  display.display();
  WiFi.softAP(APssid); // no password
  Serial.println();
  Serial.println(F("AP setup mode"));
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());

  //wd serve the home page
  server.on("/home", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", HTMLHome);
  });

  // Parse in GET parameteres and store in NVSRAM
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest * request) {

    const char* PARAM_MESSAGE1 = "WiFi_Name";
    const char* PARAM_MESSAGE2 = "WiFi_Password";
    const char* PARAM_MESSAGE3 = "Project_Name";
    const char* PARAM_MESSAGE4 = "Lat_Str";
    const char* PARAM_MESSAGE5 = "Long_Str";
    const char* PARAM_MESSAGE6 = "Guest_WiFi";

    if (myVars.varsGood != 190)
    {
      if (request->hasParam(PARAM_MESSAGE1)) {
        WiFiName = request->getParam(PARAM_MESSAGE1)->value();
      }
      if (request->hasParam(PARAM_MESSAGE2)) {
        WiFiPassword = request->getParam(PARAM_MESSAGE2)->value();
      }
      if (request->hasParam(PARAM_MESSAGE3)) {
        ProjectName = request->getParam(PARAM_MESSAGE3)->value();
      }
      if (request->hasParam(PARAM_MESSAGE4)) {
        LatStr = request->getParam(PARAM_MESSAGE4)->value();
      }
      if (request->hasParam(PARAM_MESSAGE5)) {
        LongStr = request->getParam(PARAM_MESSAGE5)->value();
      }
      if (request->hasParam(PARAM_MESSAGE6)) {
        GuestWiFi = request->getParam(PARAM_MESSAGE6)->value();
      }
      else {
        // do nothing
      }
      String HTMLText_2 = "<h1>LoRa Gateway Setup</h1>Hello, you entered values:<br><form> Wifi name:  " + WiFiName + "<br>Wifi password:  " + WiFiPassword + "<br>Project name:  " + ProjectName + "<br>Latitude: " + LatStr + "<br>Longitude: " + LongStr + "<br>Guest Wifi:  " + GuestWiFi + "<br></form><a href='/home'>RE-ENTER</a> | <a href='/save'> CONFIRM</a>";
      String HTMLConfirm = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + HTMLText_2 + HTMLFoot);
      request->send(200, "text/html", HTMLConfirm);
      Serial.print(F("WiFi SSID: ")); Serial.println(WiFiName);
      Serial.print(F("WiFi Password: ")); Serial.println(WiFiPassword);
      Serial.print(F("Lattitude: ")); Serial.println(LatStr);
      Serial.print(F("Logitude: ")); Serial.println(LongStr);
      Serial.print(F("Project Name: ")); Serial.println(ProjectName);
      Serial.print(F("WEP: ")); Serial.println(GuestWiFi);

      // wd convert strings and save to char
      WiFiName.toCharArray(myVars.ssid, 25);
      WiFiPassword.toCharArray(myVars.password, 20);
      WiFiPassword.toCharArray(myVars.password, 20);
      LatStr.toCharArray(myVars.Latstr, 12);
      LongStr.toCharArray(myVars.Lonstr, 12);
      ProjectName.toCharArray(myVars.ProjectName, 100);
      GuestWiFi.toInt();
      /*
            if (GuestWiFi != 1) {
              myVars.varsWEP = 200;
            }
      */
      //GuestWiFi.toInt(uint8_t(myVars.varsWEP));
      //uint8_t(myVars.varsWEP) = uint8_t(GuestWiFi);
      Serial.print(F("Project name = ")); Serial.println(myVars.ProjectName);
      Serial.print(F("Wifi Name = ")); Serial.println(myVars.ssid);
      Serial.print(F("Wifi password = ")); Serial.println(myVars.password);
      Serial.print(F("WEP value = ")); Serial.println(myVars.varsWEP);
      Serial.print(F("varsGood value = ")); Serial.println(myVars.varsGood);
    }
  });

  // Save values and restart
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLSave = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><br><br>Saving values and restarting in 5 seconds...</div>" + HTMLFoot);
    request->send(200, "text/html", HTMLSave);

    myVars.varsGood = 190;
    EEPROM.put(0, myVars);
    EEPROM.commit();
    display.clear();
    display.drawString(0, 30, "Saved, restarting");
    display.display();
    delay(5000);
    ESP.restart();
  });

  // Save monitor values and restart
  server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLMonitor = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><br><br>Starting Monitor Mode in 5 seconds...</div>" + HTMLFoot);
    request->send(200, "text/html", HTMLMonitor);
    myVars.varsGood = 200;
    EEPROM.put(0, myVars);
    EEPROM.commit();
    display.clear();
    display.drawString(0, 30, "Monitor Mode, restarting");
    display.display();
    delay(5000);
    ESP.restart();
  });

  // Save monitor values and restart
  server.on("/repeater", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLMonitor = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><br><br>Starting Repeater Mode in 5 seconds...</div>" + HTMLFoot);
    request->send(200, "text/html", HTMLMonitor);
    myVars.varsGood = 250;
    EEPROM.put(0, myVars);
    EEPROM.commit();
    display.clear();
    display.drawString(0, 30, "Repeater Mode, restarting");
    display.display();
    delay(5000);
    ESP.restart();
  });

  // Select Spreading factor
  server.on("/SetSF", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLSetSF = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><h2>Advanced Settings:</h2>Set Radio Spreading Factor<form><a href='/SF_1'>Default</a> | <a href='/SF_2'>Long Range</a></form><br><a href='/home'>BACK</a></div>" + HTMLFoot);
    request->send(200, "text/html", HTMLSetSF);
  });

  // Set SF Default
  server.on("/SF_1", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLSetSF1 = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><h2>Advanced Settings:</h2><form>Radio spreading factor set to default.<br><br></form><a href='/home'>Continue</a></div>" + HTMLFoot);
    request->send(200, "text/html", HTMLSetSF1);
    myVars.varsSF = 100;
  });

  // Set SF Default
  server.on("/SF_2", HTTP_GET, [](AsyncWebServerRequest * request) {
    String HTMLSetSF2 = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + "<div><h2>Advanced Settings:</h2><form>Radio spreading factor set to long range.<br><br></form><a href='/home'>Continue</a></div>" + HTMLFoot);
    request->send(200, "text/html", HTMLSetSF2);
    myVars.varsSF = 200;
  });

  digitalWrite(LED_BUILTIN, HIGH);
  server.begin();

}

void FLASHvariables::save()
{
  EEPROM.put(0, myVars);
  EEPROM.commit();
}

void FLASHvariables::get()
{
  EEPROM.begin(sizeof(myVars));
  EEPROM.get(0, myVars);
}
/********************** End of AP Setup Section *******************************/



/************************** Start of LoRa Monitor Function ******************************/
//char Mbuffer[50];
void Monitor() {

  Serial.println();
  Serial.println(F("Starting Monitor listen"));
  Serial.println();

  int i = 0; //used for oled saver
  while (myVars.varsGood == 200) {
    // This section prevents OLED display burn when running for long periods
    i++;
    if (i <= 80) {
      display.clear();
      display.drawString(10, 0, "Monitor Ready");
      display.display();
    }
    if (i > 81 && i <= 160) {
      display.clear();
      display.drawString(10, 25, "Monitor Ready");
      display.display();
    }
    if (i > 161 && i <= 220) {
      display.clear();
      display.drawString(10, 45, "Monitor Ready");
      display.display();
    }
    if (i > 220) {
      display.clear();
      display.display();
      i = 0;
    }

    // wd this is the LoRa Radio recieve section
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      // Should be a message for us now
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println();
        RH_RF95::printBuffer("Received Data: ", buf, len);
        Serial.println();
        Serial.print(F("Received Message: "));
        Serial.println((char*)buf);
        Serial.print(F("Message Length: "));
        Serial.println(len);
        Serial.print(F("RSSI: "));
        Serial.println(rf95.lastRssi(), DEC);

        RS = rf95.lastRssi();
        dtostrf(RS, 1, 0, Rstr);
        sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s", GWID, buf, Rstr, myVars.Latstr, myVars.Lonstr, myVars.ProjectName);
        Serial.println(F("Local Mode Message:"));
        Serial.println(Gbuffer);
        delay(10);
        memset(Gbuffer, '\0', sizeof(Gbuffer)); // reset buffer to clear previous messages
        delay(10);
        display.clear();
        display.drawString(0, 0, "LoRa Message:");
        display.drawString(0, 15, (char*)buf);
        display.drawString(0, 35, "RSSI:");
        display.drawString(50 , 35 , Rstr);
        display.display();
        delay(3000);
        digitalWrite(LED_BUILTIN, LOW);
      }
      else
      {
        Serial.println(F("recv failed"));
      }
    }
    if (! digitalRead(BUTTON_A))
    {
      Serial.println();
      display.clear();
      display.drawString(0, 0, "LoRa Message: ");
      display.drawString(0, 15, "Monitor TEST");
      display.display();
      sprintf(Sbuffer, "Test:%s", MonID);
      sendLen = strlen(Sbuffer);
      Serial.print(F("Sending LoRa Test Message: "));
      Serial.print(Sbuffer);
      Serial.print(F(". Length: "));
      Serial.println(sendLen);
      rf95.send((uint8_t *) Sbuffer, sendLen);
      rf95.waitPacketSent();
      Serial.println(F("Message Sent."));
      Serial.println();
      display.drawString(0, 30, "Sent Test Message..");
      display.display();
      delay(1000);
      display.clear();
    }


    // Print out some dots...
    uint8_t dot;
    dot++;
    Serial.print(F("."));
    if (dot == 80) {
      Serial.println(F("."));
      dot = 0;
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
  }
}



/************************** Start of Repeater Function ******************************/

void Repeater() {

  // Enable watchdog
  Serial.println();
  Serial.println(F("Enable Watchdog"));
  Serial.println(F("Starting Repeater listen"));
  Serial.println();
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, true);  //set time in us, reboot true
  timerAlarmEnable(timer);                          //enable interrupt

  int i = 0; // used for oled saver

  while (myVars.varsGood == 250) {
    byte sendLen;

    // This section prevents OLED display burn when running for long periods
    i++;
    if (i <= 80) {
      display.clear();
      display.drawString(0, 45, "RP Ready");
      display.display();
    }
    if (i > 81 && i <= 160) {
      display.clear();
      display.drawString(40, 45, "RP Ready");
      display.display();
    }
    if (i > 161 && i <= 240) {
      display.clear();
      display.drawString(80, 45, "RP Ready");
      display.display();
    }
    if (i > 240) {
      display.clear();
      display.display();
      i = 0;
    }

    // wd this is the LoRa Radio recieve section
    // Serial.println();
    timerWrite(timer, 0); //reset timer (feed watchdog)
    delay(10);
    // Serial.println(F("Reset Watchdog"));
    // Serial.println();
    // This is the LoRa message repeat section
    // Serial.println(F(" Check for LoRa message"));
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        Serial.println();
        Serial.println(F("LoRa message received"));
        //digitalWrite(LED_BUILTIN, HIGH);
        Serial.print(F("got message: "));
        Serial.println((char*)buf);
        Serial.print(F("RSSI: "));
        Serial.println(rf95.lastRssi(), DEC);

        display.clear();
        display.drawString(0, 0, "LoRa Message:");
        display.drawString(0, 15, (char*)buf);
        display.drawString(0, 35, "RSSI:");
        display.drawString(50 , 35 , Rstr);
        display.display();

        RS = rf95.lastRssi();
        dtostrf(RS, 1, 0, Rstr);

        Found = strstr((char*)buf, RPID); //check for looped message

        if (Found != NULL) {
          Serial.println(F("This repeater ID found, dropping message")); //Do not send
          Serial.println();
          display.clear();
          display.drawString(0, 0, "DROPPED Message:");
          display.drawString(0, 15, (char*)Rbuffer);
          display.drawString(0, 35, "RSSI:");
          display.drawString(50 , 35 , Rstr);
          display.display();
          delay(3000);
        }
        else {
          digitalWrite(LED_BUILTIN, HIGH);

          //NOTE: sprintf does not support String objects.  It only understands char types.
          sprintf(Rbuffer, "%s,%s,%s", buf, RPID, Rstr);
          Serial.print(F("TX delay: ")); Serial.print(RPDelay); Serial.println(F(" mSec"));
          // Send buffer and send length to radio
          delay(RPDelay);      // delay to avoid message collision, for multi repeaters installs use different delay time
          sendLen = strlen(Rbuffer);
          rf95.send((uint8_t *) Rbuffer, sendLen);
          rf95.waitPacketSent();
          rf95.sleep();
          Serial.print(F("Sent Data: "));
          Serial.println(Rbuffer);
          Serial.print(F("Send Length: "));
          Serial.println(sendLen);
          Serial.println();
          display.clear();
          display.drawString(0, 0, "Repeated Message:");
          display.drawString(0, 15, (char*)Rbuffer);
          display.drawString(0, 35, "RSSI:");
          display.drawString(50 , 35 , Rstr);
          display.display();
          Found = '\0'; //clear "Found" char value
          memset(Rbuffer, '\0', sizeof(Rbuffer)); //reset buffer to clear pervious messages
          delay(10);
          delay(1500);
          digitalWrite(LED_BUILTIN, LOW);
        }
      }
      else
      {
        Serial.println(F("recv failed"));
      }
    }
    if (! digitalRead(BUTTON_A))
    {
      sendTest();
    }

    // Send node health status and sensor data every 10 minutes
    if (millis() > time_now + period) {
      time_now = millis();
      /*
        // Measure battery and send repeater status
        float measuredvbat = analogRead(VBATPIN);
        measuredvbat *= 2; // we divided by 2, so multiply back
        measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
        measuredvbat /= 1024; // convert to voltage
        B = measuredvbat;
        Serial.print(F("VBat: " ); Serial.println(B);
      */
      // uptime
      Mills = (millis() / 1000) / 60;
      dtostrf(Mills, 1, 0, Uptime);
      // convert doubles to char-string, add char-string to buffer array
      //NOTE: sprintf does not support String objects.  It only understands char.
      sprintf(Rbuffer, "%s,%s", SNodeID, Uptime);
      //transmit buffer and send length
      sendLen = strlen(Rbuffer);
      rf95.send((uint8_t *) Rbuffer, sendLen);
      rf95.waitPacketSent();
      rf95.sleep();
      display.clear();
      display.drawString(0, 15, "Send Status");
      display.drawString(0, 25, SNodeID);
      display.display();
      delay(2000);
      Serial.print(F("Send Status: "));
      Serial.println(Rbuffer);
    }

    // Print out some dots...
    uint8_t dot;
    dot++;
    //Serial.print(F("."));
    if (dot == 80) {
      dot = 0;
      //Serial.println(F("."));
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}



/************************** Start LoRa Send Test Function ******************************/
void sendTest() {
  display.clear();
  display.drawString(0, 0, "LoRa Message: ");
  display.drawString(0, 15, "SENDTEST");
  display.display();

  // Get uptime
  Mills = (millis() / 1000) / 60;
  dtostrf(Mills, 1, 0, Uptime);
  sprintf(Sbuffer, "Test:%s:%s:%s", GWID, Uptime, Version);
  sendLen = strlen(Sbuffer);
  Serial.println();
  Serial.print(F("Sending LoRa Test Message: "));
  Serial.print(Sbuffer);
  Serial.print(F(". Length: "));
  Serial.println(sendLen);
  rf95.send((uint8_t *) Sbuffer, sendLen);
  rf95.waitPacketSent();
  Serial.println(F("Message Sent."));
  Serial.println();
  display.drawString(0, 30, "Sent Test Message..");
  display.display();
  delay(1000);
  display.clear();
}


/************************** Start of MQTT SUbscribe Function ******************************/
void mqttSubscribe() {
  // Serial.print(F("   Check MQTT Connection. "));
  GWConnect();
  // Serial.println();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &control) {
      Serial.print(F("4a. Subscribe got: "));
      Serial.println((char *)control.lastread);
      if (0 == strcmp((char *)control.lastread, "RESET")) {
        Serial.println(F("WD resetting"));
        display.clear();
        display.drawString(0 , 0 , "MQTT Message:");
        display.drawString(0 , 15 , "RESET");
        display.display();
        delay(1000);
        ESP.restart();
        delay(1000);
      }
      if (0 == strcmp((char *)control.lastread, "SENDTEST")) {
        Serial.print(F("TX delay: ")); Serial.print(RPDelay); Serial.println(F(" mSec"));
        // Send buffer and send length to radio
        delay(RPDelay);       // delay to avoid message collision
        sendTest();           // Send LoRa status
        delay(100);
        gatewayStatus();
      }
      if (0 == strcmp((char *)control.lastread, "SETUP")) {
        display.clear();
        display.drawString(0, 0, "MQTT Message:");
        display.drawString(0, 15, "SETUP");
        display.display();
        Serial.println(F("Setting AP Mode true  "));
        myVars.varsGood = 180;
        delay(1000);
        myVars.save();
        display.clear();
        display.drawString(15, 25, "AP MODE");
        display.display();
        delay(3000);
        ESP.restart();
      }
      if (0 == strcmp((char *)control.lastread, "DEFAULT")) {
        display.clear();
        display.drawString(0, 0, "MQTT Message:");
        display.drawString(0, 15, "DEFAULT");
        display.display();
        Serial.println(F("Setting Default SF"));
        myVars.varsSF = 100;
        delay(1000);
        myVars.save();
        display.clear();
        display.drawString(15, 25, "Default SF");
        display.display();
        delay(3000);
        ESP.restart();
      }
      if (0 == strcmp((char *)control.lastread, "LONG")) {
        display.clear();
        display.drawString(0, 0, "MQTT Message:");
        display.drawString(0, 15, "LONG");
        display.display();
        Serial.println(F("Setting Long SF"));
        myVars.varsSF = 200;
        delay(1000);
        myVars.save();
        display.clear();
        display.drawString(15, 25, "Long SF");
        display.display();
        delay(3000);
        ESP.restart();
      }
    }
  }
}


void loop() {
  // leave this empty, it all happens in the functions. Any stuff in here messes with the Wifi AP setup mode.
}

// -- End of File --