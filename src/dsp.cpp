#include <cstdint>
#include <stdio.h>
#include <math.h>

#include "dsp.h"
#include "filters.h"
#include "post_filter.h"

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



static float dc_i;
static float dc_q;
static float acg_gain = 1.0f;

const float dc_filter_k = 0.9f;


static float filter_buf_i[ IF_FILT1_TAPS + 1 ];
static float filter_buf_q[ IF_FILT1_TAPS + 1 ];

// At sampling rate 9f 44100 Hz -> shift down frequency = 44100 / SHIFT_DOWN_STEPS = 44100 / 32 = 1378 Hz
// for CW use / 64 -> 689 Hz?
const uint32_t SHIFT_DOWN_STEPS = 32;
static float sin_32[SHIFT_DOWN_STEPS];
uint32_t cos_idx = SHIFT_DOWN_STEPS / 4; // 90 degree phase shift
uint32_t sin_idx = 0;


void dsp_init() {

    post_filter_select(SSB_3000);

	for (auto i = 0; i < SHIFT_DOWN_STEPS; i++) {
		sin_32[i] = sinf(i * 2.0f * (float)M_PI / (float)SHIFT_DOWN_STEPS );
	}

    for ( auto i=0; i < IF_FILT1_TAPS + 1; i++) {
        filter_buf_i[i] = (float)i * 0.0f;
        filter_buf_q[i] = (float)i * 0.0f;
    }

    dc_i = 0.0f;
    dc_q = 0.0f;

    acg_gain = 1.0f;
}

void dsp_pass_thru_sample(float* psample_i, float* psample_q) {
	
	auto sample_i = *psample_i;
	auto sample_q = *psample_q;

    if ( sample_i > peak_i ) peak_i = sample_i;
    if ( sample_q > peak_q ) peak_q = sample_q;

    *psample_i = (sample_i + sample_q ) /2.0f;
    *psample_q = (sample_i + sample_q ) /2.0f;
}


void dsp_demod_weaver_sample(float* psample_i, float* psample_q) {
	
	auto sample_i = *psample_i;
	auto sample_q = *psample_q;

    if ( sample_i > peak_i ) peak_i = sample_i;
    if ( sample_q > peak_q ) peak_q = sample_q;

	// insert new sample filter into the filter buffer
	filter_buf_i[IF_FILT1_TAPS] = sample_i;
	filter_buf_q[IF_FILT1_TAPS] = sample_q;

    // clear filter results
	sample_i = 0.0f;
	sample_q = 0.0f;

    // apply LP filter
	for (auto i = 0; i < IF_FILT1_TAPS; i++) {
		
		// get next sample
		auto inp_i = filter_buf_i[i + 1];
		auto inp_q = filter_buf_q[i + 1];
		
		// shift sample in buffer
		filter_buf_i[i] = inp_i;
		filter_buf_q[i] = inp_q;

		// multiply LP filter coeffs
		sample_i += inp_i * lpf1_coeffs[i];
		sample_q += inp_q * lpf1_coeffs[i];
	}

	// BFO (BW filter /2 )
	sample_i *= sin_32[cos_idx];
	sample_q *= sin_32[sin_idx];

	cos_idx = (cos_idx + 1 ) % SHIFT_DOWN_STEPS;
	sin_idx = (sin_idx + 1) % SHIFT_DOWN_STEPS;

    // TODO ADD AGC
	auto gain = 10.0f;
    auto audio_sample = ( sample_i + sample_q );


    audio_sample *= gain; 

    // post filtering
    audio_sample = post_filter_sample(audio_sample);


	*psample_i = audio_sample;
	*psample_q = audio_sample;

}
