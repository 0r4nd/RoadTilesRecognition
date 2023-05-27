

#include <Arduino.h>
#include <math.h>
#include "mpu9250.h"
#include "Filter1D.h"

#define MPU9250_IMU_ADDRESS 0x68
#define MPU9250_MAG_ADDRESS 0x0C

#define RAD2DEG (180.0 / 3.141592654)
#define G 9.80665f


float mpu_getYaw(bfs::Mpu9250 &imu, Filter1D *filter = NULL) {
  float yaw = 0;
  if (yaw < 0) yaw += 360.0;
  return yaw;
}

float mpu_getPitch(bfs::Mpu9250 &imu, Filter1D *filter = NULL) {
  float accel_x = imu.accel_x_mps2() / G;
  float accel_z = -imu.accel_z_mps2() / G;
  float theta = -atan2(accel_x, accel_z) * RAD2DEG;
  if (filter != NULL) {
    filter->addSample(theta);
    theta = filter->getValue();
  }
  return theta;
}


float mpu_getRoll(bfs::Mpu9250 &imu, Filter1D *filter = NULL) {
  float accel_y = imu.accel_y_mps2() / G;
  float accel_z = -imu.accel_z_mps2() / G;
  float phi = atan2(accel_y, accel_z) * RAD2DEG;
  if (filter != NULL) {
    filter->addSample(phi);
    phi = filter->getValue();
  }
  return phi;
}

float *mpu_getEuler(bfs::Mpu9250 &imu, float dt,
                    Filter1D *yaw = NULL,
                    Filter1D *pitch = NULL,
                    Filter1D *roll = NULL) {
  static float euler[3] = {0,0,0};
  dt /= 1000.0;
  euler[0] = mpu_getYaw(imu,yaw);
  euler[1] = (euler[1] - imu.gyro_y_radps()*RAD2DEG*dt)*0.95 + mpu_getPitch(imu,pitch)*0.05;
  euler[2] = (euler[2] - imu.gyro_x_radps()*RAD2DEG*dt)*0.95 + mpu_getRoll(imu,roll)*0.05;
  //euler[0] = mpu_getYaw(imu,yaw);
  //euler[1] = mpu_getPitch(imu,pitch);
  //euler[2] = mpu_getRoll(imu,roll);
  return euler;
}


/* Mpu9250 object */
bfs::Mpu9250 imu;
Filter1D pitch_filter(80);
Filter1D roll_filter(80);

// esp32-default
//#define I2C_SDA 40
//#define I2C_SCL 41

// esp32-cam
#define I2C_SDA 14
#define I2C_SCL 15
  
// esp32-lolin32 lite
//#define I2C_SDA 15
//#define I2C_SCL 2

#define I2C_FREQUENCY 100000 // 400000


float thetaG = 0.0;
float phiG = 0.0;
float theta;
float phi;
float dt = 0.0;
uint32_t millisOld;

void setup() {
  /* Serial to display data */
  Serial.begin(115200);
  while(!Serial) {}
  
  /* Start the I2C bus */
  // if ESP32
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQUENCY);
  // if arduino
  //Wire.begin();
  //Wire.setClock(I2C_FREQUENCY);
  
  /* I2C bus,  0x68 address */
  imu.Config(&Wire, bfs::Mpu9250::I2C_ADDR_PRIM);
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

  millisOld = millis();
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
    dt = millis() - millisOld;
    millisOld = millis();
    float *euler = mpu_getEuler(imu, dt, NULL, &pitch_filter, &roll_filter);

    //Serial.print("Accel: (");
    // scale
    Serial.print(-50);
    Serial.print(",");
    Serial.print(50);
    Serial.print(",");
/*
    Serial.print(imu.accel_x_mps2()/G);
    Serial.print(",");
    Serial.print(imu.accel_y_mps2()/G);
    Serial.print(",");
    Serial.print(-imu.accel_z_mps2()/G);
    Serial.print(",");*/
    /*
    Serial.print(mpu_getPitch(imu));
    Serial.print(",");
    Serial.print(mpu_getRoll(imu));
    Serial.print(",");
*/
    Serial.print(euler[1]);
    Serial.print(",");
    Serial.print(euler[2]);
    Serial.print(",");


/*
    thetaG = thetaG - imu.gyro_y_radps()*RAD2DEG*dt;
    phiG =  phiG - imu.gyro_x_radps()*RAD2DEG*dt;
    theta = (theta - imu.gyro_y_radps()*RAD2DEG*dt)*0.95 + euler[1]*0.05;
    phi = (phi - imu.gyro_x_radps()*RAD2DEG*dt)*0.95 + euler[2]*0.05;

    Serial.print(theta);
    Serial.print(",");
    Serial.print(phi);
    Serial.println(",");*/

    /*
    Serial.print(imu.gyro_x_radps());
    Serial.print(",");
    Serial.print(imu.gyro_y_radps());
    Serial.print(",");
    Serial.println(imu.gyro_z_radps());
    */

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

    pitch_filter.update();
    roll_filter.update();
  }
}

