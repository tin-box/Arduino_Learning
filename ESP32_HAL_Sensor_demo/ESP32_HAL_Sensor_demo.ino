
int value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // Read hall sensor
  //value = hallRead();

  Serial.println("Hall Sensor: " + String(value));
  delay(1000);
}
