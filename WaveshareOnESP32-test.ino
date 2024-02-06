#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "math.h"
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>


// Define pins for display interface. You'll probably need to edit this for
// your own needs:


// ESP32 Pin assignment
#define TFT_DC 27 // Data/command
#define TFT_CS 5 // Chip select
#define TFT_BL 22 // Backlight control


// general config
int detents = 20; //physical detents per rotation
float virtualDetents = detents/2.5; //1 for smooth, 2.5 for quick, 5 for two-step sweep and 10 for instant menu jump
int circleRad = 120;
// void fillCircle(uint16_t -25, uint16_t 120, uint16_t circleRad, uint16_t 0x555555);
// //circle = Circle(-25,120,circleRad, fill=  0x555555 );
// void fillCircle(uint16_t -25, uint16_t 120, uint16_t circleRad, uint16_t 0x0045D0);
// //modeCircle = Circle(-25,120,circleRad, fill=  0x0045D0 );
float theta = M_PI;
int re = 35; // rotary encoder's pixel equivalent radius
int sweepRad = circleRad+re;
//String currentMode = 'humidity';
int thresholdHumidity = 80; // default relative humidity threshold in %
int countdownTimer = 300; // default time for timer mode in seconds = 5 minutes
int countdownMin = 60; // 1 minute
int countdownMax = 1800; // 30 minutes
int countdownIncrement = 15; // secs
int maxTimer = 3600; // default time for max mode in seconds = 1 hour
int maxTimerMin = 1800; // 30 minutes
int maxTimerMax = 7200; // 2 hours
int MaxTimeIncrement = 1800; // 30 minutes
// originX = 0-re
// originY = 120




// Display constructor for primary hardware SPI connection -- the specific
// pins used for writing to the display are unique to each board and are not
// negotiable. "Soft" SPI (using any pins) is an option but performance is
// reduced; it's rarely used, see header file for syntax if needed.
Adafruit_GC9A01A tft(TFT_CS, TFT_DC);

void setup() {
  // Serial.begin(9600);
  Serial.begin(115200);
  Serial.println("GC9A01A Test!");

  tft.begin();

#if defined(TFT_BL)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Backlight on
#endif // end TFT_BL
  Serial.println("Red BG fill");
  tft.fillScreen(GC9A01A_MAGENTA);

  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(50, 120);
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.println("Greg");
  //tft.setT
  // Serial.println(F("Benchmark                Time (microseconds)"));
  // delay(10);
  // Serial.print(F("Screen fill              "));
  // Serial.println(testFillScreen());
  // delay(500);

  // Serial.print(F("Text                     "));
  // Serial.println(testText());
  // delay(3000);

  // Serial.print(F("Lines                    "));
  // Serial.println(testLines(GC9A01A_CYAN));
  // delay(500);

  // Serial.print(F("Horiz/Vert Lines         "));
  // Serial.println(testFastLines(GC9A01A_RED, GC9A01A_BLUE));
  // delay(500);

  // Serial.print(F("Rectangles (outline)     "));
  // Serial.println(testRects(GC9A01A_GREEN));
  // delay(500);

  // Serial.print(F("Rectangles (filled)      "));
  // Serial.println(testFilledRects(GC9A01A_YELLOW, GC9A01A_MAGENTA));
  // delay(500);

  // Serial.print(F("Circles (filled)         "));
  // Serial.println(testFilledCircles(10, GC9A01A_MAGENTA));

  // Serial.print(F("Circles (outline)        "));
  // Serial.println(testCircles(10, GC9A01A_WHITE));
  // delay(500);

  // Serial.print(F("Triangles (outline)      "));
  // Serial.println(testTriangles());
  // delay(500);

  // Serial.print(F("Triangles (filled)       "));
  // Serial.println(testFilledTriangles());
  // delay(500);

  // Serial.print(F("Rounded rects (outline)  "));
  // Serial.println(testRoundRects());
  // delay(500);

  // Serial.print(F("Rounded rects (filled)   "));
  // Serial.println(testFilledRoundRects());
  // delay(500);

  // Serial.println(F("Done!"));
}

void loop(void) {
  
  // for(uint8_t rotation=0; rotation<4; rotation++) {
  //   tft.setRotation(rotation);
  //   testText();
  //   delay(1000);
  // }
}

// unsigned long testFillScreen() {
//   unsigned long start = micros();
//   tft.fillScreen(GC9A01A_BLACK);
//   yield();
//   tft.fillScreen(GC9A01A_RED);
//   yield();
//   tft.fillScreen(GC9A01A_GREEN);
//   yield();
//   tft.fillScreen(GC9A01A_BLUE);
//   yield();
//   tft.fillScreen(GC9A01A_BLACK);
//   yield();
//   return micros() - start;
// }

// unsigned long testText() {
//   tft.fillScreen(GC9A01A_BLACK);
//   unsigned long start = micros();
//   tft.setCursor(0, 0);
//   tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(1);
//   tft.println("Hello World!");
//   tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(2);
//   tft.println(1234.56);
//   tft.setTextColor(GC9A01A_RED);    tft.setTextSize(3);
//   tft.println(0xDEADBEEF, HEX);
//   tft.println();
//   tft.setTextColor(GC9A01A_GREEN);
//   tft.setTextSize(5);
//   tft.println("Groop");
//   tft.setTextSize(2);
//   tft.println("I implore thee,");
//   tft.setTextSize(1);
//   tft.println("my foonting turlingdromes.");
//   tft.println("And hooptiously drangle me");
//   tft.println("with crinkly bindlewurdles,");
//   tft.println("Or I will rend thee");
//   tft.println("in the gobberwarts");
//   tft.println("with my blurglecruncheon,");
//   tft.println("see if I don't!");
//   return micros() - start;
// }

// unsigned long testLines(uint16_t color) {
//   unsigned long start, t;
//   int           x1, y1, x2, y2,
//                 w = tft.width(),
//                 h = tft.height();

//   tft.fillScreen(GC9A01A_BLACK);
//   yield();

//   x1 = y1 = 0;
//   y2    = h - 1;
//   start = micros();
//   for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   x2    = w - 1;
//   for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   t     = micros() - start; // fillScreen doesn't count against timing

//   yield();
//   tft.fillScreen(GC9A01A_BLACK);
//   yield();

//   x1    = w - 1;
//   y1    = 0;
//   y2    = h - 1;
//   start = micros();
//   for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   x2    = 0;
//   for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   t    += micros() - start;

//   yield();
//   tft.fillScreen(GC9A01A_BLACK);
//   yield();

//   x1    = 0;
//   y1    = h - 1;
//   y2    = 0;
//   start = micros();
//   for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   x2    = w - 1;
//   for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   t    += micros() - start;

//   yield();
//   tft.fillScreen(GC9A01A_BLACK);
//   yield();

//   x1    = w - 1;
//   y1    = h - 1;
//   y2    = 0;
//   start = micros();
//   for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
//   x2    = 0;
//   for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);

//   yield();
//   return micros() - start;
// }

// unsigned long testFastLines(uint16_t color1, uint16_t color2) {
//   unsigned long start;
//   int           x, y, w = tft.width(), h = tft.height();

//   tft.fillScreen(GC9A01A_BLACK);
//   start = micros();
//   for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
//   for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);

//   return micros() - start;
// }

// unsigned long testRects(uint16_t color) {
//   unsigned long start;
//   int           n, i, i2,
//                 cx = tft.width()  / 2,
//                 cy = tft.height() / 2;

//   tft.fillScreen(GC9A01A_BLACK);
//   n     = min(tft.width(), tft.height());
//   start = micros();
//   for(i=2; i<n; i+=6) {
//     i2 = i / 2;
//     tft.drawRect(cx-i2, cy-i2, i, i, color);
//   }

//   return micros() - start;
// }

// unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
//   unsigned long start, t = 0;
//   int           n, i, i2,
//                 cx = tft.width()  / 2 - 1,
//                 cy = tft.height() / 2 - 1;

//   tft.fillScreen(GC9A01A_BLACK);
//   n = min(tft.width(), tft.height());
//   for(i=n; i>0; i-=6) {
//     i2    = i / 2;
//     start = micros();
//     tft.fillRect(cx-i2, cy-i2, i, i, color1);
//     t    += micros() - start;
//     // Outlines are not included in timing results
//     tft.drawRect(cx-i2, cy-i2, i, i, color2);
//     yield();
//   }

//   return t;
// }

// unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
//   unsigned long start;
//   int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

//   tft.fillScreen(GC9A01A_BLACK);
//   start = micros();
//   for(x=radius; x<w; x+=r2) {
//     for(y=radius; y<h; y+=r2) {
//       tft.fillCircle(x, y, radius, color);
//     }
//   }

//   return micros() - start;
// }

// unsigned long testCircles(uint8_t radius, uint16_t color) {
//   unsigned long start;
//   int           x, y, r2 = radius * 2,
//                 w = tft.width()  + radius,
//                 h = tft.height() + radius;

//   // Screen is not cleared for this one -- this is
//   // intentional and does not affect the reported time.
//   start = micros();
//   for(x=0; x<w; x+=r2) {
//     for(y=0; y<h; y+=r2) {
//       tft.drawCircle(x, y, radius, color);
//     }
//   }

//   return micros() - start;
// }

// unsigned long testTriangles() {
//   unsigned long start;
//   int           n, i, cx = tft.width()  / 2 - 1,
//                       cy = tft.height() / 2 - 1;

//   tft.fillScreen(GC9A01A_BLACK);
//   n     = min(cx, cy);
//   start = micros();
//   for(i=0; i<n; i+=5) {
//     tft.drawTriangle(
//       cx    , cy - i, // peak
//       cx - i, cy + i, // bottom left
//       cx + i, cy + i, // bottom right
//       tft.color565(i, i, i));
//   }

//   return micros() - start;
// }

// unsigned long testFilledTriangles() {
//   unsigned long start, t = 0;
//   int           i, cx = tft.width()  / 2 - 1,
//                    cy = tft.height() / 2 - 1;

//   tft.fillScreen(GC9A01A_BLACK);
//   start = micros();
//   for(i=min(cx,cy); i>10; i-=5) {
//     start = micros();
//     tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
//       tft.color565(0, i*10, i*10));
//     t += micros() - start;
//     tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
//       tft.color565(i*10, i*10, 0));
//     yield();
//   }

//   return t;
// }

// unsigned long testRoundRects() {
//   unsigned long start;
//   int           w, i, i2,
//                 cx = tft.width()  / 2 - 1,
//                 cy = tft.height() / 2 - 1;

//   tft.fillScreen(GC9A01A_BLACK);
//   w     = min(tft.width(), tft.height());
//   start = micros();
//   for(i=0; i<w; i+=6) {
//     i2 = i / 2;
//     tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
//   }

//   return micros() - start;
// }

// unsigned long testFilledRoundRects() {
//   unsigned long start;
//   int           i, i2,
//                 cx = tft.width()  / 2 - 1,
//                 cy = tft.height() / 2 - 1;

//   tft.fillScreen(GC9A01A_BLACK);
//   start = micros();
//   for(i=min(tft.width(), tft.height()); i>20; i-=6) {
//     i2 = i / 2;
//     tft.fillRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(0, i, 0));
//     yield();
//   }

//   return micros() - start;
// }
