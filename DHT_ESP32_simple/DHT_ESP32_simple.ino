#include "DHTesp.h"
#include "Ticker.h"
#include "SimpleBLE.h"

#ifndef ESP32
#pragma message(FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

#define LED_BUILTIN 2
#define LED_5 5
#define BUZZER_12 12
#define LED_21 21
#define LED_22 22
#define LED_23 23

SimpleBLE ble;
String beaconMsg = "ESP00";
int rec=0;
bool debugstate = true;
DHTesp dht;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

TaskHandle_t tempTaskHandle = NULL;   /** Task handle for the light value read task */
Ticker tempTicker;                    /** Ticker for temperature reading */

bool tasksEnabled = false;            /** Flag if task should run */
int dhtPin = 4;                       /** Pin number for DHT11 data pin */
int ALARMTEMP = 85;                   // Temperature for alarm

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp() {
  byte resultValue = 0;
	dht.setup(dhtPin, DHTesp::DHT11);      // Initialize temperature sensor
	Serial.println("DHT initiated");

	xTaskCreatePinnedToCore(               // Start task to get temperature
			tempTask,                          /* Function to implement the task */
			"tempTask ",                       /* Name of the task */
			4000,                              /* Stack size in words */
			NULL,                              /* Task input parameter */
			5,                                 /* Priority of the task */
			&tempTaskHandle,                   /* Task handle. */
			1);                                /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {

    tempTicker.attach(10, triggerGetTemp);   // Start update of environment data every 10 seconds
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
}

void tempTask(void *pvParameters) {       // Task to read temperature from DHT11 sensor
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
			getTemperature();
		}
    // Got sleep again
		vTaskSuspend(NULL);
	}
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
/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool getTemperature() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
		if (dht.getStatus() != 0) {                                               // Check if any reads failed and exit early (to try again).
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		return false;
	}
  Serial.println(" Temp: " + String((newValues.temperature*1.8)+32) + "F Hum:  " + String(newValues.humidity) + "%");
	if ((newValues.temperature*1.8)+32 < ALARMTEMP){
    digitalWrite(22,LOW);
    digitalWrite(23,LOW);
    digitalWrite(12,LOW);
    delay(500);
    digitalWrite(22,HIGH);    
	} else {
    digitalWrite(23,LOW);
    digitalWrite(12,LOW);    
    digitalWrite(22,LOW);
    delay(500);
    digitalWrite(23,HIGH);
    digitalWrite(12,HIGH);
	}
	return true;
}

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(debugstate);
  Serial.println("\nESP32 DHT11 sensor\n");
  initTemp();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  lightInit();
  // Signal end of setup() to tasks
  tasksEnabled = true;
}

void loop() {
  if (!tasksEnabled) {
    // Wait 2 seconds to let system settle down
    delay(2000);
    // Enable task that will read values from the DHT sensor
    tasksEnabled = true;
    if (tempTaskHandle != NULL) {
			vTaskResume(tempTaskHandle);
		}
  }
  yield();
}
