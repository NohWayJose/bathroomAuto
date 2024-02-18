//#include <ESP8266WiFi.h>
//Is Git working from VSCode?
//Yes, it is
#include <time.h>
#include <ESPPubSubClientWrapper.h>
#include <EasyButton.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <sstream>

//#include <string>

// Network & MQTT constants
const char* ssid = "FINBAR24";
const char* password = "61agwsso199ew";
const char* mqtt_server = "192.168.7.245";


// MQTT subscriptions
const char* heaterTopic          = "bathroom/mirror/heater"; //ON or OFF
const char* nightLightColTopic   = "bathroom/mirror/nightLightCol"; //string Hex RGB, eg 0062FF
const char* BGColTopic           = "bathroom/mirror/BGLightCol"; //string Hex RGB, eg FFFFFF
const char* fiveMinTickColTopic  = "bathroom/mirror/fiveMinTickCol"; //string Hex RGB
const char* fifteenMinTickColTopic  = "bathroom/mirror/fifteenMinTickCol"; //string Hex RGB
const char* minColTopic          = "bathroom/mirror/minCol"; //string Hex RGB
const char* hourColTopic         = "bathroom/mirror/hourCol"; //string Hex RGB
const char* lightTopic           = "bathroom/mirror/light"; //ON or OFF

// time
int myYear;
int myMonth;
int myDay;
int myHour;
int myMin;
int mySec;

//state
bool heaterState = false;
bool lightState = true;
bool wasPressed = false;

//various LED colour storage objects
class nightLightCol {
  public:
    int rr;
    int gg;
    int bb;
    nightLightCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};
class backgroundCol {
  public:
    int rr;
    int gg;
    int bb;
    backgroundCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};
class fiveMinTickCol {
  public:
    int rr;
    int gg;
    int bb;
    fiveMinTickCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};
class fifteenMinTickCol {
  public:
    int rr;
    int gg;
    int bb;
    fifteenMinTickCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};
class minCol {
  public:
    int rr;
    int gg;
    int bb;
    minCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};
class hourCol {
  public:
    int rr;
    int gg;
    int bb;
    hourCol(uint8_t red, uint8_t grn, uint8_t blu) {
      rr = red;
      gg = grn;
      bb = blu;
    }
};

//Instantiate classes, with default values
nightLightCol nightLightColObj(50, 0, 0); //comes on in response to a trigger from the phototransistor detecting low light
backgroundCol backgroundColObj(75, 75, 75); //comes on in response to the touch switch and overrides the night light and the clock
fiveMinTickCol fiveMinTickColObj(50, 20, 20); //the five minute/hour markers round the clock face
fifteenMinTickCol fifteenMinTickColObj(90, 0, 30); //the cardinal markers (15,30,45,60mins/12,3,6,9hours)
minCol minColObj(0, 0, 255); //the minute hand
hourCol hourColObj(0, 255, 0); //the hour hand

#define MIRROR_RELAY_PIN 5 //D1
#define NEOPIXEL_PIN 12 //D6
#define NUM_PIXELS 280
#define TOUCH_SW_PIN 14 //D5
#define SECONDS .001

int val;
int brightness = 56;
int LEDoff = 0;
int fiveMin = NUM_PIXELS/12;
int fifteenMin = fiveMin*3;
int lastMinute = 0;
int currentMinute;
int currentHour;
int hourPixel;
int minutePixel;

//NTP setup
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);
ESPPubSubClientWrapper client(mqtt_server); // ommiting ,port - defaults to 1883
Adafruit_NeoPixel NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//----------------------------------

// makes a number from two ascii hexa characters
uint32_t ahex2int(char a, char b){

    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}

std::array<uint32_t, 3> RGBstring2RGBarray(std::string RGB){ //Only takes hex string RRGGBB, no leading 0x & should return unint32_t component colours, ie, unsigned int in the range 0-255 in an array of R/G/B values
  std::array<uint32_t,3> RGB_array; //array declared
    using namespace std;
    string Rgb = RGB.substr(0,2); // grab each 2 character part of the RGB hex string in turn
    string rGb = RGB.substr(2,2);
    string rgB = RGB.substr(4,2);

    uint32_t red   = ahex2int(Rgb[0],Rgb[1]); 
    uint32_t green = ahex2int(rGb[0],rGb[1]);
    uint32_t blue  = ahex2int(rgB[0],rgB[1]);

    RGB_array[0] = red; //store the values in an array, where each colour component is accessible by the array index
    RGB_array[1] = green;
    RGB_array[2] = blue;
    return RGB_array; //array returned
}

// void callbackNightLight(char* nightLightColTopic, char * strPayload) {
//    //Need to think about this more. Use phototransistor to trigger it
// }

void callbackBG(char* BGColTopic, char * strPayload) {
  std::array<uint32_t, 3> colArr;
  Serial.println("Message Mirror light background colour change received");
  Serial.println("BGColTopic");
  Serial.println(strPayload);
  Serial.println("-------");
  colArr=RGBstring2RGBarray(strPayload);
  backgroundColObj.rr = colArr[0];
  backgroundColObj.gg = colArr[1];
  backgroundColObj.bb = colArr[2];
  LEDs('u');
}

void callbackFiveMinTick(char* fiveMinTickColTopic, char * strPayload) {
  std::array<uint32_t, 3> colArr;
  Serial.println("Message: Mirror five minute/hour tick mark colour change received");
  Serial.println("fiveMinTickColObj");
  Serial.println(strPayload);
  Serial.println("-------");
  colArr=RGBstring2RGBarray(strPayload);
  fiveMinTickColObj.rr = colArr[0];
  fiveMinTickColObj.gg = colArr[1];
  fiveMinTickColObj.bb = colArr[2];
  LEDs('u');
}

void callbackFifteenMinTick(char* fifteenMinTickColTopic, char * strPayload) {
  std::array<uint32_t, 3> colArr;
  Serial.println("Message: Mirror fifteen minute 1/4 hour tick mark colour change received");
  Serial.println("fifteenMinTickColObj");
  Serial.println(strPayload);
  Serial.println("-------");
  colArr=RGBstring2RGBarray(strPayload);
  fifteenMinTickColObj.rr = colArr[0];
  fifteenMinTickColObj.gg = colArr[1];
  fifteenMinTickColObj.bb = colArr[2];
  LEDs('u');
}

void callbackMinute(char* minColTopic, char * strPayload) {
  std::array<uint32_t, 3> colArr;
  Serial.println("Message Mirror minute hand colour change received");
  Serial.println("minColObj");
  Serial.println(strPayload);
  Serial.println("-------");
  colArr=RGBstring2RGBarray(strPayload);
  minColObj.rr = colArr[0];
  minColObj.gg = colArr[1];
  minColObj.bb = colArr[2];
  LEDs('u');
}

void callbackHour(char* hourColTopic, char * strPayload) {
  std::array<uint32_t, 3> colArr;
  Serial.println("Message: Mirror hour hand colour change received");
  Serial.println("hourColObj");
  Serial.println(strPayload);
  Serial.println("-------");
  colArr=RGBstring2RGBarray(strPayload);
  hourColObj.rr = colArr[0];
  hourColObj.gg = colArr[1];
  hourColObj.bb = colArr[2];
  LEDs('u');
}

void callbackLightState(char* lightTopic, char * strPayload) {  //Turn the light on/off via MQTT
  Serial.println("Message: Mirror light instruction received");
  Serial.println("lightTopic");
  Serial.println(strPayload);
  Serial.println("-------");
  if (strcmp(strPayload,"ON") == 0)
  {
    Serial.printf("Payload-len=%d, ON-Payload=\"%s\"\r\n", strlen(strPayload), strPayload);
    lightState = true;
    wasPressed = true;
  }
  else if (strcmp(strPayload,"OFF") == 0)
  {
    Serial.printf("Payload-len=%d, OFF-Payload=\"%s\"\r\n", strlen(strPayload), strPayload);
    lightState = false;
    wasPressed = true;
  }
}

void callbackHeater(char* strTopic, char * strPayload) { //React to MQTT signal to heat mirror (turn on relay)
  Serial.println("\r\nMessage ""Mirror instruction"" received");
  Serial.println(strTopic);
  Serial.println(strPayload);
  Serial.println("-------");

  if (strcmp(strPayload,"ON") == 0)
  {
    Serial.printf("Payload-len=%d, ON-Payload=\"%s\"\r\n", strlen(strPayload), strPayload);
    digitalWrite(MIRROR_RELAY_PIN, HIGH); 
  }
  else if (strcmp(strPayload,"OFF") == 0)
  {
    Serial.printf("Payload-len=%d, OFF-Payload=\"%s\"\r\n", strlen(strPayload), strPayload);
    digitalWrite(MIRROR_RELAY_PIN, LOW);
  }
}

void callbackLED(char* strTopic, char * strPayload) { //will be used for MQTT LED control - not used yet
  Serial.println("\r\nMessage ""LED instruction"" received");
  Serial.println(strTopic);
  Serial.println(strPayload);
  Serial.println("-------");
}

IRAM_ATTR void buttonPressed(){ //attached to interrupt and sets wasPressed to true
  wasPressed = true;
}

void calculatePixels(int currentHour, int currentMinute){
  int hr;
  if (currentHour >= 12){hr = currentHour - 12;} else {hr = currentHour;} //regress to 12hr clock
  hourPixel = NUM_PIXELS/12*hr;
  minutePixel = NUM_PIXELS/60*currentMinute;
  float fractionalHour = currentMinute/60;
  Serial.print("Fraction of hour: ");
  Serial.println(fractionalHour);
  int fractionalPixel = fractionalHour*NUM_PIXELS/12; //the proportion of the hour elapsed expressed as the number of pixels covered from the number representing one hour
  Serial.print("As pixels: ");
  Serial.println(fractionalPixel);
  hourPixel = hourPixel + fractionalPixel; // the hour marker plus the proportion elapsed
 

  //print time
  Serial.println();
  Serial.print("Time - ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.println(currentMinute);
  //print time in pixel locations
  Serial.print("     Hour pixel:");
  Serial.println(hourPixel);
  Serial.print("     Minute pixel:");
  Serial.println(minutePixel);  
}

void LEDs(char reason){

  if (reason == 'e'){
    Serial.println("Button was pressed");
    wasPressed = false;
    if (lightState){lightState = false;} else {lightState = true;}
  }else if (reason == 'm'){
    Serial.println("It's been a minute");
    lastMinute = currentMinute;
  }else if (reason == 'u'){
    Serial.println("colour updated");
  }
  if (lightState == false) { //cycle through each pixel, setting its colour
    Serial.println("  lightstate is true"); //we're the opposite way round as I needed to flip state early
    //NeoPixel.clear();
    for (int ON_pixel = 0; ON_pixel < NUM_PIXELS; ON_pixel++) { // for each pixel, set colour
      if (ON_pixel % fifteenMin == 0) { //make every 15 mins worth of pixel red
        NeoPixel.setPixelColor(ON_pixel, NeoPixel.Color(fifteenMinTickColObj.rr, fifteenMinTickColObj.gg, fifteenMinTickColObj.bb)); //200,45,45
        Serial.print("0");
      } else if (ON_pixel % fiveMin == 0) { //make every 5 mins worth of pixel pink
        NeoPixel.setPixelColor(ON_pixel, NeoPixel.Color(fiveMinTickColObj.rr, fiveMinTickColObj.gg, fiveMinTickColObj.bb)); //200,45,45
        Serial.print("+"); 
      } else if (lightState == false){ //all other pixels are dim white
        NeoPixel.setPixelColor(ON_pixel, NeoPixel.Color(backgroundColObj.rr, backgroundColObj.gg, backgroundColObj.bb)); //150,150,150
        Serial.print("-");
      }
      NeoPixel.setPixelColor(hourPixel, NeoPixel.Color(minColObj.rr, minColObj.gg, minColObj.bb)); // Green hour
      NeoPixel.setPixelColor(minutePixel, NeoPixel.Color(hourColObj.rr, hourColObj.gg, hourColObj.bb)); // Blue minute
    }
    Serial.println("");

  } else { //cycle through each pixel, setting its colour to off (apart from clock and dim hour markers)
    Serial.println("  lightstate is false"); //we're the opposite way round as I needed to flip state early
      for (int OFF_pixel = 0; OFF_pixel < NUM_PIXELS; OFF_pixel++) { // for each pixel, set colour to off, apart from 
        if (OFF_pixel % fifteenMin == 0) { //make every 5 mins worth of pixel red
        NeoPixel.setPixelColor(OFF_pixel, NeoPixel.Color(fifteenMinTickColObj.rr, fifteenMinTickColObj.gg, fifteenMinTickColObj.bb)); //200,45,45
        Serial.print("0");
      } else if (OFF_pixel % fiveMin == 0) { //make every 5 mins worth of pixel red
        NeoPixel.setPixelColor(OFF_pixel, NeoPixel.Color(fiveMinTickColObj.rr, fiveMinTickColObj.gg, fiveMinTickColObj.bb));
        Serial.print("+"); 
      } else { //all other pixels are off
        NeoPixel.setPixelColor(OFF_pixel, NeoPixel.Color(1, 1, 1)); //endimify the background
        Serial.print("-");
      }
      NeoPixel.setPixelColor(hourPixel, NeoPixel.Color(minColObj.rr, minColObj.gg, minColObj.bb)); // Green hour
      NeoPixel.setPixelColor(minutePixel, NeoPixel.Color(hourColObj.rr, hourColObj.gg, hourColObj.bb)); // Blue minute
      NeoPixel.setBrightness(128);
    }
    Serial.println("");
  }
  calculatePixels(currentHour, currentMinute);
  NeoPixel.show();  //turn on light
}


void setup() {
  Serial.begin(115200);
  NeoPixel.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  WiFi.begin(ssid, password);
  client.on(heaterTopic, callbackHeater);
  client.on(nightLightColTopic, callbackHeater);
  client.on(BGColTopic, callbackBG);
  client.on(fiveMinTickColTopic, callbackFiveMinTick);
  client.on(fifteenMinTickColTopic, callbackFifteenMinTick);
  client.on(minColTopic, callbackMinute);
  client.on(hourColTopic, callbackHour);
  client.on(lightTopic, callbackLightState);
  pinMode(MIRROR_RELAY_PIN, OUTPUT);
  pinMode(TOUCH_SW_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(TOUCH_SW_PIN), buttonPressed, FALLING);
  
  timeClient.begin();// Initialize a NTPClient to get time
  // GMT 0 = 0
  timeClient.setTimeOffset(0);
}

void loop() {
  client.loop();
  delay(1000*SECONDS);
  timeClient.update();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();

  if (currentMinute != lastMinute){ //react every minute to adjust clock (draw all the pixels)
    LEDs('m');
  }
  if (wasPressed){ //react if button pressed to adjust clock (draw all the pixels)
    LEDs('e');
  }
}







