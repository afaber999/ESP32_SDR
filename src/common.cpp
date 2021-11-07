#include <cstdint>
#include <Arduino.h>
#include <Wire.h>
#include "common.h"


void i2cWrite(uint8_t addr, uint8_t reg, uint8_t val) {   // write reg via i2c
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void i2c_write_blocking(uint8_t addr, uint8_t *vals, uint8_t vcnt) {
  Wire.beginTransmission(addr);
  while (vcnt--) Wire.write(*vals++);
  Wire.endTransmission();
}



