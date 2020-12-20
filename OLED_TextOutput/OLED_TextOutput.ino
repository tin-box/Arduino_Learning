 #ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();
#include <TimeLib.h>
//#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "OLEDDisplayUi.h"  // Include the UI lib
#include "images.h"         // Include custom images

SSD1306Wire  display(0x3c, 5, 4);         // Initialize the OLED display using Wire library
OLEDDisplayUi ui ( &display );

// Instantiate Variables
float temp = 20;
float humidity = 90;
int screenW = 128;
int screenH = 64;
int screenCenterX = screenW/2;
int screenCenterY = ((screenH-16)/2)+8;   // top yellow part is 16 px height

String twoDigits(int digits){             // utility function for digital clock display: prints leading 0
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

//void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
//}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, timenow );
  Serial.println("Clock function " + timenow);
}

void temperatureReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){ 
  temp = temprature_sens_read();
  String tempnow = String("Temp:  " + String(temp) + "F");
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, tempnow );
  Serial.println("temp function " + tempnow );
}

void humidityReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  String humiditynow = String("Humidity: " + String(humidity) + "%" );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, humiditynow );
  Serial.println("Humidity function: " + humiditynow);
}
 
FrameCallback frames[] = { digitalClockFrame, temperatureReadingFrame, humidityReadingFrame }; // frames are the single views that slide in
int frameCount = 3;

void setup() {
  Serial.begin(9600);
  Serial.println("setup function");
  Serial.println();

  // The ESP is capable of rendering 60fps in 80Mhz mode, run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(30);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
    ui.setIndicatorPosition(TOP);       // TOP, LEFT, BOTTOM, RIGHT
  
  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);
  
  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_DOWN);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
 // ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  setTime(epoch);

}

void loop() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);

  }
}
