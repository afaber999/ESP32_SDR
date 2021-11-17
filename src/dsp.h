#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

void dsp_init();


void clear_peak();
float get_peak_i();
float get_peak_q();

enum AGC_MODES {
    AGC_OFF,
    AGC_SLOW,
    AGC_FAST
};

void dsp_demod_ssb(float* psamples);
void select_agc_mode(AGC_MODES mode);


#endif