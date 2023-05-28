

#include <Arduino.h>
#include <math.h>
#include <EEPROM.h>

#include "MPU9250.h"
#include "Filter1D.h"

#define MPU9250_IMU_ADDRESS 0x68
#define MPU9250_MAG_ADDRESS 0x0C

#define LED_BUILTIN     33
#define LED_BUILTIN_2    4 


#define RAD2DEG (180.0 / 3.141592654)
#define G 9.80665f

MPU9250 mpu;
// Filter1D pitch_filter(80);
// Filter1D roll_filter(80);


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


float dt = 0.0;


void delayBlink(uint32_t ms, uint32_t steps, int blink_2 = false) {
  float sub_ms = (float)ms / steps;
  int i = 0;
  for (float pos = 0; pos < ms; pos += sub_ms, i++) {
    delay((uint32_t)sub_ms);
    digitalWrite(LED_BUILTIN, i&1);
    if (blink_2) digitalWrite(LED_BUILTIN_2, i&1);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  if (blink_2) digitalWrite(LED_BUILTIN_2, LOW);
}


void mpu_calibrate_eeprom(MPU9250 &mpu) {
#if defined(ESP_PLATFORM) || defined(ESP8266)
  EEPROM.begin(0x80);
#endif
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_BUILTIN_2, OUTPUT);

  mpu.verbose(true);
  delayBlink(5000, 50);

  // calibrate anytime you want to
  Serial.println("");
  Serial.println("Accel Gyro calibration will start in 5sec.");
  Serial.println("Please leave the device still on the flat plane.");
  delayBlink(5000, 50);
  mpu.calibrateAccelGyro();

  if (mpu.get_use_AK8963()) {
    Serial.println("Mag calibration will start in 5sec.");
    Serial.println("Please Wave device in a figure eight until done.");
    delayBlink(5000, 50);
    mpu.calibrateMag();
  }

  // save to eeprom
  Serial.println("");
  mpu.saveCalibration();
  // load from eeprom
  mpu.loadCalibration();
  Serial.println("");
  mpu.print_calibration();
  Serial.println("");
  delayBlink(5000, 50);
  // final
  digitalWrite(LED_BUILTIN_2, HIGH);
  delay(1);
  digitalWrite(LED_BUILTIN_2, LOW);

  mpu.verbose(false);
}


void print_euler(MPU9250 &mpu) {
  float *euler = mpu.getEuler();
  Serial.print(euler[0], 2);
  Serial.print(", ");
  Serial.print(euler[1], 2);
  Serial.print(", ");
  Serial.print(euler[2], 2);
  Serial.println();
}












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
  //imu.Config(&Wire, bfs::Mpu9250::I2C_ADDR_PRIM);
  //delay(1000);
  
  /* Initialize and configure IMU */
  mpu.set_use_AK8963(false);
  if (!mpu.setup(0x68)) {
    while (1) {
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
      delay(5000);
    }
  }

  // configuration
  mpu.selectFilter(QuatFilterSel::MAHONY); //NONE, MADGWICK, MAHONY,
  mpu.setFilterKp(0.5);

  // calibration
  mpu_calibrate_eeprom(mpu);
}


void loop() {
  if (mpu.update()) {
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 25) {
      print_euler(mpu);
      prev_ms = millis();
    }
  }

}

