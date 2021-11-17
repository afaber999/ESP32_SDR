
#ifndef I2S_INTERFACE_H_INCLUDED
#define I2S_INTERFACE_H_INCLUDED

#include "config.h"


void setup_i2s();
void i2s_write_buffer(float *cpx_buffer);
void i2s_read_buffer(float *cpx_buffer, float gain_i, float gain_q);

#endif




