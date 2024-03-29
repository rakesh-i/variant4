#include <AccelStepper.h>

// Define the connections for your stepper motor and motor driver
#define STEP_PIN 3
#define DIR_PIN 4

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

#define TENSION_SENSOR_PIN A6

const int minTension = 450; // Minimum tension (adjust as needed)
const int maxTension = 500; // Maximum tension (adjust as needed)

void setup() {
  stepper.setMaxSpeed(2000); // Adjust the maximum speed as needed
  stepper.setAcceleration(1000); // Adjust the acceleration as needed
  
  // Set up serial communication
  Serial.begin(9600);
}

void loop() {
  // Call the function to control the stepper motor based on tension
  controlStepperBasedOnTension();
}

void controlStepperBasedOnTension() {
  int tension = analogRead(TENSION_SENSOR_PIN);
  if (tension >= minTension && tension <= maxTension) {
  } else if (tension < minTension) {
    stepper.setSpeed(500); 
    stepper.runSpeed(); 
  } else if (tension > maxTension) {
    stepper.setSpeed(-500); 
    stepper.runSpeed(); 
  }
}
