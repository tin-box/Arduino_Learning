int x = 1;
void setup() {
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  Serial.println("Hey I just initialized");

}

void loop() {
  digitalWrite(12,HIGH);
  Serial.println("Digital 12 High");
  digitalWrite(13,LOW);
  Serial.println("Digital 13 Low");
  delay(500);
  
  digitalWrite(12,LOW);
  Serial.println("Digital 12 Low");
  digitalWrite(13,HIGH);
  delay(500);

}
