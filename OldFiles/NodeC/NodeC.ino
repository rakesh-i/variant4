/* Code for NODE B */

#include <SPI.h>
#include "RF24.h"
#include <Bounce2.h>
#define HOME 2

unsigned long last_signal_time = 0;

RF24 radio(9, 10);

Bounce swHome = Bounce();

byte node_A_address[8] = "1112";
byte node_B_address[8] = "1113";

struct receivedPacket{
  int state[5] = {1,1,1,1,1};
  int step;
  int toHome;
};

int noSteps = 0;
int home = 0;

struct toSend{
  int tension;
  int limitSwitchA; 
};

toSend packetS;
receivedPacket packetR;

void setup() {
  Serial.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(node_A_address);
  radio.openReadingPipe(1, node_B_address);
  radio.setChannel(40);
  radio.startListening();

  swHome.attach(HOME, INPUT_PULLUP);
  swHome.interval(50);

}

void loop() {
  dataAquire();
  Serial.print(home);
  Serial.print(" ");
  Serial.println(noSteps);
}

void updateButtons(){
  swHome.update();
}

void dataAquire(){
  updateButtons();
  if ( radio.available()) {
    last_signal_time = millis();
    byte data1[sizeof(receivedPacket)];
    while (radio.available()) {                                 
      radio.read( &data1, sizeof(data1) );       
    }

    memcpy(&packetR, &data1, sizeof(packetR));
    home = packetR.toHome;
    noSteps = packetR.step;
    radio.stopListening();

    packetS.tension = analogRead(A6);
    packetS.limitSwitchA = swHome.read();

    byte data2[sizeof(packetS)];
    memcpy(&data2, &packetS, sizeof(packetS));

    radio.write( &data2, sizeof(data2) );

    
    radio.startListening();
  }
  else{
    if(millis()-last_signal_time>=5000){
      home = 0;
      noSteps = 0;
    }
  }
}