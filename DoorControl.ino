/* Example sketch to control a stepper motor with Arduino Motor Shield Rev3, Arduino UNO and AccelStepper.h library: number of steps or revolutions. More info: https://www.makerguides.com */

// Include the AccelStepper library:
#include <AccelStepper.h>

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Give the motor control pins names:
#define pwmA 3
#define pwmB 11
#define brakeA 9
#define brakeB 8
#define dirA 12
#define dirB 13

String doorStatus = "Closed";
bool alreadyOpen;

// Define the AccelStepper interface type:


String currentMode;
#define MotorInterfaceType 2

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(MotorInterfaceType, dirA, dirB);

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);


  //while (!Serial) {
  ;  // wait for serial port to connect. Needed for native USB port only
  //}

  // establishContact();  // send a byte to establish contact until receiver responds

  // Set the PWM and brake pins so that the direction pins can be used to control the motor:
  pinMode(pwmA, OUTPUT);
  pinMode(pwmB, OUTPUT);
  pinMode(brakeA, OUTPUT);
  pinMode(brakeB, OUTPUT);

  digitalWrite(pwmA, HIGH);
  digitalWrite(pwmB, HIGH);
  digitalWrite(brakeA, LOW);
  digitalWrite(brakeB, LOW);

  // Set the maximum steps per second:
  stepper.setMaxSpeed(600);
}

void runMotor(byte revolutions, byte spd, int dn) {
  // Set the current position to 0:
  stepper.setCurrentPosition(0);

  // Run the motor forward at 400 steps/second until the motor reaches 200 steps (1 revolution):
  while (stepper.currentPosition() != 200 * revolutions * dn) {
    stepper.setSpeed(spd * 100 * dn);
    stepper.runSpeed();
  }

  delay(1000);
}


void openDoor() {
  runMotor(4, 5, 1);
  doorStatus = "Open";
  alreadyOpen = true;
}
void closeDoor() {
  runMotor(4, 2, -1);
  doorStatus = "Closed";
  alreadyOpen = false;
}
 void establishContact() {
   while (Serial.available() <= 0) {
     Serial.print('A');  // send a capital A
     delay(300);
   }
 }

//if I'm in auto and sunrise happens (determined by the difference in system time and today's sunrise)
//open the door or close the door as necessary
//if auto and now-sunrise>0 then

void loop() {

  String data = Serial.readStringUntil('\n');

  if (data == "open" && alreadyOpen == false) {
    openDoor;
  } else if (data == "close" && alreadyOpen == true) {
    closeDoor;
  }
  Serial.println(doorStatus);
}
