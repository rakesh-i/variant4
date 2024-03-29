#include <Arduino.h>

class Motor
{
public:
    Motor(int speedPin, int directionPin);

    // Function to control the motor (speed: 0 to 255, direction: 0 for reverse, 1 for forward)
    void control(int speed, int direction);

private:
    int speedPin;
    int directionPin;
};

Motor::Motor(int speedPin, int directionPin)
{
    this->speedPin = speedPin;
    this->directionPin = directionPin;

    pinMode(speedPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
}

void Motor::control(int speed, int direction)
{
    // Ensure the speed is within the valid range (0 to 255)
    speed = constrain(speed, 0, 255);

    // Ensure the direction is either 0 or 1
    direction = constrain(direction, 0, 1);

    // Set the direction
    digitalWrite(directionPin, direction);

    // Set the speed using PWM
    analogWrite(speedPin, speed);
}
