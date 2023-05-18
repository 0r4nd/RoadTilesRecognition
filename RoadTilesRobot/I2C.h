

#ifndef _I2C_H
#define _I2C_H


void I2C_read(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t *Data);
void I2C_writeByte(uint8_t Address, uint8_t Register, uint8_t Data);


#endif // _I2C_H
