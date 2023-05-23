
#include <Wire.h>


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

 
void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  Serial.println("\nI2C Scanner");

  // if ESP32
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQUENCY);
  // if arduino
  //Wire.begin();
}


void I2Cscanner() {
  int nDevices = 0;
  Serial.println("Scanning...");
  
  for (int address = 1; address < 127; address++) {
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
      if (address<16) {
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
}


void loop() {
  delay(5000);
  I2Cscanner();
}
