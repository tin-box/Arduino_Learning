 #ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();
#include <TimeLib.h>
#include "SSD1306Wire.h"                   // legacy include: `#include "SSD1306.h"`
#include "OLEDDisplayUi.h"                 // Include the UI lib
#include "images.h"                        // Include custom images

SSD1306Wire  display(0x3c, 5, 4);         // Initialize the OLED display using Wire library
OLEDDisplayUi ui ( &display );

// Instantiate Variables
float temp = 20;
float humidity = 78;
int screenW = 128;
int screenH = 64;
int screenCenterX = screenW/2;
int screenCenterY = ((screenH-16)/2)+10;   // top yellow part is 16 px height

String twoDigits(int digits){             // utility function for digital clock display: prints leading 0
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

String floattostring(float in){
  char out[15];
  dtostrf(in, 4, 1, out);
  return String(out);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = "Time: " + String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, timenow );
  }

void temperatureReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){ 
  temp = temprature_sens_read();
  String tempnow = String("Temp:  " + floattostring(temp * .6) + "F");
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, tempnow );
  }

void humidityReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  String humiditynow = "Humidity: " + floattostring(humidity) + "%" ;
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(screenCenterX + x , screenCenterY + y, humiditynow );
  }

 void outputReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  int remainingTimeBudget = ui.update();
  String outputnow = "Logging: " + String(remainingTimeBudget) + "ms\n STS6 volume low"  ;
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(screenCenterX + x , (screenCenterY - 5) + y, outputnow );
  }
  
FrameCallback frames[] = { digitalClockFrame, temperatureReadingFrame, humidityReadingFrame, outputReadingFrame }; // frames are the single views that slide in
int frameCount = 4;

void setup() {
  Serial.begin(9600);
  //Serial.println();

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
    
    delay(remainingTimeBudget);

  }
}
