

#include <Arduino.h>
#include <math.h>
#include "mpu9250.h"


#define MPU9250_IMU_ADDRESS 0x68
#define MPU9250_MAG_ADDRESS 0x0C

const float G = 9.80665f;


float mpu_getPitch(bfs::Mpu9250 &imu) {
  float accel_x = imu.accel_x_mps2() / G;
  float accel_z = -imu.accel_z_mps2() / G;
  float theta = atan2(accel_x, accel_z) / 2.0 / 3.141592654 * 360.0;
  return -theta;
}

float mpu_getRoll(bfs::Mpu9250 &imu) {
  float accel_y = imu.accel_y_mps2() / G;
  float accel_z = -imu.accel_z_mps2() / G;
  float phi = atan2(accel_y, accel_z) / 2.0 / 3.141592654 * 360.0;
  return phi;
}

float mpu_getYaw(bfs::Mpu9250 &imu) {
  return 0;
}



/* Mpu9250 object */
bfs::Mpu9250 imu;

void setup() {
  /* Serial to display data */
  Serial.begin(115200);
  while(!Serial) {}
  
  /* Start the I2C bus */
  Wire.begin();
  Wire.setClock(400000);
  
  /* I2C bus,  0x68 address */
  imu.Config(&Wire, MPU9250_IMU_ADDRESS);
  delay(1000);
  
  /* Initialize and configure IMU */
  if (!imu.Begin()) {
    Serial.println("Error initializing communication with IMU");
    while(1) {}
  }
  /* Set the sample rate divider */
  if (!imu.ConfigSrd(19)) {
    Serial.println("Error configured SRD");
    while(1) {}
  }
}


void loop() {
  /* Check if data read */
  if (imu.Read()) {
    /*
    Serial.print(imu.new_imu_data());
    Serial.print("\t");
    Serial.print(imu.new_mag_data());
    Serial.print("\t");
    */

    //Serial.print("Accel: (");
    Serial.print(imu.accel_x_mps2()/G);
    Serial.print(",");
    Serial.print(imu.accel_y_mps2()/G);
    Serial.print(",");
    Serial.print(-imu.accel_z_mps2()/G);
    Serial.print(",");
    Serial.print(mpu_getPitch(imu));
    Serial.print(",");
    Serial.println(mpu_getRoll(imu));

    
    //Serial.print(")\n");
    /*
    Serial.print(" Gyro: (");
    Serial.print(imu.gyro_x_radps());
    Serial.print(", ");
    Serial.print(imu.gyro_y_radps());
    Serial.print(", ");
    Serial.print(imu.gyro_z_radps());
    Serial.print(")");*/
    /*
    Serial.print(imu.mag_x_ut());
    Serial.print("\t");
    Serial.print(imu.mag_y_ut());
    Serial.print("\t");
    Serial.print(imu.mag_z_ut());
    Serial.print("\t");*/
    /*Serial.print(" Temp: ");
    Serial.print(imu.die_temp_c());
    Serial.print("°C)");*/
  }
}




/*
#include <Arduino.h>

#include "Wire.h"
#include "MPU9250.h"
*/





/*
int status;

// -----------------I2C-----------------

// ESP32-CAM
//#define I2C_SDA_PIN 14
//#define I2C_SCL_PIN 15
//TwoWire I2CMPU = TwoWire(0);

// ESP32-WROOM
MPU9250 imu;
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22


void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Started");
  
  // ESP32-CAM
  // I2CMPU.begin(I2C_SDA, I2C_SCL, 100000);
  // MPU9250 IMU(I2CMPU, 0x68);

  // ESP32-WROOM
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  imu.setWire(&Wire);

  //status = imu.begin();
  status = imu.beginAccel();
  status = imu.beginMag();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while (1){}
  } else {
    Serial.println("IMU initialization success");
  }
  
  //pinMode(13, OUTPUT);
}

void loop() {
  // read the sensor
  //MPU9250_readSensor(imu);
  

  // display the data
  //Serial.print(IMU.getAccelX_mss(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getAccelY_mss(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getAccelZ_mss(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getGyroX_rads(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getGyroY_rads(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getGyroZ_rads(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getMagX_uT(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getMagY_uT(), 6);
  //Serial.print("\t");
  //Serial.print(IMU.getMagZ_uT(), 6);
  //Serial.print("\t");
  //Serial.println(IMU.getTemperature_C(), 6);

  delay(100);
}
*/
