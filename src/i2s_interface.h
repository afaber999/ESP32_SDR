
#ifndef I2S_INTERFACE_H_INCLUDED
#define I2S_INTERFACE_H_INCLUDED

#include "config.h"


void setup_i2s();
bool i2s_write_buffer(int16_t* data);
bool i2s_read_buffer(int16_t* data);
#endif




