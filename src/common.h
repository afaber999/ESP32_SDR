#ifndef COMMON_H_INCLUDED

void i2cWrite(uint8_t addr, uint8_t reg, uint8_t val);
void i2c_write_blocking(uint8_t addr, uint8_t *vals, uint8_t vcnt);

#endif
