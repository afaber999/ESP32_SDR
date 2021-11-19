#ifndef DSP_FFT_H_INCLUDED
#define DSP_FFT_H_INCLUDED

enum AGC_MODES {
    AGC_OFF,
    AGC_SLOW,
    AGC_FAST
};

enum DEMOD_MODE {
    DEMOD_NONE,
    DEMOD_LSB,
    DEMOD_USB,
    DEMOD_CWL,
    DEMOD_CWU,
    DEMOD_AM,
    DEMOD_FM
};



void clear_peak();
float get_peak_i();
float get_peak_q();

bool dsp_fft_init();
void dsp_fft_demod(int16_t* psamples, DEMOD_MODE mode);
void select_agc_mode(AGC_MODES mode);


#endif