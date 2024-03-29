/* Code for NODE B */
#include <avr/wdt.h>
#include <SPI.h>
#include "RF24.h"
#include <Bounce2.h>
#include </home/rakesh/Arduino/NodeD/motor.h>
#include <HX711.h>


RF24 radio(9, 10);

int dir = 1;

byte node_A_address[8] = "NodeA";
byte node_B_address[8] = "NodeB";

int tensionAB = 0;
int tensionBC = -1;
int swRight = 1;
int swHome = 1;

Bounce swLeft = Bounce();

int toHome = 1;
int step = 5;

int maxTension = 5000;
int minTension = 4500;

int firstHome = 0;

unsigned long last_signal_time = 0;
unsigned long return_to_home_time = 0;
unsigned long return_to_reset_time = 0;

int timeoutMillis = 5000;


Motor m1(6,7);

int state[11];

HX711 scale1;

void softwareReset() {
  wdt_disable();
  wdt_enable(WDTO_15MS); 
  while (1);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println(F("Started"));

  swLeft.attach(A0, INPUT_PULLUP);

  scale1.begin(A3, A4);

  scale1.set_scale(49.07);

  scale1.set_offset(22591);

  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(node_A_address);
  radio.openReadingPipe(1, node_B_address);
  radio.setChannel(10);
  radio.startListening();
  // while(firstHome==0){
  //   sendData();
  //   checkToHome();
  // }
  // Serial.println(F("Entered Main loop"));
  // returnToHome();
}

void loop() {
  // readData();
  // swLeft.update();
  // if(swRight == 0){
  //   dir = -1;
  // }
  // if(swLeft.read() == 0){
  //   dir = 1;
  // }
  // Serial.print(dir);
  // Serial.print(" ");
  // Serial.print(swHome);
  // Serial.print(" ");
  // Serial.println(swRight);
  // sendData();
  // checkToHome();
  hold();
}

void hold(){
  tensionAB = tensionAB = scale1.get_units(1);
  Serial.println(tensionAB);
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

void sendData(){
  if (radio.available()) {
    last_signal_time = millis();
    while (radio.available()) {                                 
      radio.read( &state, sizeof(state) );       
    }
    if(state[10]==0){
      Serial1.print(0);
      Serial1.print(state[1]);
      Serial1.print(state[2]);
      Serial1.print(state[3]);
      Serial1.print(state[4]);
      Serial1.print(state[5]);
      Serial1.print(state[6]);
      Serial1.print(state[7]);
      Serial1.print(toHome);
      Serial1.print(step);
      Serial1.println();
    }
    else{
      Serial1.print(state[0]);
      Serial1.print(state[1]);
      Serial1.print(state[2]);
      Serial1.print(state[3]);
      Serial1.print(state[4]);
      Serial1.print(state[5]);
      Serial1.print(state[6]);
      Serial1.print(state[7]);
      Serial1.print(toHome);
      Serial1.print(step);
      Serial1.println();
    }
    delay(50);
  }
  else{
    if (millis() - last_signal_time >= 1000) {
      for(int i=0; i<10; i++){
      state[i] = 1;
      }
      Serial1.print(state[0]);
      Serial1.print(state[1]);
      Serial1.print(state[2]);
      Serial1.print(state[3]);
      Serial1.print(state[4]);
      Serial1.print(state[5]);
      Serial1.print(state[6]);
      Serial1.print(state[7]);
      Serial1.print(0);
      Serial1.print(0);
      Serial1.println();
      delay(50);
    }
  }
  
}

void readData(){
  // if(Serial1.available())
  // {
  //   String received = Serial1.readStringUntil('\n');
  //   splitBySpace(received);
  // }
}

void checkToHome(){
  if (state[9] == 0) {
    if (return_to_home_time == 0) {
      return_to_home_time = millis(); 
    } else {
      if (millis() - return_to_home_time >= 3000) {
        Serial.println("Returning to Home");
        returnToHome();
        return_to_home_time = 0;
      }
    }
  } else {
    return_to_home_time = 0;
  }
}


void checkReset(){
  while(true){
    sendData();
    if (state[8] == 0) {
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

void splitBySpace(String inputString){
  int spaceIndex1 = inputString.indexOf(' ');

  int spaceIndex2 = inputString.indexOf(' ', spaceIndex1 + 1);

  String tensionString = inputString.substring(0, spaceIndex1);
  String swRightString = inputString.substring(spaceIndex1 + 1, spaceIndex2);
  String swHomeString = inputString.substring(spaceIndex2 + 1);

  tensionBC = tensionString.toInt();
  swRight = swRightString.toInt();
  swHome = swHomeString.toInt();
}

void returnToHome(){
  toHome = 1;
  step = 0;
  // if(!checkNodeCHome()){
  //   checkReset();
  // }
  // if(!tensionB()){
  //   checkReset();
  // }
  if(!tensionA()){
    checkReset();
  }
  firstHome = 1;
}

int checkNodeCHome(){
  unsigned long startTime = millis();
  unsigned long timeoutMillis = 5000; 
  
  Serial.println("Checking if Node reached home on BC.");
  while (millis() - startTime < timeoutMillis) {
    sendData();
    readData();
    if(swHome==0){
      toHome = 0;
      return true;
      break;
    }
  }
  Serial.println("ErrorH: Not returned to home in given timeout.");
  return false;
}

int tensionA(){
  unsigned long startTime = millis();
  Serial.println("Tensioning AB.");
  while (true) {
    int currentTension = measureTensionA(); 
    if (currentTension >= minTension) {
      Serial.println("Target tensionAB reached.");
      return 1;
      break;
    }

    if (millis() - startTime >= timeoutMillis) {
      Serial.println("ErrorA: Target tension not reached within the specified timeout.");
      m1.control(0, 1);
      break;
    }

    // int tensionDifference = maxTension - currentTension;
    m1.control(200, 1);
  }
  return 0;
}


int tensionB(){
  unsigned long startTime = millis();
  Serial.println("Tensioning BC.");
  while (true) {
    int currentTension = measureTensionB(); 
    if(tensionBC==-1){
      Serial.println("ErrorT: Transmission interrupted.");
      return 0;
    }
    if (currentTension >= minTension) {
      Serial.println("Target tensionBC reached.");
      return 1;
    }

    if (millis() - startTime >= timeoutMillis) {
      Serial.println("ErrorB: Target tension not reached within the specified timeout.");
      return 0;
    }

    // int tensionDifference = maxTension - currentTension;
    // m1.control(200, 1);
  }
  return 0;
}


int measureTensionA(){
  readData();
  tensionAB = scale1.get_units(1);
  return tensionAB;
}

int measureTensionB(){
  readData();
  return tensionBC;
}


