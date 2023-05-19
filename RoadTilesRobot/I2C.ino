

#include "I2C.h"


int I2C_read(uint8_t address, uint8_t Register, uint8_t count, uint8_t *data)
{
  Wire.beginTransmission(address);
  Wire.write(Register);
  Wire.endTransmission();
  if (Wire.requestFrom(address, count) == count) {
    while (Wire.available()) *data++ = Wire.read();
    return 1;
  }
  return -1;
}

void I2C_writeByte(uint8_t address, uint8_t Register, uint8_t data)
{
  Wire.beginTransmission(address);
  Wire.write(Register);
  Wire.write(data);
  Wire.endTransmission();
}
