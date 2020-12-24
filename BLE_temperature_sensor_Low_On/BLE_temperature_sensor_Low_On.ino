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
int   dhtPin      = 4;                       /** Pin number for DHT11 data pin */
int   ALARMTEMP   = 40.3;                 // !! Celsius!! Temperature for alarm
bool  debugstate  = true;
float temperature = -1000;
float humidity    = -1;
int   channels[6] = {2, 5, 12, 21, 22, 23};

// BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int rec = 0;

void getTemp();
void BLETransfer(int16_t);

BLECharacteristic temperatureCharacteristic(
  BLEUUID((uint16_t)0x2A6E),
  BLECharacteristic::PROPERTY_READ |
  BLECharacteristic::PROPERTY_NOTIFY
);

//BLEDescriptor tempDescriptor(BLEUUID((uint16_t)0x2901));

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* ccsServer) {
      Serial.println("BLE Connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* ccsServer) {
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
  BLEDevice::init("TempSensor1");

  // Create the BLE Server
  BLEServer *ccsServer = BLEDevice::createServer();
  ccsServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *ccsEnvironment = ccsServer->createService(enviornmentService);

  // Create a BLE Characteristic
  ccsEnvironment->addCharacteristic(&temperatureCharacteristic);

  // Create a BLE Descriptor
  temperatureCharacteristic.addDescriptor(new BLE2902());

  BLEDescriptor TemperatureDescriptor(BLEUUID((uint16_t)0x2901));
  TemperatureDescriptor.setValue("Temperature -40-60°C");
  temperatureCharacteristic.addDescriptor(&TemperatureDescriptor);

  ccsServer->getAdvertising()->addServiceUUID(enviornmentService);

  // Start the service
  ccsEnvironment->start();

  // Start advertising
  ccsServer->getAdvertising()->start();
  Serial.println("BLE Waiting for client connection to notify...");
}

void loop() {
  getTemp();
  String beaconMsg;

  if (temperature == -1000)
  {
    if (rec > 5)
    {
      beaconMsg = "STS1SNA";
      Serial.println(beaconMsg);
      temperature = 100000;
    }
    return;
  }

  if (deviceConnected) {
    int16_t value;
    value = (temperature * 100);
    //Serial.println(value);
    //BLETransfer(value);        //*******************************
    temperatureCharacteristic.setValue((uint8_t*)&value, 2);
    temperatureCharacteristic.notify();
  }
  delay(2000);
}

void lightInit() {
  Serial.println("\nInitializing and Testing Outputs");
  int i = 0;
  while (i < 6) {
    digitalWrite(channels[i], LOW);
    Serial.println("Channel: " + String(channels[i]) + " Testing");
    delay(1000);
    digitalWrite(channels[i], HIGH);
    i++;
  }
}

void configPinModes() {
  Serial.println("\nConfiguring PinModes");
  int i = 0;  
  while (i < 6) {
    pinMode(channels[i], OUTPUT);
    Serial.println("pinMode: " + String(channels[i]) + " configured");
    i++;
  }
}

bool initTemp() {
  dht.setup(dhtPin, DHTesp::DHT11);      // Initialize temperature sensor
  Serial.println("DHT initiated");
}

void BLETransfer(int16_t val) {
  temperatureCharacteristic.setValue((uint8_t*)&val, 2);
  temperatureCharacteristic.notify();
}

void setOutput() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(5, HIGH);
  digitalWrite(12, LOW);
  digitalWrite(21, HIGH);
  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH);
  delay(500);

  if (temperature > ALARMTEMP) {
    digitalWrite(23, LOW);  // Turn on red LED for overtemp
    digitalWrite(12, HIGH); // Sound buzzer for overtemp
  } else {
    digitalWrite(22, LOW);  // Turn on green LED within temp
  }
  if (deviceConnected = true) {
    digitalWrite(5, LOW);   // Turn on Blue LED when BLE connects
  } else if (deviceConnected = false) {
    digitalWrite(21, LOW);  // turn on Yellow LED when BLE is disconnected
  }
}

void getTemp() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) {                                               // Check if any reads failed and exit early (to try again).
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
  }
  temperature = newValues.temperature;
  humidity = newValues.humidity;
  setOutput(); // set leds and buzzer based on temperature reading
  Serial.println(" Temp: " + String((temperature * 1.8) + 32) + "F Hum:  " + String(humidity) + "%");
}
