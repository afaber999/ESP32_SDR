#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

void dsp_init();
void dsp_pass_thru_sample(float* psample_i, float* psample_q, bool usb);
void dsp_demod_weaver_sample(float* psample_i, float* psample_q, bool usb);

void clear_peak();
float get_peak_i();
float get_peak_q();


#endif