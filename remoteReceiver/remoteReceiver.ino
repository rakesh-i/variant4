#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

byte address[8] = "1111";

int state[11] = {1,1,1,1,1,1,1,1,1,1,1};
size_t dataSize = sizeof(state);

unsigned long last_signal_time = 0;

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, address);
  radio.setChannel(10);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    last_signal_time = millis();
    radio.read(&state, dataSize);
  }
  else{
    if(millis()-last_signal_time >= 500){
      for(int i=0; i<11; i++){
        state[i] = 1;
      }
    }
  }
  Serial.print(state[0]);
  Serial.print(state[1]);
  Serial.print(state[2]);
  Serial.print(state[3]);
  // Serial.println(state[4]);
  // Serial.print(state[5]);
  // Serial.print(state[6]);
  // Serial.print(state[7]);
  Serial.print(state[8]);
  Serial.print(state[9]);
  Serial.println(state[10]);
  delay(50);
}
