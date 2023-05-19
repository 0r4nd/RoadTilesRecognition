

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>
#include "Wire.h"    // I2C library


#define I2C_RATE  400000 // 400 kHz


int I2C_read(uint8_t address, uint8_t Register, uint8_t count, uint8_t *data);
void I2C_writeByte(uint8_t address, uint8_t Register, uint8_t data);


#endif // _I2C_H
