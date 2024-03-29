/* Code for NODE B */

#include <SPI.h>
#include "RF24.h"
#include <Bounce2.h>
RF24 radio(9, 10);
#include </home/rakesh/Arduino/NodeD/motor.h>

byte node_A_address[8] = "Sender2";
byte node_B_address[8] = "NodeC";

struct receivedPacket{
  int state[9] = {1,1,1,1,1,1,1,1};
  int step = 0;
  int toHome = 0;
};

int dir = 1;

Bounce swF = Bounce();
Bounce swB = Bounce();

Motor m1(3, 6);
Motor m2(5, 4);

unsigned long last_signal_time = 0;

int rec[9] = {1,1,1,1,1,1,1,1};

void setup() {
  Serial.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));

  swF.attach(A6,INPUT_PULLUP);
  swB.attach(A7,INPUT_PULLUP);


  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(node_A_address);
  radio.openReadingPipe(1, node_B_address);
  radio.setChannel(40);
  radio.startListening();
}

void loop() {
  readButtons();
  
  while(rec[0]==0){
    readButtons();
    swF.update();
    swB.update();
    if(swF.read()==0){
      dir = -1;
    }
    if(swB.read()==0){
      dir = 1;
    }
    if(dir==1){
      m1.control(250, 1);
      if (rec[6] == 0 && rec[7] == 1){
        m2.control(150, 0);
      }
      else if (rec[6] == 1 && rec[7] == 0){
        m2.control(150, 1);
      }
      else{
        m2.control(0, 1);
      }
    }
    if(dir==-1){
      m1.control(250, 0);
      if (rec[6] == 0 && rec[7] == 1){
        m2.control(150, 0);
      }
      else if (rec[6] == 1 && rec[7] == 0){
        m2.control(150, 1);
      }
      else{
        m2.control(0, 1);
      }
    }
  }



  if (rec[4] == 0 && rec[5] == 1)
    {
      m1.control(150, 1);
    }
    else if (rec[4] == 1 && rec[5] == 0)
    {
      m1.control(150, 0);
    }
    else
    {
      m1.control(0, 0);
    }

    if (rec[6] == 0 && rec[7] == 1)
    {
      m2.control(150, 0);
    }
    else if (rec[6] == 1 && rec[7] == 0)
    {
      m2.control(150, 1);
    }
    else
    {
      m2.control(0, 1);
    }
}

void readButtons(){
  if ( radio.available()) {
    // Serial.println("dd");
    last_signal_time = millis();
    byte data[sizeof(receivedPacket)];
  
    while (radio.available()) {                                 
      radio.read( &data, sizeof(data) );       
    }

    receivedPacket packet;
    memcpy(&packet, &data, sizeof(packet));
    for(int i=0; i<9; i++){
      if(packet.state[i]==1||packet.state[i]==0)
      rec[i] = packet.state[i];
    }
    // Serial.println()
    // Serial.print(packet.state[0]);
    // Serial.print(packet.state[1]);
    // Serial.print(packet.state[2]);
    // Serial.print(packet.state[3]);
    // Serial.print(packet.state[4]);
    // Serial.print(packet.state[5]);
    // Serial.print(packet.state[6]);
    // Serial.print(packet.state[7]);
    // Serial.println();
  }

  else{
    if (millis() - last_signal_time >= 1000) {
       for(int i=0; i<8; i++){
        rec[i] = 1;
       }
    }
  }
}