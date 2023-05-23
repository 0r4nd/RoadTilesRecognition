
#include "Arduino.h"
#include "Wire.h"


#define MPU9250_IMU_ADDRESS 0x68
#define MPU9250_MAG_ADDRESS 0x0C

#define GYRO_FULL_SCALE_250_DPS  0x00
#define GYRO_FULL_SCALE_500_DPS  0x08
#define GYRO_FULL_SCALE_1000_DPS 0x10
#define GYRO_FULL_SCALE_2000_DPS 0x18

#define ACC_FULL_SCALE_2G  0x00
#define ACC_FULL_SCALE_4G  0x08
#define ACC_FULL_SCALE_8G  0x10
#define ACC_FULL_SCALE_16G 0x18

#define TEMPERATURE_OFFSET 21 // As defined in documentation

#define INTERVAL_MS_PRINT 1000/2



#define G 9.80665

typedef struct {
  int16_t x, y, z;
} Gyroscope;

typedef struct {
  int16_t x, y, z;
} Accelerometer;

typedef struct {
  int16_t x, y, z;
  struct {
    int8_t x, y, z;
  } adjustment;
} Magnetometer;

typedef struct {
  int16_t value;
} Temperature;

typedef struct {
  struct {
    float x, y, z;
  } accelerometer, gyroscope, magnetometer;
  float temperature;
} Normalized;



/////////////////////////////////////////////////////////////////////// I2C.cpp
void I2Cread(uint8_t address, uint8_t reg, uint8_t bytes, uint8_t* data) {
  Wire.beginTransmission(address); // Set register address
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(address, bytes); //Read bytes amount
  while (Wire.available()) *data++ = Wire.read();
}
void I2CwriteByte(uint8_t address, uint8_t reg, uint8_t data) {
  Wire.beginTransmission(address); //Set register address
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

/////////////////////////////////////////////////////////////////////// normalizer.cpp
void normalize(Gyroscope *gyro, Normalized *norm) {
  // Sensitivity Scale Factor (MPU datasheet page 8)
  norm->gyroscope.x = gyro->x / 32.8;
  norm->gyroscope.y = gyro->y / 32.8;
  norm->gyroscope.z = gyro->z / 32.8;
}
void normalize(Accelerometer *acc, Normalized *norm) {
  // Sensitivity Scale Factor (MPU datasheet page 9)
  norm->accelerometer.x = acc->x * G / 16384;
  norm->accelerometer.y = acc->y * G / 16384;
  norm->accelerometer.z = acc->z * G / 16384;
}
void normalize(Temperature *temp, Normalized *norm) {
  // Sensitivity Scale Factor (MPU datasheet page 11) & formula (MPU registers page 33)
  norm->temperature = ((temp->value - TEMPERATURE_OFFSET) / 333.87) + TEMPERATURE_OFFSET;
}
void normalize(Magnetometer *magneto, Normalized *norm){
  // Sensitivity Scale Factor (MPU datasheet page 10)
  // 0.6 µT/LSB (14-bit)
  // 0.15µT/LSB (16-bit)
  // Adjustment values (MPU register page 53)
  norm->magnetometer.x = magneto->x * 0.15 * (((magneto->adjustment.x - 128) / 256) + 1);
  norm->magnetometer.y = magneto->y * 0.15 * (((magneto->adjustment.y - 128) / 256) + 1);
  norm->magnetometer.z = magneto->z * 0.15 * (((magneto->adjustment.z - 128) / 256) + 1);
}


/////////////////////////////////////////////////////////////////////// magnetometer.cpp
void setMagnetometerAdjustmentValues(Magnetometer *magneto) {
  uint8_t buff[3];
  I2CwriteByte(MPU9250_MAG_ADDRESS, 0x0A, 0x1F); // Set 16-bits output & fuse ROM access mode
  delay(1000);
  I2Cread(MPU9250_MAG_ADDRESS, 0x10, 3, buff); // Read adjustments
  magneto->adjustment.x = buff[0]; //Adjustment for X axis
  magneto->adjustment.y = buff[1]; //Adjustment for Y axis
  magneto->adjustment.z = buff[2]; //Adjustment for Z axis
  I2CwriteByte(MPU9250_MAG_ADDRESS, 0x0A, 0x10); // Power down
}

bool isMagnetometerReady() {
  uint8_t isReady; // Interruption flag
  I2Cread(MPU9250_MAG_ADDRESS, 0x02, 1, &isReady);
  return isReady & 0x01; // Read register and wait for the DRDY
}

void readRawMagnetometer(Magnetometer *magneto) {
  uint8_t buff[7];
  // Read output registers:
  // [0x03-0x04] X-axis measurement
  // [0x05-0x06] Y-axis measurement
  // [0x07-0x08] Z-axis measurement
  I2Cread(MPU9250_MAG_ADDRESS, 0x03, 7, buff);
  // Magnetometer, create 16 bits values from 8 bits data
  magneto->x = (buff[1] << 8 | buff[0]);
  magneto->y = (buff[3] << 8 | buff[2]);
  magneto->z = (buff[5] << 8 | buff[4]);
}

/////////////////////////////////////////////////////////////////////// imu.cpp
bool isImuReady() {
  uint8_t isReady; // Interruption flag
  I2Cread(MPU9250_IMU_ADDRESS, 58, 1, &isReady);
  return isReady & 0x01; // Read register and wait for the RAW_DATA_RDY_INT
}


void whoAmI() {
  uint8_t who_am_i = 0;
  I2Cread(MPU9250_IMU_ADDRESS, 0x75, 1, &who_am_i);

  Serial.print("WhoAmI Register: 0x");
  Serial.println(who_am_i, HEX);
  switch(who_am_i) {
    case 0x70:
      Serial.println("Your device is an MPU6500.");
      Serial.println("The MPU6500 does not have a magnetometer."); 
      break;
    case 0x71:
      Serial.println("Your device is an MPU9250");
      break;
    case 0x73:
      Serial.println("Your device is an MPU9255");
      Serial.println("Not sure if it works with this library, just try");
      break;
    case 0x75:
      Serial.println("Your device is probably an MPU6515"); 
      Serial.println("Not sure if it works with this library, just try");
      break;
    case 0x00:
      Serial.println("Can't connect to your device. Check all connections.");
      break;
    default:
      Serial.println("Unknown device - it may work with this library or not, just try");
  }
  Serial.println();
}



void readRawImu(Accelerometer *acc, Temperature *temp, Gyroscope *gyro) {
  uint8_t buff[14];
  // Read output registers:
  // [59-64] Accelerometer
  // [65-66] Temperature
  // [67-72] Gyroscope
  I2Cread(MPU9250_IMU_ADDRESS, 59, 14, buff);
  // Accelerometer, create 16 bits values from 8 bits data
  acc->x = (buff[0] << 8 | buff[1]);
  acc->y = (buff[2] << 8 | buff[3]);
  acc->z = (buff[4] << 8 | buff[5]);
  // Temperature, create 16 bits values from 8 bits data
  temp->value = (buff[6] << 8 | buff[7]);
  // Gyroscope, create 16 bits values from 8 bits data
  gyro->x = (buff[8] << 8 | buff[9]);
  gyro->y = (buff[10] << 8 | buff[11]);
  gyro->z = (buff[12] << 8 | buff[13]);
}





unsigned long lastPrintMillis = 0;
Gyroscope GYROSCOPE;
Accelerometer ACCELEROMETER;
Magnetometer MAGNETOMETER;
Temperature TEMPERATURE;
Normalized NORMALIZED;





 
void I2C_scanner() {
  int nDevices = 0;
  Serial.println("Scanning...");
  
  for (int address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
  Serial.println();
}


// esp32-default
//#define I2C_SDA 40
//#define I2C_SCL 41

// esp32-cam
#define I2C_SDA 14
#define I2C_SCL 15
  
// esp32-lolin32 lite
//#define I2C_SDA 15
//#define I2C_SCL 2

#define I2C_FREQUENCY 100000



void setup()
{
  Serial.begin(115200);
  while(!Serial) {}
  
  // if ESP32
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQUENCY);
  // if arduino
  //Wire.begin();

  delay(2000);
  
  I2C_scanner();
  whoAmI();

  I2CwriteByte(MPU9250_IMU_ADDRESS, 27, GYRO_FULL_SCALE_1000_DPS); // Configure gyroscope range
  I2CwriteByte(MPU9250_IMU_ADDRESS, 28, ACC_FULL_SCALE_2G);        // Configure accelerometer range

  I2CwriteByte(MPU9250_IMU_ADDRESS, 55, 0x02); // Set by pass mode for the magnetometers
  I2CwriteByte(MPU9250_IMU_ADDRESS, 56, 0x01); // Enable interrupt pin for raw data

  setMagnetometerAdjustmentValues(&MAGNETOMETER);

  //Start magnetometer
  I2CwriteByte(MPU9250_MAG_ADDRESS, 0x0A, 0x12); // Request continuous magnetometer measurements in 16 bits (mode 1)
}



void loop()
{
  unsigned long currentMillis = millis();
  
  if (isImuReady()) {
    readRawImu(&ACCELEROMETER, &TEMPERATURE, &GYROSCOPE);
    normalize(&GYROSCOPE, &NORMALIZED);
    normalize(&ACCELEROMETER, &NORMALIZED);
    normalize(&TEMPERATURE, &NORMALIZED);
  }

  if (isMagnetometerReady()) {
    readRawMagnetometer(&MAGNETOMETER);
    normalize(&MAGNETOMETER, &NORMALIZED);
  }

  if (currentMillis - lastPrintMillis > INTERVAL_MS_PRINT) {
    Serial.print("Temperature: ");
    Serial.print(NORMALIZED.temperature,2);
    Serial.print("\xC2\xB0"); //Print degree symbol
    Serial.print("C");
    Serial.println();

    Serial.print("Gyroscope(");
    Serial.print("\xC2\xB0"); //Print degree symbol
    Serial.print("/s):       (");
    Serial.print(NORMALIZED.gyroscope.x,2);
    Serial.print(", ");
    Serial.print(NORMALIZED.gyroscope.y,2);
    Serial.print(", ");
    Serial.print(NORMALIZED.gyroscope.z,2);
    Serial.print(")");
    Serial.println();
    
    Serial.print("Accelerometer(m/s^2): (");
    Serial.print(NORMALIZED.accelerometer.x, 2);
    Serial.print(", ");
    Serial.print(NORMALIZED.accelerometer.y, 2);
    Serial.print(", ");
    Serial.print(NORMALIZED.accelerometer.z, 2);
    Serial.print(")");
    Serial.println();

    Serial.print("Magnetometer(");
    Serial.print("\xce\xbc"); //Print micro symbol
    Serial.print("T):     (");
    Serial.print(NORMALIZED.magnetometer.x, 2);
    Serial.print(", ");
    Serial.print(NORMALIZED.magnetometer.y, 2);
    Serial.print(", ");
    Serial.print(NORMALIZED.magnetometer.z, 2);
    Serial.print(")");
    Serial.println();
    
    Serial.println();

    lastPrintMillis = currentMillis;
  }
}
