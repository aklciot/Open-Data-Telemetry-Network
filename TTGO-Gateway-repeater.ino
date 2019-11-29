
//*************************************************************************************************
//  TTGO ESP32 LoRa Wifi MQTT gateway for version 1.0, version 1.6 and T-Beam. Power Utilisation: 120mAh?
//  Includes remote send LoRa test message and system reset functions via MQTT subscription
//  Hold Button A when starting to enter AP setup mode
//  Browser connect to http://192.168.4.1/home to enter wifi settings, monitor or repeater mode and LoRa spreading factor settings.
//  version JW-2.1.7 Moved changable parameters to a separate file
//  version AC-2.1.6 Added repeater loop'd message detection. Thanks Aldo!
//  version WD-2.1.5 Enabled watchdog reset to Gateway mode.
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
//***********************************************************************************************

// Paramaters that are likely to change are stored in a separate file.
// This source code file should not change between rebuilds
#include "param.h"

/*
  //********* MQTT Connection Setup ********
  #define MQTT_SERVER       "m13.cloudmqtt.com"
  #define MQTT_SERVERPORT   17320
  #define MQTT_USERNAME     "rcmlpohh"
  #define MQTT_KEY          "3t20uZtJTn6Z"              // key or password
  #define MQTT_TOPIC        "AKLC"                  // topic root
*/

/********* MQTT WD Connection Setup ********/
/*#define MQTT_SERVER       "m14.cloudmqtt.com"
  #define MQTT_SERVERPORT   14844
  #define MQTT_USERNAME     "hdxpjczm"
  #define MQTT_KEY          "O0HdZvb3kD6m"              // key or password
  #define MQTT_TOPIC        "AKLC"                  // topic root
*/



// *************************************************************************************************//
// ********************************** Do not edit below this line **********************************//
// *************************************************************************************************//

#define Version "2.1.7"

// ******************** Required Libraries **********************//
#include "EEPROM.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SSD1306.h"      // OLED Display
#include <WiFi.h>         // Wifi driver
#include <RH_RF95.h>      // LoRa radio
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
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

#define RF95_FREQ 915.          // Set radio frequency


// TTGO version 1.0
#define RFM95_RST 14
#define RFM95_CS  18
#define RFM95_INT 26
#define LED_BUILTIN 2
#define BUTTON_A 0                // Setup AP initialise button
#define Type "TTV1"
SSD1306 display(0x3c, 4, 15);     // TTGO VER 1.0 // OLED Display SDA = pin 4, SCL = pin 15, RESET = pin 16


//TTGO Version 1.6
/*
  #define RFM95_RST     23
  #define RFM95_CS      18
  #define RFM95_INT     26
  #define LED_BUILTIN   25            //TTGO V1:2, TTGO V2:25, TBeam:14
  #define BUTTON_A      2             //setup AP initialise button
  #define Type "TTV2"
  SSD1306 display(0x3c, 21, 22);      //TTGO VER 2.1.6 and TBeam OLED Display SDA = pin 21, SCL = pin 22, RESET = pin 16
*/

/*
  //TTGO TBeam Version 1.0
  #define RFM95_RST 23
  #define RFM95_CS  18
  #define RFM95_INT 26
  #define LED_BUILTIN 14            //TTGO V1:2, TTGO V2:25, TBeam:14
  #define BUTTON_A 39               //setup AP initialise button
  #define Type "TTV3"
  SSD1306 display(0x3c, 21, 22);    //TTGO VER 2.1.6 and TBeam OLED Display SDA = pin 21, SCL = pin 22, RESET = pin 16
*/
/*******************************************************************************************/


/****************************** Load drivers ***************************************/

// wd load singleton instance of the radio driver and called it "rf95"
RH_RF95 rf95(RFM95_CS, RFM95_INT);

WiFiClient client;

// wd load mqtt driver called mqtt
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

/***************************** MQTT Queue Config **********************************/

//const char MQTT_TOPIC = (GWID).c_str(); // topic root value

// Notice MQTT paths for Adafruit AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish pubmessage = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC "/Gateway/"GWID);
Adafruit_MQTT_Publish pubstatus = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC "/Status/"GWID);
Adafruit_MQTT_Subscribe control = Adafruit_MQTT_Subscribe(&mqtt, MQTT_TOPIC "/Control/"GWID);

/******************************* ESP Watchdog ***********************************/

const int button = 0;         //gpio to use to trigger delay
const int wdtTimeout = 3000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

int keyIndex = 0;             // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

/**************************** SETUP SECTION ***************************/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
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
  Serial.println("<<<--------------- SRAM Values ----------------->>>");
  Serial.println();
  Serial.print("Gateway name = "); Serial.println(GWID);
  Serial.print("Project name = "); Serial.println(myVars.ProjectName);
  Serial.print("Wifi Name = "); Serial.println(myVars.ssid);
  Serial.print("Wifi password = "); Serial.println(myVars.password);
  Serial.print("varsGood value = "); Serial.println(myVars.varsGood);
  Serial.print("WEP value = "); Serial.println(myVars.varsWEP);
  Serial.print("Spreading Factor value = "); Serial.println(myVars.varsSF);
  Serial.print("WiFi.macAddress: "); Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.println("<<<--------------------------------------------->>>");
  delay(2000);

  display.clear();
  display.drawString(0, 0, "LoRa MQTT Gateway");
  display.drawString(0, 10, "Gateway ID: " GWID);
  display.drawString(0, 20, "Version: "Version);
  display.display();
  delay(4000);
  display.drawString(0, 35, "STARTING..");
  display.display();
  delay(2000);

  // wd Set SRAM varsGood value to 180 trigger APSetup mode on reboot
  if (! digitalRead(BUTTON_A))
  {
    Serial.println("Button A:  ");
    Serial.println("Setting AP Mode true  ");
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
  Serial.println("****************************");
  if (myVars.varsSF == 200)
  {
    //wd config for long range. Put just after radio init
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: LowDataRate=On, Agc=On
    };
    rf95.setModemRegisters(&modem_config);
    Serial.println("LoRa radio set to long range");
  }
  else {
    Serial.println("LoRa radio set to default");
  }
  Serial.println("****************************");

  Serial.println();
  Serial.println("LoRa radio init OK!");

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
  Serial.println("Finished Setup");

  // wd start Wifi Connect Mode or
  if (myVars.varsGood == 190)
  {
    /*
      // ESP Watchdog setup
      timer = timerBegin(0, 80, true);                  //timer 0, div 80
      timerAttachInterrupt(timer, &resetModule, true);  //attach callback
      timerAlarmWrite(timer, wdtTimeout * 10000, false); // multiple of 3000ms wdTimeout set time in ms
      timerAlarmEnable(timer);                          //enable interrupt */
    mqtt.subscribe(&control); // wd mqtt subsribe topic
    Serial.println("Good eeprom variables found, starting Gateway mode... ");
    display.clear();
    display.drawString(25, 25, "Gateway Mode");
    display.display();
    delay(3000);
    GWConnect();
  }
  // wd start LoRa Monitor Mode
  if (myVars.varsGood == 200)
  {
    Serial.println("Good eeprom variables found, starting Monitor mode... ");
    display.clear();
    display.drawString(25, 25, "Monitor Mode");
    display.display();
    delay(3000);
    Monitor();
  }
  // wd start LoRa Repeater Mode
  if (myVars.varsGood == 250)
  {
    Serial.println("Good eeprom variables found, starting Repeater mode... ");
    display.clear();
    display.drawString(25, 25, "Repeater Mode");
    display.display();
    delay(3000);
    Repeater();
  }
  else
    // wd start APSetup Mode
  {
    Serial.println("Starting AP mode... ");
    display.clear();
    display.drawString(25, 25, "Setup Mode");
    display.display();
    delay(3000);
    APSetup();
  }
}




// ****** wd variables told hold message values *********
char Gbuffer[100];
char* Endstr = "End";                   //last character in string, end of message
char* Statstr = "OK";                   // holds status string value used to indicate power fail if implemented
char Uptime[20];                        // holds uptime string value
char Rstr[10];                          // holds RSSI string value
double RS;                              // create variable doubles to hold numeric values
double Mills;                           // double for uptime value
int period = 300000;                    // 5min delay for status message publish
unsigned long time_now = 0;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];   //wd keep outside of loop to avoid klingon
byte sendLen;
int value = 0;


/************************** Gateway Loop ******************************/
void Gateway() {
  // Ensure the connection to the MQTT server is alive and automatically reconnect when disconnected.
  // See the GWConnect function definition below.

  // ESP Watchdog setup
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 10000, false); // multiple of 3000ms wdTimeout set time in ms
  timerAlarmEnable(timer);                          //enable interrupt

  while (myVars.varsGood == 190) {
    Serial.println("");
    timerWrite(timer, 0); //reset timer (feed watchdog)
    Serial.print(F("1. Start Loop. Check mqtt connect, "));
    GWConnect();

    // check for messages on mqtt subscribe topic
    //timerWrite(timer, 0); //reset timer (feed watchdog)
    Serial.println(F("2. Wait for incoming mqtt message"));
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(500))) {
      if (subscription == &control) {
        Serial.print(F("Got: "));
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
          display.clear();
          display.drawString(0, 0, "MQTT Message:");
          display.drawString(0, 15, "SENDTEST");
          display.display();
          sprintf(Gbuffer, "Test from Gateway: %s", GWID);
          sendLen = strlen(Gbuffer);
          rf95.send((uint8_t *) Gbuffer, sendLen);
          rf95.waitPacketSent();
          display.drawString(0, 30, "Sent Test Message..");
          display.display();
          delay(1000);
        }
        if (0 == strcmp((char *)control.lastread, "SETUP")) {
          display.clear();
          display.drawString(0, 0, "MQTT Message:");
          display.drawString(0, 15, "SETUP");
          display.display();
          Serial.println("Setting AP Mode true  ");
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
          Serial.println("Setting Default SF");
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
          Serial.println("Setting Long SF");
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

    // wd this is the LoRa Radio receive section
    //timerWrite(timer, 0); //reset timer (feed watchdog)
    Serial.println(F("3. Check radio is available"));
    display.clear();
    display.drawString(0, 45, "Ready...");
    display.display();
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      //timerWrite(timer, 0); //reset timer (feed watchdog)
      Serial.println(F("4. Check for LoRa message"));
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        Serial.println(F("5. LoRa message received"));
        digitalWrite(LED_BUILTIN, HIGH);
        RS = rf95.lastRssi();
        dtostrf(RS, 3, 2, Rstr);
        //NOTE: sprintf does not support String objects.  It only understands char arrays.
        //sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s", NodeID, buf, Rstr, Latstr, Lonstr, Endstr);
        sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s", GWID, buf, Rstr, myVars.Latstr, myVars.Lonstr, myVars.ProjectName);
        Serial.println(F("6. Message Data: "));
        Serial.print(Gbuffer);
        Serial.println();
        display.clear();
        display.drawString(0, 0, "LoRa Message:");
        display.drawString(0, 15, (char*)buf);
        display.display();

        // MQTT Message publish...
        Serial.println(F("7. Start Publish"));
        if (! pubmessage.publish(Gbuffer)) {
          Serial.println(F("Failed"));
          display.drawString(0, 35, "Publish Failed");
          display.display();
        }
        else {
          Serial.println(F("8. Publish OK!"));
          display.drawString(0, 35, "Publish OK");
          display.display();
        }
        delay(1500);
        display.clear();
        display.display();
        digitalWrite(LED_BUILTIN, LOW);
      }
      else
      {
        Serial.println(F("recv failed"));
      }
    }

    // Send node health status and sensor data every 5 minutes
    if (millis() > time_now + period) {
      time_now = millis();
      // uptime
      Mills = (millis() / 1000) / 60;
      dtostrf(Mills, 1, 0, Uptime);
      sprintf(Gbuffer, "%s,%s,%s,%s,%s,%s,%s,%s", GWID, Statstr, Version, Type, Uptime, myVars.Latstr, myVars.Lonstr, myVars.ProjectName);
      if (! pubstatus.publish(Gbuffer)) {
        Serial.println(F("Publish Failed"));
      }
      Serial.println(Gbuffer);
      display.clear();
      display.drawString(0, 15, "Send Status");
      display.drawString(0, 25, GWID);
      display.display();
      delay(2000);
    }

    // Send LoRa test message
    if (! digitalRead(BUTTON_A))
    {
      display.clear();
      display.drawString(0, 0, "Test Message:");
      display.drawString(0, 15, GWID);
      display.display();
      sprintf(Gbuffer, "Test from Gateway: %s", GWID);
      sendLen = strlen(Gbuffer);
      rf95.send((uint8_t *) Gbuffer, sendLen);
      rf95.waitPacketSent();
      display.drawString(0, 30, "Sent Test Message");
      display.display();
      Serial.print("Test Message Button A:  ");
      Serial.println(F(Gbuffer));
      delay(1000);
    }

    Serial.println();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    display.clear();
    display.display();
  }
}


/********************** Connect Section *******************************/
int counter1 = 0; //health reset counter 1
int counter2 = 0; //health reset counter 2
void GWConnect()
{
  // MQTT Connection section
  int8_t ret;
  if (mqtt.connected()) {
    Serial.println("connection OK.");
    timerWrite(timer, 0); //reset timer (feed watchdog)
    return;
  }
  else {
    // Wifi Connection section
    while (WiFi.status() != WL_CONNECTED) {
      counter1++;
      Serial.print("WiFi Connect Counter: ");
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
      Serial.println(F("Attempting to connect to SSID: "));
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
        Serial.print(".");
      }
    }
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // MQTT Connection section
    Serial.print(F("Connecting to MQTT... "));
    display.clear();
    display.drawString(0 , 0 , "Connecting to MQTT... ");
    display.drawString(0 , 15 , MQTT_SERVER);
    display.display();

    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      if (counter2 == 5) {
        counter2 = 0;
        Serial.println(F("WD resetting"));
        display.clear();
        display.drawString(0 , 0 , "MQTT Connect Error");
        display.drawString(0 , 15 , "Resetting");
        display.display();
        delay(10000);
        //digitalWrite(resetPin, LOW);
        ESP.restart();
        delay(1000);
      }
      Serial.println(mqtt.connectErrorString(ret));
      Serial.print("MQTT Host is: ");
      Serial.println(MQTT_SERVER);
      Serial.print("MQTT port is: ");
      Serial.println(MQTT_SERVERPORT);
      Serial.print("MQTT user is: ");
      Serial.println(MQTT_USERNAME);
      Serial.print("MQTT password is: ");
      Serial.println(MQTT_KEY);

      Serial.println(F("Retrying MQTT connection in 4 seconds..."));
      mqtt.disconnect();
      //timerWrite(timer, 0); //reset timer (feed watchdog)
      delay(4000);  // wait 4 seconds
      counter2++;
      Serial.print("MQTT Counter: "); Serial.println(counter2);
    }
    Serial.println(F("MQTT Connected!"));
    //timerWrite(timer, 0); //reset timer (feed watchdog)
    display.clear();
    display.drawString(15, 15, "MQTT Connected!");
    display.display();
    delay(2000);
    Gateway();
  }
}



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
String HTMLForm_2 = "<div class='input'><form action='update' method='get' ><br>Wifi Name:<br><input type='text' name='WiFi_Name'><br>Wifi Password:<br><input type='text' name='WiFi_Password'><br>Project Name:<br><input type='text' name='Project_Name'><br>Gateway Location:<br>Lattitude:<br><input type='text' name='Lat_Str' value='-36.833343'><br><br>Longitude:<br><input type='text' name='Long_Str' value='174.831117'><br><input type='checkbox' name='Guest_WiFi' value='1'>Use Guest Wifi, no password<br><input type='submit' value='Submit'></form></div>";
String HTMLNav = "<p><a href='/monitor'>MONITOR MODE</a>|<a href='/repeater'>REPEATER MODE</a>|<a href='/SetSF'>ADVANCED</a>|<a href='/save'>EXIT SETUP</a>";
String HTMLFoot = "<br><br><p>Gateway Version: 2.1.6, Innovate Auckland<br>Copyright W.Davies 2019</p></body></html>";

String HTMLHome = String(HTMLHead + HTMLStyle_1 + HTMLStyle_2 + HTMLStyle_3 + HTMLText_1 + HTMLForm_2 + HTMLNav + HTMLFoot);

AsyncWebServer server(80);
void APSetup()
{
  //*** Variables for APSetup mode and GET Parameters ******
  const char *APssid = "GWSetup_1";
  const char *APpassword = "testpassword"; //not used
  display.clear();
  display.drawString(25, 5, "SETUP MODE");
  display.drawString(0, 30, "Wifi connect to:");
  display.drawString(0, 45, APssid);
  display.display();
  WiFi.softAP(APssid); // no password
  Serial.println();
  Serial.println("AP setup mode");
  Serial.print("AP IP address: ");
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
      Serial.print("WiFi SSID: "); Serial.println(WiFiName);
      Serial.print("WiFi Password: "); Serial.println(WiFiPassword);
      Serial.print("Lattitude: "); Serial.println(LatStr);
      Serial.print("Logitude: "); Serial.println(LongStr);
      Serial.print("Project Name: "); Serial.println(ProjectName);
      Serial.print("WEP: "); Serial.println(GuestWiFi);

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
      Serial.print("Project name = "); Serial.println(myVars.ProjectName);
      Serial.print("Wifi Name = "); Serial.println(myVars.ssid);
      Serial.print("Wifi password = "); Serial.println(myVars.password);
      Serial.print("WEP value = "); Serial.println(myVars.varsWEP);
      Serial.print("varsGood value = "); Serial.println(myVars.varsGood);
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

char Mbuffer[50];

void Monitor() {
  while (myVars.varsGood == 200) {
    // wd this is the LoRa Radio recieve section
    display.clear();
    display.drawString(0, 45, "Monitor Ready...");
    display.display();
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      // Should be a message for us now
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        digitalWrite(LED_BUILTIN, HIGH);
        RH_RF95::printBuffer("request: ", buf, len);
        Serial.println();
        Serial.println((char*)buf);
        Serial.println("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);
        RS = rf95.lastRssi();
        dtostrf(RS, 3, 2, Rstr);
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
        Serial.println("recv failed");
      }
    }
    if (! digitalRead(BUTTON_A))
    {
      byte sendLen;
      display.clear();
      display.drawString(0, 0, "Test Message:");
      display.drawString(0, 15, MonID);
      display.display();
      sprintf(Mbuffer, "Test from Monitor: %s", MonID);
      sendLen = strlen(Mbuffer);
      rf95.send((uint8_t *) Mbuffer, sendLen);
      rf95.waitPacketSent();
      display.clear();
      display.drawString(0, 0, "Sent Test Message:");
      display.drawString(0, 15, Mbuffer);
      display.display();
      Serial.println();
      Serial.println("Test Message Button A:  ");
      Serial.println(F(Mbuffer));
      delay(2000);
    }
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    display.clear();
    display.display();
  }
}



/************************** Start of Repeater Function ******************************/


// ****** wd variables told hold values *********
char Rbuffer[50];
char Bstr[10];         // Battery Voltage
char* Found;           // Holds repeater ID value to prevent repeater loops.

//int period = 300000;   // 5min non blocking delay for light sensor section
//uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];   //wd keep outside of loop to avoid klingon

int r_counter = 0; //health counter
void Repeater() {
  while (myVars.varsGood == 250) {
    byte sendLen;
    // wd this is the LoRa Radio recieve section
    display.clear();
    display.drawString(30, 45, "RP Ready...");
    display.display();
    r_counter++;
    if (r_counter == 3000) {
      /*
        // Measure battery and send repeater status
        float measuredvbat = analogRead(VBATPIN);
        measuredvbat *= 2; // we divided by 2, so multiply back
        measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
        measuredvbat /= 1024; // convert to voltage
        B = measuredvbat;
        Serial.print("VBat: " ); Serial.println(B);
      */
      // uptime
      Mills = (millis() / 1000) / 60;
      dtostrf(Mills, 1, 0, Uptime);
      // convert doubles to char-string, add char-string to buffer array
      //NOTE: sprintf does not support String objects.  It only understands char arrays.
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
      Serial.print("Send Status: ");
      Serial.println(Rbuffer);
      r_counter = 0;
    }

    // This is the LoRa message repeat section
    Serial.print(r_counter); Serial.println(F(" Check for LoRa message"));
    if (rf95.available())
    {
      memset(buf, '\0', sizeof(buf)); //reset buffer to clear pervious messages
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
        Serial.println(F("LoRa message received"));
        //digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("got message: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);

        display.clear();
        display.drawString(0, 0, "LoRa Message:");
        display.drawString(0, 15, (char*)buf);
        display.drawString(0, 35, "RSSI:");
        display.drawString(50 , 35 , Rstr);
        display.display();

        RS = rf95.lastRssi();
        dtostrf(RS, 1, 1, Rstr);

        Found = strstr((char*)buf, RPID); //check for looped message

        if (Found != NULL) {
          Serial.println("This repeater ID found, dropping message"); //Do not send
          Serial.println();
          display.clear();
          display.drawString(0, 0, "DROPPED Message:");
          display.drawString(0, 15, (char*)Rbuffer);
          display.drawString(0, 35, "RSSI:");
          display.drawString(50 , 35 , Rstr);
          display.display();
          delay(1500);
        }
        else {
          digitalWrite(LED_BUILTIN, HIGH);
          //NOTE: sprintf does not support String objects.  It only understands char arrays.
          sprintf(Rbuffer, "%s,%s,%s", buf, RPID, Rstr);
          Serial.println("TX delay, 5sec");
          // Send buffer and send length to radio
          delay(5000); //wait to avoid TX collision
          sendLen = strlen(Rbuffer);
          rf95.send((uint8_t *) Rbuffer, sendLen);
          rf95.waitPacketSent();
          rf95.sleep();
          Serial.print("Sent Data: ");
          Serial.println(Rbuffer);
          Serial.print("Send Length: ");
          Serial.println(sendLen);
          Serial.println();
          display.clear();
          display.drawString(0, 0, "Repeated Message:");
          display.drawString(0, 15, (char*)Rbuffer);
          display.drawString(0, 35, "RSSI:");
          display.drawString(50 , 35 , Rstr);
          display.display();
          Found = '\0'; //clear "Found" char value
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
      display.clear();
      display.drawString(0, 0, "Send Test Message");
      display.display();
      sprintf(Rbuffer, "Test from Repeater: %s", RPID);
      sendLen = strlen(Rbuffer);
      rf95.send((uint8_t *) Rbuffer, sendLen);
      rf95.waitPacketSent();
      display.clear();
      display.drawString(0, 0, "Sent Test Message:");
      display.drawString(0, 15, Rbuffer);
      display.display();
      Serial.println();
      Serial.println("Test Message Button A:  ");
      Serial.println(F(Rbuffer));
      delay(2000);
    }
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    display.clear();
    display.display();
  }
}

void loop() {
  // leave this empty, it all happens in the gateway function. stuff in here messes with the Wifi setup mode.
}
