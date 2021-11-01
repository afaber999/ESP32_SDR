
#ifndef I2S_INTERFACE_H_INCLUDED
#define I2S_INTERFACE_H_INCLUDED

#include "config.h"

void setup_i2s();

bool i2s_write_stereo_samples(float *fl_sample, float *fr_sample);
void i2s_read_stereo_samples(float *fl_sample, float *fr_sample);


#ifdef SAMPLE_BUFFER_SIZE
    bool i2s_write_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen);
    void i2s_read_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen);
#endif

#endif




