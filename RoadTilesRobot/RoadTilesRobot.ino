
#include <Arduino.h>

#include "Wire.h"
#include "I2C.h"
#include "MPU9250.h"

MPU9250 *imu;

void setup() {
  Serial.begin(115200);

  imu = new_MPU9250(0x68);
  MPU9250_begin(imu);

  pinMode(13, OUTPUT);
}

void loop() {

}
