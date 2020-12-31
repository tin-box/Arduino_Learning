//************************************************************
// Temperature Sensor network
//
// 1. Checks Temp and Humidity at a time between 4.5 and 6.5 seconds
// 2. Transmits data to Mesh, to node 0 if it exists, if not to all nodes
//  mtaft
//************************************************************
#include <TimeLib.h>                        // Time lib
#include "SSD1306Wire.h"                    // I2C driver for OLED display
#include "OLEDDisplayUi.h"                  // UserInterface lib
#include "images.h"                         // Custom images
#include "painlessMesh.h"                   // WiFi Mesh lib
#include "DHTesp.h"                         // DHT11 Sensor Lib


#define   LED_BUILTIN 2
#define   LED_5 5
#define   BUZZER_14 14
#define   LED_21 21
#define   LED_22 22
#define   LED_23 23
#define   OLEDDISPLAY_COLOR 1   //  BLACK = 0, WHITE = 1, INVERSE = 2
#define   JSON_DOC_SZ     128
#define   TASK_SECOND     5000
#define   MESH_PREFIX     "sensornet"
#define   MESH_PASSWORD   "thinkfast77"
#define   MESH_PORT       5555
#define   RADIO_CH        11
#define   STS_NUMBER      0


//int sts = STS_NUMBER;
DHTesp dht;
size_t IOSnodeId = 0;
int screenW = 128;
int screenH = 64;
int screenCenterX = screenW/2;
int screenCenterY = ((screenH-16)/2)-10;   // top yellow part is 16 px height
int   dhtPin      = 4;                     /* Pin number for DHT11 data pin */
int   ALARMTEMP   = 40.3;                  // !! Celsius!! Temperature for alarm
float temperature = -1000;                 // local board Temp in Celsius 
float humidity    = -1;                    // local board Humidity 
float temp[7]     = {69,69,69,69,69,69,69};
float hum[7]      = {49,49,49,49,49,49,49};
int   channels[6] = {2, 5, 14, 21, 22, 23};


SSD1306Wire  display(0x3c, 5, 4);          // Initialize the OLED display using Wire library
OLEDDisplayUi ui ( &display );             // Instantiate OLED object using I2C

Scheduler userScheduler; // to control user tasks
painlessMesh  mesh;

void sendMessage() ;    // Send Data out on network
void readTemp();        // Collect local sensor data  
void uiUpdate();        //

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskReadTemp( TASK_SECOND * 2, TASK_FOREVER, &readTemp);
Task taskUIUpdate ( 33, TASK_FOREVER, &uiUpdate);

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
  dtostrf(in, 4, 0, out);
  return String(out);
}

String oneDecimal(float f){
  char str[6];
  dtostrf(f, 5, 1, str);
  return String(str);
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

void lightInit() {
  Serial.println("\nInitializing and Testing Outputs");
  int i = 0;
  while (i < 6) {
    digitalWrite(channels[i], LOW);
    Serial.println("Channel: " + String(channels[i]) + " Testing");
    delay(500);
    digitalWrite(channels[i], HIGH);
    i++;
  }
}

void setOutput(float t, float h) {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(5, HIGH);
  //digitalWrite(14, LOW);
  digitalWrite(21, HIGH);
  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH);

  if (t > ALARMTEMP) {
    digitalWrite(23, LOW);  // Turn on red LED for overtemp
    digitalWrite(14, HIGH); // Sound buzzer for overtemp
  } else {
    digitalWrite(22, LOW);  // Turn on green LED within temp
    digitalWrite(14, LOW); // Sound buzzer for overtemp
  }
  
  if (mesh.getNodeList().size() > 0) {
    digitalWrite(5, LOW);   // Turn on Blue LED when Wirelss connects
  } else{
    digitalWrite(21, LOW);  // turn on Yellow LED when Wirelss is disconnected
  }
}

void uiUpdate() {
  ui.update();
}
void readTemp() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) {   // Check if any reads failed and exit early (to try again).
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return;
  }
  temperature = newValues.temperature;
  humidity = newValues.humidity;
  setOutput(temperature,humidity);        // set leds and buzzer based on latest sensor input  
}

void sendMessage() {
  DynamicJsonDocument jsonBuffer(JSON_DOC_SZ);            // Instantiate JSON doc and set size (128)
  JsonObject msg = jsonBuffer.to<JsonObject>();   // Instantiate JSON object
  String str;                                     // String that JSON object is cast to for transmission
                              // Add current data to the JSON object for transmission
  msg["station"]  = STS_NUMBER;
  msg["nodeid"]   = mesh.getNodeId();
  msg["temp"]     = temperature;
  msg["humid"]    = humidity;
    
  serializeJson(msg, str);    // Serialize the data into JSON String to be transmitted 
    
  if (IOSnodeId == 0)         // If we don't know the IOSnodeId yet broadcast the data to all nodes
    mesh.sendBroadcast(str);
  else                        // If we know IOSnodeId just send data to it. 
    mesh.sendSingle(IOSnodeId, str);
                              // Randomize data send period to reduce collisions 
  taskSendMessage.setInterval( random( TASK_SECOND * .83, TASK_SECOND * 1.27 ));
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Rcv src %u msg= %s\n", from, msg.c_str());
  
  DynamicJsonDocument jsonBuffer(JSON_DOC_SZ + msg.length());
  DeserializationError error = deserializeJson(jsonBuffer, msg);
  if (error) {
    Serial.printf("DeserializationError\n");
    return;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();
  int stsNum = root["station"];
  temp[stsNum] = (float(root["temp"])*1.8)+32;  // change from C to F
  hum[stsNum] = root["humid"];
  setOutput(temp[stsNum],hum[stsNum]);
  
  if (root["station"] == 0) {
    IOSnodeId = root["nodeid"];
    Serial.printf("IOSnode detected!\n");    
    Serial.printf("Rcvd src %u msg=%s\n", from, msg.c_str());
  }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void initMesh(){
  
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  // all types ( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE )
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, RADIO_CH );   // Set channel
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = "Time: " + String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(screenCenterX + x , screenCenterY + y, timenow );
  }

void envReadingFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){ 
  String EnvF[7]; 
  int xOffset = 0;
  /*
  for (int i = 0; i <= 6; i++){
    if (i = 0){
      EnvF[i] = String("IOS: " + String(temp[i]) + "F");
    } else {      
      EnvF[i] = String("STS" + String(i) + ": " + String(temp[i]) + "F");      
    }    
  }
  */
  
  EnvF[0] = String("IOS   : " + oneDecimal(temp[0]) + "F  " + floattostring(hum[0]) + "%");
  EnvF[1] = String("STS1: " + oneDecimal(temp[1]) + "F  " + floattostring(hum[1]) + "%");
  EnvF[2] = String("STS2: " + oneDecimal(temp[2]) + "F  " + floattostring(hum[2]) + "%");
  EnvF[3] = String("STS3: " + oneDecimal(temp[3]) + "F  " + floattostring(hum[3]) + "%");
  display->setColor(WHITE);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(DejaVu_Sans_12);
  display->drawString(x + xOffset,(screenCenterY - 10) + y, EnvF[0]);
  display->drawString(x + xOffset,(screenCenterY +  0) + y, EnvF[1]);
  display->drawString(x + xOffset,(screenCenterY + 10) + y,EnvF[2]);
  display->drawString(x + xOffset,(screenCenterY + 20) + y,EnvF[3]);
      
  }

void envReadingFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  String EnvF[7];
  int xOffset = 2;
  
  EnvF[4] = String("STS4: " + oneDecimal(temp[4]) + "F " + floattostring(hum[4]) + "%");
  EnvF[5] = String("STS5: " + oneDecimal(temp[5]) + "F " + floattostring(hum[5]) + "%");
  EnvF[6] = String("STS6: " + oneDecimal(temp[6]) + "F " + floattostring(hum[6]) + "%");
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(DejaVu_Sans_12);
  display->drawString(x + xOffset,(screenCenterY - 10) + y,EnvF[4]);
  display->drawString(x + xOffset,(screenCenterY +  0) + y,EnvF[5]);
  display->drawString(x + xOffset,(screenCenterY + 10) + y,EnvF[6]);
  }

 void outputReadingFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y){
  int remainingTimeBudget = ui.update();
  String outputnow = "Logging: " + String(remainingTimeBudget);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x , y, outputnow );
  }
  
//FrameCallback frames[] = { digitalClockFrame, envReadingFrame1, envReadingFrame2, outputReadingFrame }; // frames are the single views that slide in
FrameCallback frames[] = { envReadingFrame1, envReadingFrame2, outputReadingFrame }; // frames are the single views that slide in
int frameCount = 3;


void initScheduler(){
  userScheduler.addTask( taskReadTemp );
  taskReadTemp.enable();
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  userScheduler.addTask( taskUIUpdate );
  taskUIUpdate.enable();
}

void setup() {
  Serial.begin(115200);   // Initialize Serial port for monitoring
  configPinModes();       // Initialize IO pins
  initTemp();             // Initialize Temp Sensor
  lightInit();            // OpCheck lights and buzzer at startup
  initMesh();             // Start Mesh network
  initScheduler();        // Setup Scheduler Tasks

  ui.setTargetFPS(30);                  // Render 60fps/80Mhz mode, run 160Mhz mode or 30 fps
  ui.setActiveSymbol(activeSymbol);     // Customize the active and inactive symbol
  ui.setInactiveSymbol(inactiveSymbol);

  ui.setIndicatorPosition(BOTTOM);       // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorDirection(LEFT_RIGHT); // Define where first frame located in the bar.
  ui.setFrameAnimation(SLIDE_RIGHT);     // Transition used SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrames(frames, frameCount);     // Add frames
  // ui.setOverlays(overlays, overlaysCount);  // Add overlays
  ui.init();                           // Initialising the UI will init the display too.
  display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  const unsigned long seventyYears = 2208988800UL;  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;// subtract seventy years:
  setTime(epoch);

}

void loop() {
  mesh.update();           // Runs the user scheduler as well
  //ui.update();
}
