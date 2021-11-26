#include <Arduino.h>
#include <cstdint>
#include <cassert>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include "dsp_fft.h"
#include "filters.h"
#include "post_filter.h"
#include "kiss_fft.h"
#include "config.h"

#define SAMPLE_TO_FLOAT(x) ((float)(x)* (1.0f / 32767.0f))

//#define DEBUG_FILTER (1)
static const float F_PI = (float)M_PI;
static const float F_2PI = 2.0f * (float)M_PI;

float inline power2dB(float x) {
    return 10.0f * log10f(x);
}

static uint32_t dsp_loop_cnt = 0;
volatile DSP_FFT_FILT_SETTINGS dsp_fft_filt;

static const uint32_t fft_filt_l = FFT_SAMPLES / 2;                 // 1024
static const uint32_t fft_filt_m = fft_filt_l + 1;                  // 1025
static const uint32_t fft_filt_n = fft_filt_l + fft_filt_m - 1;     // FFT_SAMPLES = 2048

auto fft_cfg_fwd = kiss_fft_alloc(FFT_SAMPLES, 0, NULL, NULL);

static kiss_fft_cpx fft_in[FFT_SAMPLES];
static kiss_fft_cpx fft_buf1[FFT_SAMPLES];
static kiss_fft_cpx fft_buf2[FFT_SAMPLES];
static kiss_fft_cpx fft_filt_coeffs[FFT_SAMPLES];

static volatile float peak_i;
static volatile float peak_q;

// Bessel helper functions for kaiser window
static float i0(float z) {

    const auto t = (z * z) / 4.0f;
    auto sum = 1 + t;
    auto term = t;
    for (auto k = 2; k < 40; k++) {
        term *= t / ((float)k * (float)k);
        sum += term;
        if (term < 1e-12f * sum)
            break;
    }
    return sum;
}

static void make_kaiser_window(int size, float* window, float beta)
{
    auto midn = size / 2;
    auto midm1 = (size - 1) / 2;

    const auto numc = F_PI * beta;
    const auto inv_denom = 1.0f / i0(numc);
    const auto pc = 2.0f / (size - 1);

    // The window is symmetrical, so compute only half of it and mirror
    // this won't compute the middle value in an odd-length sequence
    for (int n = 0; n < midn; n++) {
        auto const p = pc * (float)n - 1.0f;
        window[size - 1 - n] = window[n] = i0(numc * sqrtf(1 - p * p)) * inv_denom;
    }
    // If sequence length is odd, middle value is unity
    if (size & 1) {
        window[midm1] = 1;
    }
}


void tune_bpf_filter(float low, float high, float window_param) {

    // Serial.printf("High set to: low: %f high: %f %f %f \n", dsp_fft_filt.low, dsp_fft_filt.high,low, high );

    // borrow fft_buf2, has enough room to hold the entire window
    auto window = (float*)fft_buf2;
    make_kaiser_window(fft_filt_m, window, window_param);

    float fl = low;
    float fh = high;
    float fc = (fh - fl) / 2.0f;
    float ff = (fl + fh) * F_PI;
    int midpoint = fft_filt_m >> 1;

    // make sure were using a clean buffer
    memset(fft_buf1, 0, sizeof(fft_buf1));

    for (int i = 1; i <= fft_filt_m; i++)
    {
        int j = i - 1;
        int k = i - midpoint;
        float amp = 0.0f;
        if (k != 0) {
            amp = (sinf(F_2PI * k * fc) / (F_PI * k)) * window[j];
        }
        else {
            amp = (fh - fl);
        }

        float phase = k * ff;
        fft_buf1[j].r = amp * cosf(phase);
        fft_buf1[j].i = amp * sinf(phase);
    }

    // Convert from time domain to frequency domain
    kiss_fft(fft_cfg_fwd, fft_buf1, fft_filt_coeffs);
    
#ifdef DEBUG_FILTER
    for (int n = 0; n < 200; n++) {
        Serial.printf("#%d\t%.1f\n", n,
            power2dB(fft_filt_coeffs[n].r * fft_filt_coeffs[n].r + fft_filt_coeffs[n].i * fft_filt_coeffs[n].i) );
    }

    auto pf = fopen("c:\\temp\\window_mag4.txt", "w");
    for (int n = 0; n < fft_filt_n; n++) {
        fprintf(pf, "#%d\t%.1f\t%g\t%g\n", n,
            power2dB(fft_filt_coeffs[n].r * fft_filt_coeffs[n].r + fft_filt_coeffs[n].i * fft_filt_coeffs[n].i),
            fft_filt_coeffs[n].r,
            fft_filt_coeffs[n].i);
    }
    fclose(pf);
#endif
}


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


bool dsp_fft_init() {
    //post_filter_select(SSB_3000);
    dsp_fft_filt.low = 300.0f;
    dsp_fft_filt.high = 2900.0f;
    // force initial update of the bpf filter
    dsp_fft_filt.updated_at = 0;
    return (DMA_SAMPLES == fft_filt_l);
}


void dsp_fft_demod(int16_t* psamples, DEMOD_MODE mode, int32_t shift_bins) {

    dsp_loop_cnt++;

    if (dsp_fft_filt.updated_at == 0 ) {
        tune_bpf_filter(dsp_fft_filt.low / (float)SAMPLE_RATE, dsp_fft_filt.high / (float)SAMPLE_RATE, 5.0f);
        dsp_fft_filt.updated_at = dsp_loop_cnt;
    }

    // overlap and add
    int idx = FFT_SAMPLES / 2;
    auto new_samples = psamples;
    bool swap_iq = (mode == DEMOD_CWL) || (mode == DEMOD_LSB);

    //swap_iq = false;

    if (swap_iq) {
        for (int n = 0; n < fft_filt_l; n++)
        {
            fft_in[n] = fft_in[idx];
            fft_in[idx].i = SAMPLE_TO_FLOAT(*new_samples++);
            fft_in[idx].r = SAMPLE_TO_FLOAT(*new_samples++);

            peak_i = fmaxf(peak_i, fft_in[idx].r);
            peak_q = fmaxf(peak_q, fft_in[idx].i);
            idx++;
        }
    }
    else {
        for (int n = 0; n < fft_filt_l; n++)
        {
            fft_in[n] = fft_in[idx];
            fft_in[idx].r = SAMPLE_TO_FLOAT(*new_samples++);
            fft_in[idx].i = SAMPLE_TO_FLOAT(*new_samples++);

            peak_i = fmaxf(peak_i, fft_in[idx].r);
            peak_q = fmaxf(peak_q, fft_in[idx].i);
            idx++;
        }
    }

    // from time domain to frequency domain
    kiss_fft(fft_cfg_fwd, fft_in, fft_buf1);

     // apply shift, can be optimized with mask out step
    //int32_t shift_bins = 0;

    // Shift is independent of LSB and USB since IQ are 
    // already swapped for LSB
    // shift must be even number of bins
    // a negative idx will shift up
    // a positive idx will shift down
    auto shift_idx = (shift_bins < 0)? shift_bins + fft_filt_n : shift_bins;

    for (int32_t i = 0; i < fft_filt_n; i++) {
        if (shift_idx >= fft_filt_n)
            shift_idx -= fft_filt_n;
        fft_buf2[i] = fft_buf1[shift_idx++];
    }

    // mask out
    switch (mode) {
        case DEMOD_CWL:
        case DEMOD_CWU:
        case DEMOD_LSB:
        case DEMOD_USB:

            // apply filter (complex multiply)
            for (auto i = 0; i < fft_filt_n; i++) {
                float ra = fft_buf2[i].r;
                float ia = fft_buf2[i].i;
                float rb = fft_filt_coeffs[i].r;
                float ib = fft_filt_coeffs[i].i;

                // swap I&Q so we can use the FWD FFT transfer
                // (ra * j ia ) * (rb * j ib ) = 
                // ra * rb - ia * ib  + j ( ra * ib + ia * rb)
                fft_buf2[i].i = ra * rb - ia * ib;
                fft_buf2[i].r = ra * ib + ia * rb ;
            }
        break;

        case DEMOD_AM:
        case DEMOD_FM:
        case DEMOD_NONE:
        default:
            // apply filter (complex multiply)
            for (auto i = 0; i < fft_filt_n; i++) {
                float ra = fft_buf2[i].r;
                float ia = fft_buf2[i].i;
                // swap I&Q so we can use the FWD FFT transfer
                fft_buf2[i].i = ra;
                fft_buf2[i].r = ia;
            }
        break;
    }

	// DO FWD FFT transfer, notice that I&Q are swapped
    // so we can perform a FWD FFT transform
    kiss_fft(fft_cfg_fwd, fft_buf2, fft_buf1);

	// I&Q are sapped again since we use FWD FFT to calculate the inverse FFT
    switch (mode) {
        case DEMOD_AM:
        case DEMOD_CWL:
        case DEMOD_CWU:
        case DEMOD_FM:
        case DEMOD_LSB:
        case DEMOD_USB:
        case DEMOD_NONE:
        default:

            // Overlap and add, take 2nd half of FFT output buffer
            for (auto i = FFT_SAMPLES / 2; i < FFT_SAMPLES; i++) {
                // beware that I & Q are swapped since we used the 
                // fft_cfg_fwd configuration to perform an inverse FFT
                *psamples++ = (int16_t)(fft_buf1[i].i * (32767.0f / (float)FFT_SAMPLES));
                *psamples++ = (int16_t)(fft_buf1[i].r * (32767.0f / (float)FFT_SAMPLES));
            }
        break;
    }
}

