#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

RF24 radio(9, 10);
const byte address[6] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};

const int MPU_ADDR = 0x68;

struct ControlData {
  int forwardBackward; 
  int leftRight;       
};

ControlData data;
float pitch, roll;
float pitchOffset = 0, rollOffset = 0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); 
  radio.setDataRate(RF24_1MBPS);    // Must match Receiver
  radio.setChannel(76);            // Use a specific channel (0-125)
  radio.openWritingPipe(address);
  radio.stopListening();
  Serial.println("Transmitter Ready");
}

void loop() {
  readMPU();
  
  data.forwardBackward = constrain(map(pitch, -30, 30, 0, 255), 0, 255);
  data.leftRight = constrain(map(roll, -30, 30, 0, 255), 0, 255);

  bool success = radio.write(&data, sizeof(data));
  
  Serial.print("Pitch: "); Serial.print(pitch, 0);
  Serial.print(" | Roll: "); Serial.print(roll, 0);
  Serial.print(" | FB: "); Serial.print(data.forwardBackward);
  Serial.print(" LR: "); Serial.print(data.leftRight);

  if (success) {
    Serial.println(" >> [SENT OK]");
  } else {
    Serial.println(" >> [SEND FAILED - Receiver not found]");
  }
  
  delay(100);
}

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  if (Wire.available() >= 6) {
    int16_t rawX = Wire.read() << 8 | Wire.read();
    int16_t rawY = Wire.read() << 8 | Wire.read();
    int16_t rawZ = Wire.read() << 8 | Wire.read();

    float ax = rawX / 16384.0;
    float ay = rawY / 16384.0;
    float az = rawZ / 16384.0;

    pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI - pitchOffset;
    roll = atan2(-ax, az) * 180.0 / PI - rollOffset;
  }
}

void calibrateMPU() {
  float pSum = 0, rSum = 0;
  for (int i = 0; i < 50; i++) {
    readMPU();
    pSum += pitch;
    rSum += roll;
    delay(10);
  }
  pitchOffset = pSum / 50;
  rollOffset = rSum / 50;
}
