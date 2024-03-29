#include <Arduino.h>

#include <SPI.h>
#include <RF24.h>
#include <Bounce2.h>

#define LUP 4
#define LDOWN 3
#define LLEFT 6
#define LRIGHT 5
#define RUP 18
#define RDOWN 19
#define RLEFT 21
#define RRIGHT 20
#define SELECT 0
#define BACK 2
#define SW 10

int state[11];
size_t dataSize = sizeof(state);

Bounce lUp = Bounce();
Bounce lDown = Bounce();
Bounce lLeft = Bounce();
Bounce lRight = Bounce();

Bounce rUp = Bounce();
Bounce rDown = Bounce();
Bounce rLeft = Bounce();
Bounce rRight = Bounce();

Bounce select = Bounce();
Bounce back = Bounce();

Bounce sw = Bounce();

RF24 radio(8, 9); // CE, CSN

byte address[8] = "1111";

void setup()
{
  SPI.begin();
  Serial.begin(115200);
  for (int i = 0; i < 10; i++)
  {
    state[i] = 1;
  }
  // Left side
  lUp.attach(LUP, INPUT_PULLUP);
  lUp.interval(5);
  lDown.attach(LDOWN, INPUT_PULLUP);
  lDown.interval(5);
  lLeft.attach(LLEFT, INPUT_PULLUP);
  lLeft.interval(5);
  lRight.attach(LRIGHT, INPUT_PULLUP);
  lRight.interval(5);
  // Right side
  rUp.attach(RUP, INPUT_PULLUP);
  rUp.interval(5);
  rDown.attach(RDOWN, INPUT_PULLUP);
  rDown.interval(5);
  rLeft.attach(RLEFT, INPUT_PULLUP);
  rLeft.interval(5);
  rRight.attach(RRIGHT, INPUT_PULLUP);
  rRight.interval(5);
  // interact
  select.attach(SELECT, INPUT_PULLUP);
  select.interval(5);
  back.attach(BACK, INPUT_PULLUP);
  back.interval(5);

  sw.attach(SW, INPUT_PULLUP);
  sw.interval(5);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.setChannel(10);
}

void readButtons()
{
  lUp.update();
  lDown.update();
  lLeft.update();
  lRight.update();
  state[0] = lUp.read();
  state[1] = lDown.read();
  state[2] = lLeft.read();
  state[3] = lRight.read();

  rUp.update();
  rDown.update();
  rLeft.update();
  rRight.update();
  state[4] = rUp.read();
  state[5] = rDown.read();
  state[6] = rLeft.read();
  state[7] = rRight.read();

  select.update();
  back.update();
  state[8] = select.read();
  state[9] = back.read();

  sw.update();
  state[10] = sw.read();
}

void loop()
{
  readButtons();
  unsigned long start_time = micros();      
  if (!sendData()) {
    Serial.println(F("failed"));
  }
  delay(50);
  // Serial.print(state[0]);
  // Serial.print(state[1]);
  // Serial.print(state[2]);
  // Serial.print(state[3]);
  // Serial.print(state[4]);
  // Serial.print(state[5]);
  // Serial.print(state[6]);
  // Serial.print(state[7]);
  // Serial.print(state[8]);
  // Serial.print(state[9]);
  // Serial.print(state[10]);
  // Serial.println();
}

bool sendData()
{
  return radio.write(&state, dataSize);
}

