
//#ifdef__cplusplus
//extern "C" {
//#endif
//uint8_t temprature_sens_read();
//#ifdef__cplusplus
//}
//#endif
uint8_t temprature_sens_read();

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  float temp = (temprature_sens_read() - 32) / 1.8;

  Serial.print("Temperature: " + String(temp) + " C");
  delay(1000);
}
