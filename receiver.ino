#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// CE on 9, CSN on 10
RF24 radio(9, 10);
// This address MUST match the transmitter exactly
const byte address[6] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};

// Pin Definitions
int enA = 3; int in1 = 2; int in2 = 4; // Left Side (PWM on 3)
int enB = 5; int in3 = 7; int in4 = 8; // Right Side (PWM on 5)

struct ControlData {
  int forwardBackward;
  int leftRight;
};

ControlData data;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); 
  radio.setDataRate(RF24_1MBPS);    // Must match Transmitter
  radio.setChannel(76);            // Must match Transmitter
  radio.openReadingPipe(0, address);
  radio.startListening();
  Serial.println("Receiver Ready");
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    // Print values to Serial Monitor to verify radio link
    Serial.print("Data Rx -> FB: "); Serial.print(data.forwardBackward);
    Serial.print(" LR: "); Serial.println(data.leftRight);

    processMovement();
  } else {
    // If no signal, stop for safety
    // Serial.println("No signal..."); // Uncomment for heavy debugging
    // stopMotors(); 
  }
}

void processMovement() {
    int fb = data.forwardBackward;
    int lr = data.leftRight;
    int speed;

    if (fb > 160) {
      speed = map(fb, 160, 255, 0, 255);
      moveForward(speed);
    } 
    else if (fb < 90) {
      speed = map(fb, 90, 0, 0, 255);
      moveBackward(speed);
    }
    else if (lr > 160) {
      speed = map(lr, 160, 255, 0, 255);
      turnRight(speed);
    }
    else if (lr < 90) {
      speed = map(lr, 90, 0, 0, 255);
      turnLeft(speed);
    }
    else {
      stopMotors();
    }
}

void moveForward(int s) {
  analogWrite(enA, s); digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  analogWrite(enB, s); digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void moveBackward(int s) {
  analogWrite(enA, s); digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  analogWrite(enB, s); digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void turnRight(int s) {
  analogWrite(enA, s); digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  analogWrite(enB, s); digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void turnLeft(int s) {
  analogWrite(enA, s); digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  analogWrite(enB, s); digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void stopMotors() {
  analogWrite(enA, 0); digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  analogWrite(enB, 0); digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}