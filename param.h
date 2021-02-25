// *************** Unique ID's *******************//
#define GWID "TTGWLB1"        // Gateway mode ID
char* MonID = "TTMonLB1";     // Monitor mode ID must start with TTMon to trigger auto-reply from a receiving gateway
char* RPID = "RP8";           // Repeater mode ID used to prevent endless message loops between repeaters
char* SNodeID = "TT-RP8";     // Node ID used in repeater mode status message. Please see the manual for more explanations of the monitor and repeater operating modes

#define MQTT_SERVER       "mqtt.innovateauckland.nz"
#define MQTT_SERVERPORT   1883
#define MQTT_USERNAME     "simple"
#define MQTT_KEY          "simplePassword"              // key or password
#define MQTT_TOPIC        "AKLC"                  // topic root
