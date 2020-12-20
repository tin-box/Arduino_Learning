
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include <Adafruit_AM2320.h>
//#include "DHT.h"
//Adafruit_SSD1306 display(0x3c, 5, 4);
Adafruit_SSD1306 display(-1);
//Adafruit_AM2320 am2320 = Adafruite_AM2320();

void setup() {
  // Start wire library
  Wire.begin();

  // Init OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Init Temp/Humid sensor
  //am2320.begin();

}

void displayTempHumid(){
  delay(2000);

  // Read Humidity
  //float h = am2320.readHumidity();
  int h = 3;
  // Read Temp
  //float t = am2320.readTemperature();
  int t = 4;
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Test display");
  display.setCursor(0,10);
  display.print("Hum:    ");
  display.print(h);
  display.print("%");
  display.setCursor(0,20);
  display.print("Temp:    ");
  display.print(t);
  display.print(" C");
}

void loop() {
  // put your main code here, to run repeatedly:
  displayTempHumid();
  display.display();
}
