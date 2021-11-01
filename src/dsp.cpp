#include <cstdint>
#include <stdio.h>
#include <math.h>

#include "dsp.h"
#include "filters.h"


static volatile float peak_i;
static volatile float peak_q;

static volatile float peakm_i;
static volatile float peakm_q;


static float dc_i;
static float dc_q;
static float acg_gain = 1.0f;

const float dc_filter_k = 0.9f;


static float filter_buf_i[ IF_FILT1_TAPS + 1 ];
static float filter_buf_q[ IF_FILT1_TAPS + 1 ];
static float sin_32[32];

uint32_t cos_idx = 32 / 4;
uint32_t sin_idx = 0;


void dsp_init() {

	for (auto i = 0; i < 32; i++) {
		sin_32[i] = (float)sin(i * 2.0 * M_PI / 32.0 );
	}


    for ( auto i=0; i < IF_FILT1_TAPS + 1; i++) {
        filter_buf_i[i] = (float)i * 0.0f;
        filter_buf_q[i] = (float)i * 0.0f;
    }

    dc_i = 0.0f;
    dc_q = 0.0f;

    acg_gain = 1.0f;
}



void dsp_demod_weaver_sample(float* psample_i, float* psample_q) {
	
	auto sample_i = *psample_i;
	auto sample_q = *psample_q;

	// insert new sample filter into the filter buffer
	filter_buf_i[IF_FILT1_TAPS] = sample_i;
	filter_buf_q[IF_FILT1_TAPS] = sample_q;

	sample_i = 0.0f;
	sample_q = 0.0f;

	for (auto i = 0; i < IF_FILT1_TAPS; i++) {
		
		// get next sample
		auto inp_i = filter_buf_i[i + 1];
		auto inp_q = filter_buf_q[i + 1];
		
		// shift sample in buffer
		filter_buf_i[i] = inp_i;
		filter_buf_q[i] = inp_q;

		// apply LPF filter
		sample_i += inp_i * lpf1_coeffs[i];
		sample_q += inp_q * lpf1_coeffs[i];
	}

	// BFO (BW filter /2 )
	sample_i *= sin_32[cos_idx];
	sample_q *= sin_32[sin_idx];

	cos_idx = (cos_idx + 1 ) % 32;
	sin_idx = (sin_idx + 1) % 32;

	float gain = 50.0;
	*psample_i = ( sample_i + sample_q ) * gain;
	*psample_q = *psample_i;
}




// inline bool do_rx_sample( float* psample_i, float* psample_q ) {

//     float sample_q = *psample_q;
//     float sample_i = *psample_i;

//     //last_sample = sin_table[phase];
//     //phase = (phase + 17)%256;

//     //*psample_i = last_sample * 0.99f;
//     //*psample_q = last_sample * 0.50f;


//     if ( sample_q > peak_q ) peak_q = sample_q;
//     if ( sample_i > peak_i ) peak_i = sample_i;

//     if ( sample_q < peakm_q ) peakm_q = sample_q;
//     if ( sample_i < peakm_i ) peakm_i = sample_i;


//     *psample_i = ( sample_q + sample_i ) / 2.0 ;
//     *psample_q = ( sample_q + sample_i ) / 2.0 ;

//     return true;

// 	/* 
// 	 * Shift-in I and Q raw samples 
// 	 */

//     for ( auto i=0; i < LPF_FILTER_TAPS-1; i++) {
//         sig_raw_q[i] = sig_raw_q[i+1];
//         sig_raw_i[i] = sig_raw_i[i+1];
//     }

//     // add new sample to end of buffer
//     sig_raw_q[LPF_FILTER_TAPS-1] = sample_q;
//     sig_raw_i[LPF_FILTER_TAPS-1] = sample_i;

// 	/*
// 	 * Low pass filter + decimation
// 	 */
// 	rx_cnt = (rx_cnt+1)&3;                          // Calculate only every fourth sample
// 	if (rx_cnt>0) {
//         *psample_i = last_sample;
//         *psample_q = last_sample;
//         return true;                                // So net sample time is 64us or 15.625 kHz
//     }
	
// 	for (auto i=0; i<LPF_FILTER_TAPS-1; i++)        // Shift decimated samples
// 	{
// 		q_s[i] = q_s[i+1];
// 		i_s[i] = i_s[i+1];
// 	}

// 	float q_accu = 0;                               // Initialize accumulators
// 	float i_accu = 0;

// 	for (auto i=0; i<LPF_FILTER_TAPS; i++)          // Low pass FIR filter
// 	{
// 		q_accu += sig_raw_q[i]*lpf3_62[i];	        // Fc=3kHz, at 62.5 kHz raw sampling		
// 		i_accu += sig_raw_i[i]*lpf3_62[i];            // Fc=3kHz, at 62.5 kHz raw sampling	
// 	}

// 	q_s[LPF_FILTER_TAPS-1] = q_accu;
// 	i_s[LPF_FILTER_TAPS-1] = i_accu;

// 	/*** DEMODULATION ***/

// 	/* 
// 	 * USB demodulate: I[7] - Qh, 
// 	 * Qh is Classic Hilbert transform 15 taps, 12 bits (see Iowa Hills calculator)
// 	 */	
// 	q_accu =    (q_s[0]-q_s[14])*( 315.0f/4096.0f) + 
//                 (q_s[2]-q_s[12])*( 440.0f/4096.0f) + 
//                 (q_s[4]-q_s[10])*( 734.0f/4096.0f) + 
//                 (q_s[6]-q_s[ 8])*(2202.0f/4096.0f);


//     return false;
// }
