#include <avr/wdt.h>
#include <SPI.h>
#include "RF24.h"
#include <Bounce2.h>
#include </home/rakesh/Arduino/mainController/motor.h>
#include <HX711.h>

struct packet1{
  int button[5] = {1,1,1,1,1};
  int step;
  int toHome;
};

struct packet2{
  int tension;
  bool limitSw;
  bool reached;
};

HX711 scale1;
Motor m1(6,7);           //M1(SPEED, DIRECITON);
Bounce swLeft = Bounce();

packet1 toSend;
packet2 receivedData;

int tensionAB = 0;
int tensionBC = 0;
int swR = 1; 
int maxTension = 7000;
int minTension = 6000;
int count = 0;
unsigned long lastTensionReadTime = 0;
const unsigned long tensionReadInterval = 100;

unsigned long last_signal_time = 0;
unsigned long last_signal_timeB = 0;
unsigned long return_to_reset_time = 0;
RF24 radio(9, 10);
byte thisAddr[8] = "1112";
byte otherAddr[8] = "1113";

int state[7] = {1,1,1,1,1,1,1};

void softwareReset() {
  wdt_disable();
  wdt_enable(WDTO_15MS); 
  while (1);
}

void setup(){
  Serial.begin(115200);
  Serial1.begin(115200);

  swLeft.attach(A0, INPUT_PULLUP);
  swLeft.interval(10);

  scale1.begin(A3, A4);
  scale1.set_scale(49.07);
  scale1.set_offset(22591);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(otherAddr);
  radio.openReadingPipe(1, thisAddr);
  radio.setChannel(40);
  radio.startListening();
  // int st = millis();
  // while(millis()-st<5000){
  //   m1.control(255, 0);
  // }
  //delay(5000);
  // returnToHome();
}

void loop(){ 
  Serial.println("MANUAL");
  toSend.button[4] = 1;
  readSerial();
  dataAccuire();

  // while(state[6]==0){
  //   Serial.println("AUTO");
  //   toSend.button[4] = state[6];
  //   readSerial();
  //   dataAccuire();
  //   if(swLeft.fell()&&count<10){
  //     toSend.button[4] = 1;
  //     move();
  //     Serial.println(count);
  //     toSend.button[4] = 1;
  //     count++;
  //     hold();
  //     delay(10000);
  //   }

  //   if(count==10){
  //     toSend.button[4] = 1;
  //     fwt();
  //     // returnToHome();
  //     count = 0;
  //     checkReset();
  //   }
  //   hold();
  // }
  // hold();
}

void hold(){
  if(millis() - lastTensionReadTime >= tensionReadInterval){
    measureTension();
    lastTensionReadTime = millis();
  }
  
  // Serial.println(tensionAB);
  if(tensionAB>=minTension&&tensionAB<=maxTension){
    m1.control(0, 1);
  }
  else if(tensionAB<minTension){
    m1.control(200, 1);
  }
  else if(tensionAB>maxTension){
    m1.control(200, 0);
  }

}

void move(){
  m1.control(255, 0);
  delay(2000);
  
  toSend.step = 10;
  Serial.println("Moving");
  while(true){
    dataAccuire();
    // hold();
    m1.control(255, 0);

    // delay(5000);
    // break;
    
    if(receivedData.reached==1){
      Serial.println("moved");
      toSend.step = 0;
      fwt();
      break;
    }
  }
}

void returnToHome(){
  if(!noderHome()){
    checkReset();
  }
  if(!tensionA()){
    checkReset();
  }
}

int tensionA(){
  unsigned long startTime = millis();
  unsigned long timeoutMillis = 50000;
  Serial.println("Tensioning AB.");
  while (true) {
    measureTension(); 
    Serial.println(tensionAB);
    if (tensionAB >= minTension) {
      Serial.println("Target tensionAB reached.");
      m1.control(0, 1);
      return 1;
    }
    if (millis() - startTime >= timeoutMillis) {
      Serial.println("ErrorA: Target tension not reached within the specified timeout.");
      m1.control(0, 1);
      break;
    }
    m1.control(200, 1);
  }
  return 0;
}

int noderHome(){
  unsigned long startTime = millis();
  unsigned long timeoutMillis = 50000; 
  toSend.toHome = 1;
  Serial.println("Checking if Node reached home on BC.");
  while (millis() - startTime < timeoutMillis) {
    dataAccuire();
    if(receivedData.reached==1){
      toSend.toHome = 0;
      Serial.println("Node reached home on BC.");
      fwt();
      return true;
    }
  }
  Serial.println("ErrorH: Not returned to home in given timeout.");
  toSend.toHome = 0;
  return false;
}

void checkReset(){
  while(true){
    readSerial();
    // Serial.println(state[8]);
    // dataAccuire();
    if (state[4] == 0) {
      if (return_to_reset_time == 0) {
        return_to_reset_time = millis(); 
      } else {
        if (millis() - return_to_reset_time >= 3000) {
          Serial.println("Resetting");
          softwareReset();
          return_to_reset_time = 0;
        }
      }
    } else {
      return_to_reset_time = 0;
    }
  }
}

void dataAccuire(){
  swLeft.update();
  radio.stopListening();
  byte data1[sizeof(toSend)]; 
  memcpy(&data1, &toSend, sizeof(toSend));
  if(!radio.write(&data1, sizeof(data1))){
    // Serial.println("Failed");
  }
  radio.startListening(); 

  if(radio.available()){
    last_signal_time = millis();
    byte data2[sizeof(receivedData)];
    radio.read(&data2, sizeof(data2));
    memcpy(&receivedData, &data2, sizeof(data2));
    tensionBC = receivedData.tension;
    swR = receivedData.limitSw;
  }
  else{
    if (millis() - last_signal_time >= 5000) {
      // Serial.println("Data receive error");
    }
  }
  delay(10);  
}

void readSerial(){
  if(Serial1.available()){
    String rec = Serial1.readStringUntil('\n');
    // Serial.println(rec);
    for(int i=0; i<7; i++){
      if(rec[i]=='0'||rec[i]=='1'){
        state[i] = rec[i] -'0';
      }
      
    }
    for(int i = 0; i<4; i++){
      toSend.button[i] = state[i]; 
    }
    // toSend.button[4] = 2;
    // Serial.print(state[0]);
    // Serial.print(state[1]);
    // Serial.print(state[2]);
    // Serial.print(state[3]);
    // Serial.print(state[4]);
    // Serial.print(state[5]);
    // Serial.print(state[6]);
    // Serial.println();
  }
}

void measureTension(){
  tensionAB = scale1.get_units();
}

void fwt() {
  unsigned long startTime = millis();
  while (millis() - startTime <= 1000) {
    dataAccuire();
  }
}










