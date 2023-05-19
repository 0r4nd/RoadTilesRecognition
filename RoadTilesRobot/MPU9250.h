


#ifndef _MPU9250_H
#define _MPU9250_H

#include "Arduino.h"
#include "I2C.h"

#include "Wire.h"

/*--------------------------------------------------------------------------
| Pin Name  | Function                                                     | 
----------------------------------------------------------------------------
| INT       | Interrupt pin
| ECL       | I2C Master Serial Clock for external sensors connection pin
| AD0/SDO   | I2C Address/Serial data out pin
| FSYNC     | Frame Synchronization input pin
| VCC       | Power supply pin
| GND       | Ground pin
| EDA       | I2C Serial Data input for external sensors connection pin
| nCS       | Chip selection pin
| SCL/SCLK  | I2C Serial Clock/SPI Serial Clock pin
| SDA/SDI   | I2C Serial Data/SPI Serial Data pin
--------------------------------------------------------------------------*/

// MPU9250 registers
#define MPU9250_ACCEL_OUT           0x3B
#define MPU9250_GYRO_OUT            0x43
#define MPU9250_TEMP_OUT            0x41
#define MPU9250_EXT_SENS_DATA_00    0x49
#define MPU9250_ACCEL_CONFIG        0x1C
#define MPU9250_ACCEL_FS_SEL_2G     0x00
#define MPU9250_ACCEL_FS_SEL_4G     0x08
#define MPU9250_ACCEL_FS_SEL_8G     0x10
#define MPU9250_ACCEL_FS_SEL_16G    0x18
#define MPU9250_GYRO_CONFIG         0x1B
#define MPU9250_GYRO_FS_SEL_250DPS  0x00
#define MPU9250_GYRO_FS_SEL_500DPS  0x08
#define MPU9250_GYRO_FS_SEL_1000DPS 0x10
#define MPU9250_GYRO_FS_SEL_2000DPS 0x18
#define MPU9250_ACCEL_CONFIG2       0x1D
#define MPU9250_ACCEL_DLPF_184      0x01
#define MPU9250_ACCEL_DLPF_92       0x02
#define MPU9250_ACCEL_DLPF_41       0x03
#define MPU9250_ACCEL_DLPF_20       0x04
#define MPU9250_ACCEL_DLPF_10       0x05
#define MPU9250_ACCEL_DLPF_5        0x06
#define MPU9250_CONFIG              0x1A
#define MPU9250_GYRO_DLPF_184       0x01
#define MPU9250_GYRO_DLPF_92        0x02
#define MPU9250_GYRO_DLPF_41        0x03
#define MPU9250_GYRO_DLPF_20        0x04
#define MPU9250_GYRO_DLPF_10        0x05
#define MPU9250_GYRO_DLPF_5         0x06
#define MPU9250_SMPDIV              0x19
#define MPU9250_INT_PIN_CFG         0x37
#define MPU9250_INT_ENABLE          0x38
#define MPU9250_INT_DISABLE         0x00
#define MPU9250_INT_PULSE_50US      0x00
#define MPU9250_INT_WOM_EN          0x40
#define MPU9250_INT_RAW_RDY_EN      0x01
#define MPU9250_PWR_MGMNT_1         0x6B
#define MPU9250_PWR_CYCLE           0x20
#define MPU9250_PWR_RESET           0x80
#define MPU9250_CLOCK_SEL_PLL       0x01
#define MPU9250_PWR_MGMNT_2         0x6C
#define MPU9250_SEN_ENABLE          0x00
#define MPU9250_DIS_GYRO            0x07
#define MPU9250_USER_CTRL           0x6A
#define MPU9250_I2C_MST_EN          0x20
#define MPU9250_I2C_MST_CLK         0x0D
#define MPU9250_I2C_MST_CTRL        0x24
#define MPU9250_I2C_SLV0_ADDR       0x25
#define MPU9250_I2C_SLV0_REG        0x26
#define MPU9250_I2C_SLV0_DO         0x63
#define MPU9250_I2C_SLV0_CTRL       0x27
#define MPU9250_I2C_SLV0_EN         0x80
#define MPU9250_I2C_READ_FLAG       0x80
#define MPU9250_MOT_DETECT_CTRL     0x69
#define MPU9250_ACCEL_INTEL_EN      0x80
#define MPU9250_ACCEL_INTEL_MODE    0x40
#define MPU9250_LP_ACCEL_ODR        0x1E
#define MPU9250_WOM_THR             0x1F
#define MPU9250_WHO_AM_I            0x75
#define MPU9250_FIFO_EN             0x23
#define MPU9250_FIFO_TEMP           0x80
#define MPU9250_FIFO_GYRO           0x70
#define MPU9250_FIFO_ACCEL          0x08
#define MPU9250_FIFO_MAG            0x01
#define MPU9250_FIFO_COUNT          0x72
#define MPU9250_FIFO_READ           0x74

// AK8963 registers
#define AK8963_I2C_ADDR   0x0C
#define AK8963_HXL        0x03 
#define AK8963_CNTL1      0x0A
#define AK8963_PWR_DOWN   0x00
#define AK8963_CNT_MEAS1  0x12
#define AK8963_CNT_MEAS2  0x16
#define AK8963_FUSE_ROM   0x0F
#define AK8963_CNTL2      0x0B
#define AK8963_RESET      0x01
#define AK8963_ASA        0x10
#define AK8963_WHO_AM_I   0x00




#define GYRO_RANGE_250DPS      0
#define GYRO_RANGE_500DPS      1
#define GYRO_RANGE_1000DPS     2
#define GYRO_RANGE_2000DPS     3

#define ACCEL_RANGE_2G         0
#define ACCEL_RANGE_4G         1
#define ACCEL_RANGE_8G         2
#define ACCEL_RANGE_16G        3

#define DLPF_BANDWIDTH_184HZ   0
#define DLPF_BANDWIDTH_92HZ    1
#define DLPF_BANDWIDTH_41HZ    2
#define DLPF_BANDWIDTH_20HZ    3
#define DLPF_BANDWIDTH_10HZ    4
#define DLPF_BANDWIDTH_5HZ     5

#define LP_ACCEL_ODR_0_24HZ    0
#define LP_ACCEL_ODR_0_49HZ    1
#define LP_ACCEL_ODR_0_98HZ    2
#define LP_ACCEL_ODR_1_95HZ    3
#define LP_ACCEL_ODR_3_91HZ    4
#define LP_ACCEL_ODR_7_81HZ    5
#define LP_ACCEL_ODR_15_63HZ   6
#define LP_ACCEL_ODR_31_25HZ   7
#define LP_ACCEL_ODR_62_50HZ   8
#define LP_ACCEL_ODR_125HZ     9
#define LP_ACCEL_ODR_250HZ    10
#define LP_ACCEL_ODR_500HZ    11


typedef struct {
  int _status;
    
  // reading buffer
  uint8_t _buffer[21], _address;

  // data counts
  int16_t _acounts[3],_gcounts[3],_hcounts[3], _tcounts;

  // data buffer
  float _a[3],_g[3],_h[3], _t;

  // scale factors
  float _accelScale,_gyroScale,_magScale[3];
  
  // configuration
  uint8_t _accelRange, _gyroRange, _bandwidth;
  uint8_t _srd;
  
  // gyro bias estimation
  size_t _numSamples;
  double _gbD[3];
  float _gb[3];
    
  // accel bias and scale factor estimation
  float _ab[3],_as[3];
} MPU9250;


// prototypes
MPU9250 *new_MPU9250(uint8_t address);
MPU9250 *delete_MPU9250(MPU9250 *self);

int MPU9250_readRegisters(MPU9250 *self, uint8_t subAddress, uint8_t count, uint8_t *dest);
int MPU9250_writeRegister(MPU9250 *self, uint8_t subAddress, uint8_t data);
int MPU9250_readAK8963Registers(MPU9250 *self, uint8_t subAddress, uint8_t count, uint8_t *dest);
int MPU9250_writeAK8963Register(MPU9250 *self, uint8_t subAddress, uint8_t data);

int MPU9250_whoAmI(MPU9250 *self);
int MPU9250_whoAmIAK8963(MPU9250 *self);

int MPU9250_setGyroRange(MPU9250 *self, uint8_t range);
int MPU9250_setDlpfBandwidth(MPU9250 *self, uint8_t bandwidth);
int MPU9250_setSrd(MPU9250 *self, uint8_t srd);

int MPU9250_calibrateGyro(MPU9250 *self);
int MPU9250_begin(MPU9250 *self);

int MPU9250_readSensor(MPU9250 *self);

#endif // _MPU9250_H
