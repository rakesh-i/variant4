// Sender2 code

#include <SPI.h>
#include "RF24.h"
RF24 radio(9, 10);

byte node_A_address[8] = "Sender2";
byte node_B_address[8] = "NodeC";

int tension = -1;
int swStateA = 1;
int swStateB = 1;

struct dataPacket1{
  int button[8] = {1,1,1,1,1,1,1,1};
  int step = 0;
  int toHome = 0;
};

struct dataPacket2{
  int tension;
  int limitSwitchA;
  int limitSwitchB;
};

dataPacket1 toSend;

unsigned long last_signal_timeA = 0;
unsigned long last_signal_timeB = 0;

void setup() {
  Serial.begin(9600);
  
  Serial.println(F("Starting sender2"));

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(node_B_address);
  radio.openReadingPipe(1, node_A_address);
  radio.setChannel(40);
  radio.startListening();
}

void readbuttons(){
  if(Serial.available()){
    last_signal_timeB = millis();
    String received = Serial.readStringUntil('\n');
    for(int i=0; i<8; i++){
      toSend.button[i] = received[i]-'0';
      // Serial.print(received[i]);
    }
    // Serial.println();
      

    toSend.step = received[8] - '0';
    toSend.toHome = received[9] - '0';
  }
  else{
    if(millis()-last_signal_timeB>=3000){
      for(int i=0; i<8; i++){
        toSend.button[i] = 1;
      }
      toSend.step = 0;
      toSend.toHome = 0;
    }
  }
}

void loop() {
  readbuttons();
  radio.stopListening();                              
  unsigned long start_time = micros();   
  // Serial.print(toSend.button[0]);
  // Serial.print(toSend.button[1]);
  // Serial.print(toSend.button[2]);
  // Serial.print(toSend.button[3]);
  // Serial.print(toSend.button[4]);
  // Serial.print(toSend.button[5]);
  // Serial.print(toSend.button[6]);
  // Serial.println(toSend.button[7]);

  byte data1[sizeof(toSend)]; 
  memcpy(&data1, &toSend, sizeof(toSend));

  if (!radio.write( &data1, sizeof(data1) )) {
    Serial.print(tension);
    Serial.print(" ");
    Serial.print(swStateA);
    Serial.print(" ");
    Serial.println(swStateB);
  }

  radio.startListening();       

  if (radio.available()) {
      last_signal_timeA = millis();
      dataPacket2 receivedData;
      byte data2[sizeof(receivedData)];
      while(radio.available()){
        radio.read(&data2, sizeof(data2));
      }
      memcpy(&receivedData, &data2, sizeof(data2));
      Serial.print(receivedData.tension);
      Serial.print(" ");
      Serial.print(receivedData.limitSwitchA);
      Serial.print(" ");
      Serial.println(receivedData.limitSwitchB);
      tension = receivedData.tension;
      swStateA = receivedData.limitSwitchA;
      swStateB = receivedData.limitSwitchB;

  } 
  else {
      if (millis() - last_signal_timeA >= 5000) {
        tension = -1;
        swStateA = -1;
        swStateB = -1;
      }
  }
  delay(50);
}