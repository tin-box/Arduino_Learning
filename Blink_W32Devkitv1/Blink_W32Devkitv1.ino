#define LED_BUILTIN 2
#define LED_5 5
#define LED_12 12
#define LED_21 21
#define LED_22 22
#define LED_23 23

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(5,HIGH);
  //delay(1000);
  digitalWrite(12,HIGH);
  digitalWrite(21,HIGH);
  delay(1000);
  digitalWrite(21,LOW);
  digitalWrite(22,HIGH);
  delay(1000);
  digitalWrite(22,LOW);
  digitalWrite(23,HIGH);
  delay(1000);                       // wait for a second
  digitalWrite(5,LOW);
  digitalWrite(23,LOW);
  digitalWrite(12,LOW);  
  delay(1000);                       // wait for a second
}
