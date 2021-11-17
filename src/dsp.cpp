#include <cstdint>
#include <stdio.h>
#include <math.h>

#include "dsp.h"
#include "filters.h"
#include "post_filter.h"
#include "kiss_fft.h"
#include "config.h"


static kiss_fft_cpx fft_proc[FFT_SAMPLES];
static kiss_fft_cpx fft_out[FFT_SAMPLES];

static volatile float peak_i;
static volatile float peak_q;

//static volatile float peakm_i;
//static volatile float peakm_q;

float get_peak_i() {
    return peak_i;
}

float get_peak_q() {
    return peak_q;
}

void clear_peak() {
    peak_i = 0.0f;
    peak_q = 0.0f;
}

static AGC_MODES agc_mode = AGC_OFF;

void select_agc_mode(AGC_MODES mode) {
    agc_mode = mode;
}


auto fft_cfg_fwd = kiss_fft_alloc(FFT_SAMPLES, 0, NULL, NULL);
auto fft_cfg_rev = kiss_fft_alloc(FFT_SAMPLES, 1, NULL, NULL);

void dsp_init() {
    post_filter_select(SSB_3000);
}

void dsp_demod_ssb(float* psamples) {
    kiss_fft(fft_cfg_fwd , (kiss_fft_cpx*)psamples , fft_proc);
    kiss_fft(fft_cfg_rev, fft_proc, fft_out);

    // filter
    // shift
    
    for ( auto i = 0; i < FFT_SAMPLES; i++ ) {
        auto audio_out = (fft_out[i].r + fft_out[i].i) * ( 0.5f /(float)FFT_SAMPLES );
        *psamples++ = audio_out;
        *psamples++ = audio_out;
        // *psamples++ = fft_out[i].r * 1/(float)FFT_SAMPLES;
        // *psamples++ = fft_out[i].i * 1/(float)FFT_SAMPLES;
    }
}
