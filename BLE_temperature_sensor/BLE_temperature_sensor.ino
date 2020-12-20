#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHTesp.h"


#define LED_BUILTIN 2
#define LED_5 5
#define BUZZER_12 12
#define LED_21 21
#define LED_22 22
#define LED_23 23
#define enviornmentService BLEUUID((uint16_t)0x181A)

DHTesp dht;
int dhtPin = 4;                       /** Pin number for DHT11 data pin */
int ALARMTEMP = 85;                   // Temperature for alarm
bool debugstate = true;
float temperature = -1000;
float humidity = -1;

// BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int rec=0;

void getTemp();
void BLETransfer(int16_t);

BLECharacteristic temperatureCharacteristic(
  BLEUUID((uint16_t)0x2A6E), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);
BLECharacteristic humidityCharacteristic(
  BLEUUID((uint16_t)0x2A6F), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);
//BLEDescriptor tempDescriptor(BLEUUID((uint16_t)0x2901));

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("BLE Connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("BLE Disconnected");
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(debugstate);  
  configPinModes();
  initTemp();
  lightInit();
  Serial.println("\nESP32 DHT11 sensor\n");
  
  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pEnviornment = pServer->createService(enviornmentService);

  // Create a BLE Characteristic
  pEnviornment->addCharacteristic(&temperatureCharacteristic);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  temperatureCharacteristic.addDescriptor(new BLE2902());

  BLEDescriptor TemperatureDescriptor(BLEUUID((uint16_t)0x2901));
  TemperatureDescriptor.setValue("Temperature -40-60°C");
  temperatureCharacteristic.addDescriptor(&TemperatureDescriptor);

  pServer->getAdvertising()->addServiceUUID(enviornmentService);

  // Start the service
  pEnviornment->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  getTemp();
  String beaconMsg;
  
  if(temperature==-1000)
  {
    if(rec>5)
    {
      beaconMsg = "STS1SNA";
      Serial.println(beaconMsg);
      temperature = 100000;
    }
    return;
  }

  if (deviceConnected) {
    int16_t value;
    value = (temperature*100);
    //Serial.println(value);
    //BLETransfer(value);        //*******************************
    temperatureCharacteristic.setValue((uint8_t*)&value, 2);
    temperatureCharacteristic.notify();
  }
   delay(2000);
}

void lightInit(){
  Serial.println("Initializing and Testing Outputs");
  int i = 0;  
  int channels[6] = {2,5,12,21,22,23};  
    while (i < 6) {
    digitalWrite(channels[i],HIGH);
    Serial.println("Channel: " + String(channels[i]) + " Testing");
    delay(1000);
    digitalWrite(channels[i],LOW);
    i++;
  }
}

void configPinModes(){
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
}

bool initTemp() {
  dht.setup(dhtPin, DHTesp::DHT11);      // Initialize temperature sensor
  Serial.println("DHT initiated");
}

void BLETransfer(int16_t val){
  temperatureCharacteristic.setValue((uint8_t*)&val, 2);
  temperatureCharacteristic.notify();
}

void setOutput(){
  digitalWrite(LED_BUILTIN,LOW);  
  digitalWrite(5,LOW);
  digitalWrite(12,LOW);
  digitalWrite(21,LOW);
  digitalWrite(22,LOW);
  digitalWrite(23,LOW);
  delay(500);
  
    if (temperature > ALARMTEMP){
      digitalWrite(23,HIGH);
      digitalWrite(12,HIGH);  
  } else {
      digitalWrite(22,HIGH);  
  }
  if (deviceConnected = true){
      digitalWrite(5,HIGH);
      //Serial.println(deviceConnected);
  } else if (deviceConnected = false){
      digitalWrite(21,HIGH);
      //Serial.println(deviceConnected);
  }
}

void getTemp(){
  TempAndHumidity newValues = dht.getTempAndHumidity();
    if (dht.getStatus() != 0) {                                               // Check if any reads failed and exit early (to try again).
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
  }
  temperature = newValues.temperature;
  humidity = newValues.humidity;
  setOutput(); // set leds and buzzer based on temperature reading
  Serial.println(" Temp: " + String((temperature*1.8)+32) + "F Hum:  " + String(humidity) + "%");
}
