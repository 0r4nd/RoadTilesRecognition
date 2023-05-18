
#include <Arduino.h>

#include "Wire.h"
#include "I2C.h"
#include "MPU9250.h"



uint8_t Buf[14];
float yangle;



void setup() {
  Wire.begin();
  I2C_writeByte(MPU9250_ADDRESS, 29, 0x06);
  I2C_writeByte(MPU9250_ADDRESS, 28, MPU9250_ACC_FULL_SCALE_2_G);
  I2C_writeByte(MPU9250_ADDRESS, 108, 0x2F);
}

void loop() {
  I2C_read(MPU9250_ADDRESS, 0x3B, 14, Buf);
  int16_t ay = -(Buf[2] << 8 | Buf[3]);
  yangle = ay / 182.04;
  delay(100);
}
