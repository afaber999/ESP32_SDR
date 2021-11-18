#ifndef I2C_INTERFACE_H_INCLUDED
#define I2C_INTERFACE_H_INCLUDED

#include <cstdint>

void i2c_init(int sda, int scl, uint32_t frequency);
void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t val);
uint8_t i2c_read_reg(uint8_t addr, uint8_t reg);
void i2c_write_blocking(uint8_t addr, uint8_t *vals, uint8_t vcnt);

int i2c_scan();

#endif