

#include <stdint.h>
#include "Wire.h"


void I2C_read(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t *Data)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
  Wire.requestFrom(Address, Nbytes);
  while (Wire.available()) {
    *Data++ = Wire.read();
  }
}

void I2C_writeByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}
