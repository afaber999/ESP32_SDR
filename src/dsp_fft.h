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

struct DSP_FFT_FILT_SETTINGS {
    float low;
    float high;
    uint32_t updated_at;
};

extern volatile DSP_FFT_FILT_SETTINGS dsp_fft_filt;

void clear_peak();
float get_peak_i();
float get_peak_q();

bool dsp_fft_init();
void dsp_fft_demod(int16_t* psamples, DEMOD_MODE mode, int32_t shift_bins);
void select_agc_mode(AGC_MODES mode);


#endif