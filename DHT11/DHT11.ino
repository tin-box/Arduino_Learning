#include <dht.h>
dht DHT;

#define DHT11_PIN 7

void setup(){
  Serial.begin(115200);
}

void loop(){
  int chk = DHT.read11(DHT11_PIN);
  Serial.println("Temperature:  " + 
                  String((1.8*DHT.temperature)+32) + 
                  " F");
  Serial.println("Humidity:     " +
                String(DHT.humidity) +
                " %");
    delay(5000);
}
