/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/

#include "EspMQTTClient.h"
// #include "Wire.h"
// #include "SHT31.h"
// #include "Ticker.h"

// #include "secrets.h"

//declarations for SHT31
#define SHT31_ADDRESS   0x44
uint32_t start;
uint32_t stop;
SHT31 sht;

// MQTT server connection
const char* MQTT_HOST = "192.168.7.245";
const uint16_t MQTT_PORT = 1883;
const char* mqttClientName = "BATHROOM_ESP8266";

// const int BUTTON_PIN     =7;  // the number of the pushbutton pin D7
// const int RED_LED_PIN    =5;   // the number of the RED LED pin D5
// const int GREEN_LED_PIN  =6;   // the number of the GREEN LED pin D6

// Temperature & humidity MQTT topics
#define MQTT_PUB_TEMP "esp/dht/b/temp"
#define MQTT_PUB_HUM "esp/dht/b/humid"

// Fan control topics
// #define MQTT_PUB_FAN_OFF "esp/dht/b/fanoff"
// #define MQTT_PUB_FAN_ON_10 "esp/dht/b/fanon10"
// #define MQTT_SUB_FAN_RESUME "esp/dht/b/fanresume"

//default values
// int buttonState = 0;   // variable for reading the pushbutton status
// int latchState = 2;
// bool published = true; //don't keep bombarding the server once the message is sent
   
// Variables to hold sensor readings
float temp;
float hum;

EspMQTTClient client( //details from secrets.h
  WIFI_SSID,
  WIFI_PASS,
  MQTT_HOST,  // MQTT Broker server ip
  mqttClientName
);

void setup()
{
 //SHT31 setup
  Serial.begin(115200);
  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();

  //Button setup
  //initialize the LED pins as an output:
  // pinMode(RED_LED_PIN, OUTPUT);
  // pinMode(GREEN_LED_PIN, OUTPUT);
  // initialize the pushbutton pin as a pull-up input:
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.
  // pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Optional functionalities of EspMQTTClient
 // client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
 // client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
 // client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
 // client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "esp/dht/b/fanresume" and display received message to Serial
  // client.subscribe(MQTT_SUB_FAN_RESUME, [](const String & payload) {
  //   Serial.println(payload);
  // });
  // // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  // client.subscribe("mytopic/wildcardtest/#", [](const String & topic, const String & payload) {
  //   Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
  // });

  // Publish a message to "esp/dht/b/temp"
  //client.publish("esp/dht/b/temp", "This is a message"); // You can activate the retain flag by setting the third parameter to true
  client.publish("bathroom", "MQTT connection established"); // You can activate the retain flag by setting the third parameter to true

//   // Execute delayed instructions
//   client.executeDelayed(5 * 1000, []() {
//     client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
//   });
}

void loop(){
   client.loop();
  // // read the state of the pushbutton value:
  // buttonState = digitalRead(BUTTON_PIN);
  // // cycle three states 0 = Fan OFF | 1 = Timed Fan ON | 2 = Humidity trigger (default)
  // if(buttonState == LOW) {        // If button is being pressed
  //   Serial.println("button state: " + buttonState);
  //   published = false;
  //   Serial.println("published: " + published);
  //   latchState += 1;
  //   Serial.println("latchState: " + latchState);
  //   if (latchState > 2) {
  //     latchState = 0;
  //   // Publish an MQTT message on topic esp/dht/b/temp
  //   //uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());    
  //   // Publish a message to "esp/dht/b/temp"
  //   client.publish("esp/dht/b/temp", MQTT_PUB_TEMP); // You can activate the retain flag by setting the third parameter to true                            
  //   //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub1);
  //   //Serial.printf("Message: %.2f \n", temp);

  //   // Publish an MQTT message on topic esp/dht/b/humid
  //   uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
  //   client.publish("esp/dht/b/hum", MQTT_PUB_HUM);
  //   //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
  //   //Serial.printf("Message: %.2f \n", hum);
  //   } 
  // }
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str()); 
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());    
  client.publish("esp/dht/b/temp", MQTT_PUB_TEMP);
  client.publish("esp/dht/b/hum", MQTT_PUB_HUM);
}