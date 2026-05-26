
// #define & #include lines mustn't end with ;

#include "Wire.h" //I2C library
#include "SHT31.h" //I2C sensor library
#include <ESP8266WiFi.h>
#include <Ticker.h> // non-blocking delay library
#include <AsyncMqtt_Generic.h>
#include <stdio.h>

#define WIFI_SSID "FINBAR24"
#define WIFI_PASSWORD "61agwsso199ew"

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 7, 245)
#define MQTT_PORT 1883
String hostname = "BathroomHumidTemp";

// Temperature & humidity MQTT topics
#define MQTT_PUB_TEMP "bathroom/temp"
#define MQTT_PUB_HUM "bathroom/humidity"

// Fan control PUB
#define MQTT_PUB_FAN_STATE "esp/dht/b/fanState"
// Fan control SUB
#define MQTT_SUB_FAN_STATE "esp/dht/b/fanState"
#define MQTT_SUB_FAN_RESUME "esp/dht/b/fanresume"
#define MQTT_SUB_FAN_STATENR "esp/dht/b/fanStateNR"

// I2C & DHT
uint32_t start;
uint32_t stop;
// Initialize DHT sensor
SHT31 sht;

// pin assignments:
const int BUTTON_PIN = D7;  // the number of the pushbutton pin
const int RED_LED_PIN =  D6;   // the number of the RED LED pin
const int GREEN_LED_PIN =  D5;   // the number of the GREEN LED pin

int buttonState = 0;   // variable for reading the pushbutton status
int latchState = 2;
bool published = true; //don't keep bombarding the server once the message is sent
   
// Variables to hold sensor readings
float temp;
float hum;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousReportMillis = 0;   // Stores last time temperature & humidity were published
unsigned long previousDebounceMillis = 0; // Stores last time button was pressed
unsigned long previousPubMillis = 0;      // Stores last time button press was published
const long    reportInterval = 10000;     // reportInterval at which to publish sensor readings
const long    debounceInterval = 500;     // minimum button press time
const long    pubInterval = 1000;         // minimum button press report interval

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub1 = mqttClient.subscribe(MQTT_SUB_FAN_RESUME, 2);

  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub1);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  payload:  ");
  Serial.println(payload);
//  Serial.print("  qos: ");
//  Serial.println(properties.qos);
//  Serial.print("  dup: ");
//  Serial.println(properties.dup);
//  Serial.print("  retain: ");
//  Serial.println(properties.retain);
//  Serial.print("  len: ");
//  Serial.println(len);
//  Serial.print("  index: ");
//  Serial.println(index);
//  Serial.print("  total: ");
//  Serial.println(total);
  if (String(topic) == MQTT_SUB_FAN_STATENR){ //esp/dht/b/fanStateNR
    if (payload == 'fanOff'){
      latchState = 0;
      Serial.printf("Fan off. latchState: %i ", latchState);
      published = false;
      digitalWrite(RED_LED_PIN, HIGH); // turn off red LED
      digitalWrite(GREEN_LED_PIN, LOW); // turn off green LED
    } else if (String(payload) == 'fanOn10'){
      latchState = 1;
      Serial.printf("Fan on for a period. latchState: %i ", latchState);
      published = false;
      digitalWrite(RED_LED_PIN, LOW); // turn off red LED
      digitalWrite(GREEN_LED_PIN, HIGH); // turn off green LED
    } else if (String(payload) == 'humidityTrigger'){
      latchState = 2;
      Serial.printf("Fan controlled by humidity threshold. latchState: %i ", latchState);
      published = false;
      digitalWrite(RED_LED_PIN, LOW); // turn off red LED
      digitalWrite(GREEN_LED_PIN, LOW); // turn off green LED
    } 
  }
}


void stopFan(){
    // Publish an MQTT message on topic esp/dht/b/fanOff
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_FAN_STATE, 1, true, String("fanOff").c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_FAN_STATE, packetIdPub2);
    Serial.printf("Message: %s \n", "Fan OFF & red LED on. <");
    digitalWrite(RED_LED_PIN, HIGH); // turn on red LED
    digitalWrite(GREEN_LED_PIN, LOW); // turn off green LED
    published = true; 
}

void manFan(){
    // Publish an MQTT message on topic esp/dht/b/fanOn10
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_FAN_STATE, 1, true, String("fanOn10").c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_FAN_STATE, packetIdPub2);
    Serial.printf("Message: %s \n", "Fan ON for 10 minutes & green LED. <==");
    digitalWrite(GREEN_LED_PIN, HIGH); // turn on greenLED
    digitalWrite(RED_LED_PIN, LOW); // turn off red LED
    published = true;
}

void humidityFan(){
    // Publish an MQTT message on topic esp/dht/b/fanState
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_FAN_STATE, 1, true, String("humidityTrigger").c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_FAN_STATE, packetIdPub2);
    Serial.printf("Message: %s \n", "Fan resume humidity threshold switch & both LEDs off. <===");
    digitalWrite(RED_LED_PIN, LOW); // turn off red LED
    digitalWrite(GREEN_LED_PIN, LOW); // turn off green LED  
    published = true;
}

void setup() {
  Serial.begin(115200);
  // initialize the LED pins as an output:
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  // initialize the pushbutton pin as an pull-up input:
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  
  Serial.println();
  Wire.begin();
  sht.begin(0x44);    //Sensor I2C Address
  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();

  if ((currentMillis - previousPubMillis >= pubInterval) && (published == true)){ //reset the 'published' flag after 1000ms
    previousPubMillis = currentMillis;
    published = false;
  }

  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTON_PIN);
  // cycle three states 0 = Fan OFF | 1 = Timed Fan ON | 2 = Humidity trigger (default)
  if((buttonState == LOW) && (currentMillis - previousDebounceMillis >= debounceInterval) && (published == false)) { // If button is being pressed
    // reset the debounce timer
    previousDebounceMillis = currentMillis;
    latchState += 1;
    if (latchState > 2) {
      latchState = 0;
    }

    if        (latchState == 0){  // 0 = Fan OFF
      stopFan();
    } else if (latchState == 1){  // 1 = Timed Fan ON (time controlled in NodeRed flow
      manFan();
    }else if  (latchState == 2){  // 2 = Humidity trigger (default)
      humidityFan();
    }
  }
    
   // Every X number of seconds (reportInterval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousReportMillis >= reportInterval) {
    // Save the last time a new reading was published
    previousReportMillis = currentMillis;
    sht.read();
    // New DHT sensor readings
    hum = sht.getHumidity();
    // Read temperature as Celsius (the default)
    temp = sht.getTemperature();
    
    // Publish an MQTT message on topic esp/dht/b/temp
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub3);
    Serial.printf("Message: %.2f \n", temp);

    // Publish an MQTT message on topic esp/dht/b/humid
    uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub4);
    Serial.printf("Message: %.2f \n", hum);
  }
}
