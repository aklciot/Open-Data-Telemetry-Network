// **********************************************************************************
// Sample Sensor Node, Ultrasonic Distance Sensor Example
// This Arduino code creates a simple LoRa message that sends the Node ID, ultrasonic and battery level values to the LoRa MQTT Gateway.
// Code and guide for the gateway node can be found here: https://github.com/aklciot/TTGO-Gateway-repeater
// Range is highly dependent on conditions however we average 0.5-1km urban areas and over 5km in rural.
//
// PLEASE NOTE: Set the LoRa SPI pin definitions correctly for your board, this code has been tested on LowpowerLabs Motieno and Adafruit 32u4/M0
// It is assumed you have installed an Arduino IDE,  included the latest radiohead library and have a compatible LoRa MCU board installed and attached.
//
// IMPORTANT: This is not a LoRaWAN node, it uses basic LoRa without adding the WAN overhead required for shared network systems.
// The intention is to provide the simplest possible telemetry solution.
//
//
// Questions or comments please contact us on our Github page, https://github.com/aklciot
// Innovate Auckland
// Version 0.1 26-July-2020, Initial Build, WD
// **********************************************************************************


#include <RH_RF95.h>      // Radiohead LoRa library
// #include <HCSR04.h>    // Ultrasonic distance sensor

char* NodeID = "Node1";   // Node ID

#define PWRLOOP 10        // test loop 1 sec count for sending data


//************************************ LoRa SPI pin definitions ***************************

//* RFM95 LoRa Radio Settings for moteino
/*
#ifdef __AVR_ATmega1284P__
#define LED       15      // Moteino MEGAs have LEDs on D15
#define FLASH_SS  23      // and FLASH SS on D23
#define RFM95_CS  4       // wd NSS chip select pin 4 for rf95 radio on MoteinoMEGA, 10 is standard
#define RFM95_RST 3       // reset not connected
#define RFM95_INT 2
#else
#define LED       9       // Moteinos have LEDs on D9
#define FLASH_SS  8       // and FLASH SS on D8
#define RFM95_CS  10      // wd NSS chip select pin 10 for rf95 radio on Moteino
#define RFM95_RST 9       // wd reset not used
#define RFM95_INT 2
#endif
*/

// Settings for Adafruit feather32u4 LoRa RFM95
/*
  #define RFM95_CS  8
  #define RFM95_RST 4
  #define RFM95_INT 7
  #define LED          13
*/

// Settings for Adafruit featherM0 LoRa RFM95 SPI
#define RFM95_RST    4    // LoRa radio reset pin
#define RFM95_CS     8    // LoRa radio chip select pin
#define RFM95_INT    3    // LoRa radio interrupt pin
#define LED          13


// LoRa radio frequency, use the correct frequency for your region
#define RF95_FREQ 915.0


// wd variables told hold string values
char Dstr[5];      // Distance value char
char Bstr[5];      // Battery value char
char buffer[50];    // Lora message char (max 50 bytes)
byte sendLen;       // Lora message length

double  Dist, Bat;
int distance;


//************************************ Load Drivers ********************************

RH_RF95 rf95(RFM95_CS, RFM95_INT);        // load radiohead driver instance of the radio and name it "rf95"

// UltraSonicDistanceSensor distanceSensor(6, 5);  // Initialize the HCSR04 sensor that uses digital pins (trig, echo).


void setup() {

  while (!Serial);   // Serial usb connection to show diagnostic data, comment out (//) when usb serial is not connected as the code will halt at this point
  Serial.begin(115200);    //set baud rate for the hardware serial port_0
  delay(100);

  Serial.println("Serial Started");

  pinMode(LED, OUTPUT);         // initialize digital pin as an output.

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  //wd initialise radio
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }


  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // wd set radio tx power and use PA false
  rf95.setTxPower(13, false);

  delay(500);  //wait for recode
  Serial.println("Node ready, starting main loop..");
  digitalWrite(LED, LOW);

}

/********** dtostrf function for M0 core *************************/
#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
static char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
#endif


void loop() {
  // Power Up and Start Sensors
  Serial.println();
  Serial.println("Sensor Start");

  // Measure distance, read UltraSonic
  // distance = distanceSensor.measureDistanceCm();  // HCSR04.h
  distance = 150;  // demo distance value for testing for testing
  Serial.println(distance);
  delay(100);

  // Measure battery
  /*
    double measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2; // we divided by 2, so multiply back
    measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    Bat = measuredvbat;
  */
  Bat = 4.5; // demo battery value for testing

  // convert doubles to char-string
  dtostrf(distance, 1, 0, Dstr);
  dtostrf(Bat, 1, 2, Bstr);

  // create outgoing LoRa message in "buffer"
  sprintf(buffer, "%s,%s,%s", NodeID, Dstr, Bstr);

  // send buffer message and length to radio
  sendLen = strlen(buffer);
  rf95.send((uint8_t *) buffer, sendLen);
  rf95.waitPacketSent();
  rf95.sleep();

  // wd serial diagnostics
  Serial.println("  ......... Message Data ...........  " );
  Serial.println();
  Serial.print("Distance: "); Serial.println(distance);
  Serial.print("Battery: "); Serial.println(Bat);
  Serial.println();
  Serial.print("Send Data: "); Serial.println(buffer);
  Serial.print("Send Length: "); Serial.println(sendLen);
  Serial.println("  ....................  " );
  Serial.println();
  delay(100);

  // To entering low power sleep mode or wait loop
  Serial.println("Wait Loop");

  uint8_t i;
  for (i = 0; i < PWRLOOP; i++) {
    delay(1000);
    Serial.print(i); Serial.println(",");
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
  }
}

// FILE END
