#include <cstdint>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include "dsp_fft.h"
#include "filters.h"
#include "post_filter.h"
#include "kiss_fft.h"
#include "config.h"



static kiss_fft_cpx fft_in[FFT_SAMPLES];
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

bool dsp_fft_init() {
    post_filter_select(SSB_3000);
    return ( 2 * DMA_SAMPLES == FFT_SAMPLES);
}

void dsp_fft_demod(int16_t* psamples, DEMOD_MODE mode) {

    // overlap and add
    int idx = FFT_SAMPLES / 2;
    auto new_samples = psamples;

    for (int n = 0; n < FFT_SAMPLES/2; n++)
    {
        fft_in[n] = fft_in[idx];
        fft_in[idx].r = (float)*new_samples++ * (1.0f / 32767.0f);
        fft_in[idx].i = (float)*new_samples++ * (1.0f / 32767.0f);

        peak_i = fmaxf( peak_i, fft_in[idx].r);
        peak_q = fmaxf( peak_q, fft_in[idx].i);
        idx++;
    }

    kiss_fft(fft_cfg_fwd , fft_in , fft_proc);

    // apply shift

    // mask out

    // apply filter


    kiss_fft(fft_cfg_rev, fft_proc, fft_out);

    switch (mode) {
        case DEMOD_AM:
        case DEMOD_CWL:
        case DEMOD_CWU:
        case DEMOD_FM:
        case DEMOD_LSB:
        case DEMOD_USB:
            // Overlap and add, take first half of FFT output buffer
            for ( auto i = 0; i < FFT_SAMPLES / 2; i++ ) {
                auto audio_out = (int16_t)(fft_out[i].r + fft_out[i].i) * ( 16384.0f * 0.5f /(float)FFT_SAMPLES );
                *psamples++ = audio_out;
                *psamples++ = audio_out;
            }
        break;
        
        default:
        case DEMOD_NONE:
            // Overlap and add, take first half of FFT output buffer
            for ( auto i = 0; i < FFT_SAMPLES / 2; i++ ) {
                *psamples++ = (int16_t)(fft_out[i].r) * ( 32767.0f /(float)FFT_SAMPLES );
                *psamples++ = (int16_t)(fft_out[i].i) * ( 32767.0f /(float)FFT_SAMPLES );
            }
        break;
    }
}
