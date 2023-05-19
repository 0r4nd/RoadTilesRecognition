   
#include "MPU9250.h"

// spi
#define SPI_READ     0x80
#define SPI_LS_CLOCK 1000000  // 1 MHz
#define SPI_HS_CLOCK 15000000 // 15 MHz

// constants
const float tempScale = 333.87f;
const float tempOffset = 21.0f;
const float G = 9.807f;
const float d2r = 3.14159265359f/180.0f;

/* transform the accel and gyro axes to match the magnetometer axes */
const int16_t tX[3] = {0,  1,  0}; 
const int16_t tY[3] = {1,  0,  0};
const int16_t tZ[3] = {0,  0, -1};

const float GYRO_FS_SEL_RANGE[4] = {
  250.0f/32767.5f,
  500.0f/32767.5f,
  1000.0f/32767.5f,
  2000.0f/32767.5f
};
const uint8_t GYRO_FS_SEL_DPS[4] = {
  MPU9250_GYRO_FS_SEL_250DPS,
  MPU9250_GYRO_FS_SEL_500DPS,
  MPU9250_GYRO_FS_SEL_1000DPS,
  MPU9250_GYRO_FS_SEL_2000DPS
};
const uint8_t ACCEL_DLPF[6] = {
  MPU9250_ACCEL_DLPF_184,
  MPU9250_ACCEL_DLPF_92,
  MPU9250_ACCEL_DLPF_41,
  MPU9250_ACCEL_DLPF_20,
  MPU9250_ACCEL_DLPF_10,
  MPU9250_ACCEL_DLPF_5
};
const uint8_t GYRO_DLPF[6] = {
  MPU9250_GYRO_DLPF_184,
  MPU9250_GYRO_DLPF_92,
  MPU9250_GYRO_DLPF_41,
  MPU9250_GYRO_DLPF_20,
  MPU9250_GYRO_DLPF_10,
  MPU9250_GYRO_DLPF_5
};

// utils
#define swap_bytes_u16(a) ((a << 8) & 0xff00) | (a >> 8) // a must be u16_t
#define vec3_dot(a,b) ((float)(a[0]*b[0] + a[1]*b[1] + a[2]*b[2]))



MPU9250 *new_MPU9250(uint8_t address) {
  MPU9250 *self = (MPU9250*)malloc(sizeof(*self));
  self->_address = address;
  self->_as[0] = 1.0f;
  self->_as[1] = 1.0f;
  self->_as[2] = 1.0f;
  self->_numSamples = 100;
  return self;
}
MPU9250 *delete_MPU9250(MPU9250 *self) {
  free(self);
  return (MPU9250*)NULL;
}



/******************************************************************************/

/**
 * 
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_readRegisters(MPU9250 *self, uint8_t subAddress, uint8_t count, uint8_t *dest) {
  return I2C_read(self->_address, subAddress, count, dest);
}


/******************************************************************************/

/**
 * 
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_writeRegister(MPU9250 *self, uint8_t subAddress, uint8_t data) {
  I2C_writeByte(self->_address, subAddress, data);
  delay(10);
  // verification
  MPU9250_readRegisters(self, subAddress, 1, self->_buffer);
  if (self->_buffer[0] == data) return 1;
  return -1;
}


/******************************************************************************/

/**
 * 
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_readAK8963Registers(MPU9250 *self, uint8_t subAddress, uint8_t count, uint8_t *dest) {
  // set slave 0 to the AK8963 and set for read
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_ADDR,
                            AK8963_I2C_ADDR | MPU9250_I2C_READ_FLAG) < 0) {
    return -1;
  }
  // set the register to the desired AK8963 sub address
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_REG, subAddress) < 0) {
    return -2;
  }
  // enable I2C and request the bytes
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_CTRL, MPU9250_I2C_SLV0_EN | count) < 0) {
    return -3;
  }
  delay(1); // takes some time for these registers to fill
  // read the bytes off the MPU9250 EXT_SENS_DATA registers
  self->_status = MPU9250_readRegisters(self, MPU9250_EXT_SENS_DATA_00, count, dest); 
  return self->_status;
}



/******************************************************************************/

/**
 * 
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_writeAK8963Register(MPU9250 *self, uint8_t subAddress, uint8_t data) {
  // set slave 0 to the AK8963 and set for write
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_ADDR, AK8963_I2C_ADDR) < 0) {
    return -1;
  }
  // set the register to the desired AK8963 sub address 
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_REG, subAddress) < 0) {
    return -2;
  }
  // store the data for write
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_DO, data) < 0) {
    return -3;
  }
  // enable I2C and send 1 byte
  if (MPU9250_writeRegister(self, MPU9250_I2C_SLV0_CTRL, MPU9250_I2C_SLV0_EN | 1) < 0) {
    return -4;
  }
  // read the register and confirm
  if (MPU9250_readAK8963Registers(self, subAddress, 1, self->_buffer) < 0) {
    return -5;
  }
  if(self->_buffer[0] == data) return 1;
  else return -6;
}


/******************************************************************************/

/**
 * gets the MPU9250 WHO_AM_I register value, expected to be 0x71
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_whoAmI(MPU9250 *self) {
  if (MPU9250_readRegisters(self, MPU9250_WHO_AM_I, 1, self->_buffer) < 0) {
    return -1;
  }
  return self->_buffer[0];
}


/******************************************************************************/

/**
 * gets the AK8963 WHO_AM_I register value, expected to be 0x48
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_whoAmIAK8963(MPU9250 *self) {
  if (MPU9250_readAK8963Registers(self, AK8963_WHO_AM_I, 1, self->_buffer) < 0) {
    return -1;
  }
  return self->_buffer[0];
}


/******************************************************************************/

/**
 * sets the gyro full scale range to values other than default
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_setGyroRange(MPU9250 *self, uint8_t range) {
  if (MPU9250_writeRegister(self, MPU9250_GYRO_CONFIG, GYRO_FS_SEL_DPS[range]) < 0) {
    return -1;
  }
  self->_gyroScale = GYRO_FS_SEL_RANGE[range] * d2r;
  return 1;
}

/* sets the DLPF bandwidth to values other than default */
int MPU9250_setDlpfBandwidth(MPU9250 *self, uint8_t bandwidth) {
  if (MPU9250_writeRegister(self, MPU9250_ACCEL_CONFIG2, ACCEL_DLPF[bandwidth]) < 0) {
    return -1;
  } 
  if (MPU9250_writeRegister(self, MPU9250_CONFIG, GYRO_DLPF[bandwidth]) < 0) {
    return -2;
  }
  self->_bandwidth = bandwidth;
  return 1;
}


/******************************************************************************/

/**
 * sets the sample rate divider to values other than default
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_setSrd(MPU9250 *self, uint8_t srd) {
  /* setting the sample rate divider to 19 to facilitate setting up magnetometer */
  if (MPU9250_writeRegister(self, MPU9250_SMPDIV,19) < 0) { // setting the sample rate divider
    return -1;
  }
  if (srd > 9) {
    // set AK8963 to Power Down
    if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_PWR_DOWN) < 0) {
      return -2;
    }
    delay(100); // long wait between AK8963 mode changes
    // set AK8963 to 16 bit resolution, 8 Hz update rate
    if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_CNT_MEAS1) < 0) {
      return -3;
    }
    delay(100); // long wait between AK8963 mode changes
    // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
    MPU9250_readAK8963Registers(self, AK8963_HXL, 7, self->_buffer);
  } else {
    // set AK8963 to Power Down
    if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_PWR_DOWN) < 0) {
      return -2;
    }
    delay(100); // long wait between AK8963 mode changes
    // set AK8963 to 16 bit resolution, 100 Hz update rate
    if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_CNT_MEAS2) < 0) {
      return -3;
    }
    delay(100); // long wait between AK8963 mode changes     
    // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
    MPU9250_readAK8963Registers(self, AK8963_HXL, 7, self->_buffer);
  }
  /* setting the sample rate divider */
  if (MPU9250_writeRegister(self, MPU9250_SMPDIV, srd) < 0) { // setting the sample rate divider
    return -4;
  }
  self->_srd = srd;
  return 1;
}


/******************************************************************************/

/**
 * estimates the gyro biases
 * 
 * @param self
 * @return -
 *
 */
int MPU9250_calibrateGyro(MPU9250 *self) {
  // set the range, bandwidth, and srd
  if (MPU9250_setGyroRange(self, GYRO_RANGE_250DPS) < 0) {
    return -1;
  }
  if (MPU9250_setDlpfBandwidth(self, DLPF_BANDWIDTH_20HZ) < 0) {
    return -2;
  }
  if (MPU9250_setSrd(self, 19) < 0) {
    return -3;
  }

  // take samples and find bias
  self->_gbD[0] = 0;
  self->_gbD[1] = 0;
  self->_gbD[2] = 0;
  for (size_t i = 0; i < self->_numSamples; i++) {
    MPU9250_readSensor(self);
    //self->_gbD[0] += (MPU9250_getGyroX_rads(self) + self->_gb[0])/((double)self->_numSamples);
    //self->_gbD[1] += (MPU9250_getGyroY_rads(self) + self->_gb[1])/((double)self->_numSamples);
    //self->_gbD[2] += (MPU9250_getGyroZ_rads(self) + self->_gb[2])/((double)self->_numSamples);
    delay(20);
  }
  self->_gb[0] = (float)self->_gbD[0];
  self->_gb[1] = (float)self->_gbD[1];
  self->_gb[2] = (float)self->_gbD[2];

  // set the range, bandwidth, and srd back to what they were
  if (MPU9250_setGyroRange(self, self->_gyroRange) < 0) {
    return -4;
  }
  if (MPU9250_setDlpfBandwidth(self, self->_bandwidth) < 0) {
    return -5;
  }
  if (MPU9250_setSrd(self, self->_srd) < 0) {
    return -6;
  }
  return 1;
}


/******************************************************************************/

/**
 *
 * @param self
 * @return -
 *
 */
int MPU9250_begin(MPU9250 *self) {
  Wire.begin();
  Wire.setClock(I2C_RATE);

  // select clock source to gyro
  if (MPU9250_writeRegister(self, MPU9250_PWR_MGMNT_1, MPU9250_CLOCK_SEL_PLL) < 0) {
    return -1;
  }
  // enable I2C master mode
  if (MPU9250_writeRegister(self, MPU9250_USER_CTRL, MPU9250_I2C_MST_EN) < 0) {
    return -2;
  }
  // set the I2C bus speed to 400 kHz
  if (MPU9250_writeRegister(self, MPU9250_I2C_MST_CTRL, MPU9250_I2C_MST_CLK) < 0) {
    return -3;
  }
  // set AK8963 to Power Down
  MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_PWR_DOWN);
  
  // reset the MPU9250
  MPU9250_writeRegister(self, MPU9250_PWR_MGMNT_1, MPU9250_PWR_RESET);
  
  // wait for MPU-9250 to come back up
  delay(1);
  
  // reset the AK8963
  MPU9250_writeAK8963Register(self, AK8963_CNTL2, AK8963_RESET);
  
  // select clock source to gyro
  if (MPU9250_writeRegister(self, MPU9250_PWR_MGMNT_1, MPU9250_CLOCK_SEL_PLL) < 0) {
    return -4;
  }
  // check the WHO AM I byte, expected value is 0x71 (decimal 113) or 0x73 (decimal 115)
  if ((MPU9250_whoAmI(self) != 113) && (MPU9250_whoAmI(self) != 115)) {
    return -5;
  }
  // enable accelerometer and gyro
  if (MPU9250_writeRegister(self, MPU9250_PWR_MGMNT_2, MPU9250_SEN_ENABLE) < 0) {
    return -6;
  }
  // setting accel range to 16G as default
  if (MPU9250_writeRegister(self, MPU9250_ACCEL_CONFIG, MPU9250_ACCEL_FS_SEL_16G) < 0) {
    return -7;
  }
  self->_accelScale = G * 16.0f/32767.5f; // setting the accel scale to 16G
  self->_accelRange = ACCEL_RANGE_16G;
  
  // setting the gyro range to 2000DPS as default
  if (MPU9250_writeRegister(self, MPU9250_GYRO_CONFIG, MPU9250_GYRO_FS_SEL_2000DPS) < 0) {
    return -8;
  }
  self->_gyroScale = (2000.0f/32767.5f) * d2r; // setting the gyro scale to 2000DPS
  self->_gyroRange = GYRO_RANGE_2000DPS;
  
  // setting bandwidth to 184Hz as default
  if (MPU9250_writeRegister(self, MPU9250_ACCEL_CONFIG2, MPU9250_ACCEL_DLPF_184) < 0) { 
    return -9;
  } 

  // setting gyro bandwidth to 184Hz
  if (MPU9250_writeRegister(self, MPU9250_CONFIG, MPU9250_GYRO_DLPF_184) < 0){
    return -10;
  }
  self->_bandwidth = DLPF_BANDWIDTH_184HZ;
  
  // setting the sample rate divider to 0 as default
  if (MPU9250_writeRegister(self, MPU9250_SMPDIV, 0x00) < 0) { 
    return -11;
  } 
  self->_srd = 0;
  
  // enable I2C master mode
  if (MPU9250_writeRegister(self, MPU9250_USER_CTRL, MPU9250_I2C_MST_EN) < 0) {
    return -12;
  }
  
  // set the I2C bus speed to 400 kHz
  if (MPU9250_writeRegister(self, MPU9250_I2C_MST_CTRL, MPU9250_I2C_MST_CLK) < 0) {
    return -13;
  }
  
  // check AK8963 WHO AM I register, expected value is 0x48 (decimal 72)
  if (MPU9250_whoAmIAK8963(self) != 72) {
    return -14;
  }
  
  /* get the magnetometer calibration */
  // set AK8963 to Power Down
  if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_PWR_DOWN) < 0) {
    return -15;
  }
  delay(100); // long wait between AK8963 mode changes
  
  // set AK8963 to FUSE ROM access
  if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_FUSE_ROM) < 0) {
    return -16;
  }
  delay(100); // long wait between AK8963 mode changes
  
  // read the AK8963 ASA registers and compute magnetometer scale factors
  MPU9250_readAK8963Registers(self, AK8963_ASA, 3, self->_buffer);
  self->_magScale[0] = ((((float)self->_buffer[0]) - 128.0f)/256.0f + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
  self->_magScale[1] = ((((float)self->_buffer[1]) - 128.0f)/256.0f + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
  self->_magScale[2] = ((((float)self->_buffer[2]) - 128.0f)/256.0f + 1.0f) * 4912.0f / 32760.0f; // micro Tesla 
  
  // set AK8963 to Power Down
  if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_PWR_DOWN) < 0) {
    return -17;
  }
  delay(100); // long wait between AK8963 mode changes
  
  // set AK8963 to 16 bit resolution, 100 Hz update rate
  if (MPU9250_writeAK8963Register(self, AK8963_CNTL1, AK8963_CNT_MEAS2) < 0) {
    return -18;
  }
  delay(100); // long wait between AK8963 mode changes
  
  // select clock source to gyro
  if (MPU9250_writeRegister(self, MPU9250_PWR_MGMNT_1, MPU9250_CLOCK_SEL_PLL) < 0) {
    return -19;
  }

  // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
  MPU9250_readAK8963Registers(self, AK8963_HXL,7, self->_buffer);
  
  // estimate gyro bias
  if (MPU9250_calibrateGyro(self) < 0) {
    return -20;
  }
  return 1;
}


int MPU9250_readSensor(MPU9250 *self) {  
  if (MPU9250_readRegisters(self, MPU9250_ACCEL_OUT, 21, self->_buffer) < 0) {
    return -1;
  }
  uint16_t *buf = (uint16_t*)self->_buffer;
  self->_acounts[0] = swap_bytes_u16(buf[0]);  
  self->_acounts[1] = swap_bytes_u16(buf[1]);
  self->_acounts[2] = swap_bytes_u16(buf[2]);
  self->_tcounts    = swap_bytes_u16(buf[3]);
  self->_gcounts[0] = swap_bytes_u16(buf[4]);
  self->_gcounts[1] = swap_bytes_u16(buf[5]);
  self->_gcounts[2] = swap_bytes_u16(buf[6]);
  self->_hcounts[0] = buf[7];
  self->_hcounts[1] = buf[8];
  self->_hcounts[2] = buf[9];
/*
  uint16_t *buf = self->_buffer;
  // combine into 16 bit values
  self->_axcounts = (buf[0] << 8) | buf[1];  
  self->_aycounts = (buf[2] << 8) | buf[3];
  self->_azcounts = (buf[4] << 8) | buf[5];
  self->_tcounts  = (buf[6] << 8) | buf[7];
  self->_gxcounts = (buf[8] << 8) | buf[9];
  self->_gycounts = (buf[10] << 8) | buf[11];
  self->_gzcounts = (buf[12] << 8) | buf[13];
  self->_hxcounts = (buf[15] << 8) | buf[14];
  self->_hycounts = (buf[17] << 8) | buf[16];
  self->_hzcounts = (buf[19] << 8) | buf[18];
  */
/*
  // transform and convert to float values
  self->_a[0] = (vec3_dot(tX, self->_acounts)*self->_accelScale - self->_ab[0]) * self->_as[0];
  self->_a[1] = (vec3_dot(tY, self->_acounts)*self->_accelScale - self->_ab[1]) * self->_as[1];
  self->_a[2] = (vec3_dot(tZ, self->_acounts)*self->_accelScale - self->_ab[2]) * self->_as[2];
  self->_g[0] = vec3_dot(tX, self->_gcounts)*self->_gyroScale - self->_gb[0];
  self->_g[1] = vec3_dot(tY, self->_gcounts)*self->_gyroScale - self->_gb[1];
  self->_g[2] = vec3_dot(tZ, self->_gcounts)*self->_gyroScale - self->_gb[2];
  self->_h[0] = ((float)self->_gcounts[0]*self->_magScale[0] - self->_hb[0]) * self->_hs[0];
  self->_h[1] = ((float)self->_gcounts[1]*self->_magScale[1] - self->_hb[1]) * self->_hs[0];
  self->_h[2] = ((float)self->_gcounts[2]*self->_magScale[2] - self->_hb[2]) * self->_hs[0];
  self->_t = ((((float)self->_tcounts) - tempOffset) / tempScale) + tempOffset;
  
  /*
  _ax = (((float)(tX[0]*self->_axcounts + tX[1]*self->_aycounts + tX[2]*_azcounts) * _accelScale) - _axb)*_axs;
  _ay = (((float)(tY[0]*self->_axcounts + tY[1]*self->_aycounts + tY[2]*_azcounts) * _accelScale) - _ayb)*_ays;
  _az = (((float)(tZ[0]*self->_axcounts + tZ[1]*self->_aycounts + tZ[2]*_azcounts) * _accelScale) - _azb)*_azs;
  _gx = ((float)(tX[0]*_gxcounts + tX[1]*_gycounts + tX[2]*_gzcounts) * _gyroScale) - _gxb;
  _gy = ((float)(tY[0]*_gxcounts + tY[1]*_gycounts + tY[2]*_gzcounts) * _gyroScale) - _gyb;
  _gz = ((float)(tZ[0]*_gxcounts + tZ[1]*_gycounts + tZ[2]*_gzcounts) * _gyroScale) - _gzb;
  _hx = (((float)(_hxcounts) * _magScaleX) - _hxb)*_hxs;
  _hy = (((float)(_hycounts) * _magScaleY) - _hyb)*_hys;
  _hz = (((float)(_hzcounts) * _magScaleZ) - _hzb)*_hzs;
  _t = ((((float) _tcounts) - _tempOffset)/_tempScale) + _tempOffset;
*/

  return 1;
}
