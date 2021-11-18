
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include "tests.h"
#include "dsp_fft.h"
#include "kiss_fft.h"

#ifdef USE_PSRAM
    static void dump_info() {
        Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
        Serial.printf("ESP.getFreeHeap()    : %d\n", ESP.getFreeHeap());
        Serial.printf("ESP.getMinFreeHeap() : %d\n", ESP.getMinFreeHeap());
        Serial.printf("ESP.getHeapSize()    : %d\n", ESP.getHeapSize());
        Serial.printf("ESP.getMaxAllocHeap(): %d\n", ESP.getMaxAllocHeap());
        Serial.printf("Total PSRAM          : %d\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM           : %d\n", ESP.getFreePsram());
        Serial.printf("Used PSRAM           : %d\n", ESP.getPsramSize() - ESP.getFreePsram());
        Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    }

    void test_fft_in_psram()
    {
        const uint32_t nfft = 2048;

        Serial.printf(" ------------- TEST FFT IN PSRAM ------------ \n");

        dump_info();
        auto time1 = esp_timer_get_time();

        auto fft_buf1 = (kiss_fft_cpx*)ps_malloc(sizeof( kiss_fft_cpx ) * nfft );
        auto fft_buf2 = (kiss_fft_cpx*)ps_malloc(sizeof( kiss_fft_cpx ) * nfft );

        auto time2 = esp_timer_get_time();

        auto fft_cfg_fwd = kiss_fft_alloc(nfft, 0, NULL, NULL);
        auto fft_cfg_rev = kiss_fft_alloc(nfft, 1, NULL, NULL);

        float delta_phase = 6.28f / 30.0f;

        float phase = delta_phase;

        for ( auto i=0; i< nfft; i++ ) {
            fft_buf1[i].r = sinf( phase );
            fft_buf1[i].i = cosf( phase );
            phase += delta_phase;
        }

        kiss_fft(fft_cfg_fwd , fft_buf1 , fft_buf2);
        kiss_fft(fft_cfg_rev, fft_buf2, fft_buf1);

        dump_info();
        free(fft_buf1);
        free(fft_buf2);

        dump_info();

        auto time3 = esp_timer_get_time();
        Serial.printf("Time1 %lld time2 = %lld\n", time2 -time1, time3 - time2);
        dump_info();
    }
#endif

// const uint32_t nfft = 2048;
// static kiss_fft_cpx fft_in[nfft];
// static kiss_fft_cpx fft_int[nfft];
// static kiss_fft_cpx fft_out[nfft];

// void test_kiss_fft()
// {
//     printf("size of complex %d\n", sizeof(kiss_fft_cpx));
	

// 	auto cfg_forward  = kiss_fft_alloc(nfft, 0, NULL, NULL);
// 	auto cfg_backward = kiss_fft_alloc(nfft, 1, NULL, NULL);

// 	for ( auto i = 0; i< nfft; i++ )
// 	{
//         auto phase = i * 2.0 * 3.14159267 * 21 / nfft;
//         fft_in[i].r = cos(phase);
//         fft_in[i].i = cos(phase);
//     }


//     auto time1 = esp_timer_get_time();
//     auto loops = 44;
//     for (auto i = 0; i < loops; i++)
//     {
//     	kiss_fft(cfg_forward , fft_in , fft_int);
//     	kiss_fft(cfg_backward, fft_int, fft_out);
//     }
//     auto time2 = esp_timer_get_time();
//     Serial.printf("test_sin %d loops => time: %lld\n", loops, time2 - time1);

//     yield();
//     for (auto i = 0; i < nfft/10; i++)
//     {
//         if (i % 10 == 0) printf("%04d ", i);
//         printf("%6.3f %6.3f ", fft_out[i].r, fft_out[i].i);
//         if (i % 10 == 9) printf("\n");
//     }
//     yield();
//     // for (auto i = 0; i < nfft; i++)
//     // {
//     //     if (i % 10 == 0) printf("%04d ", i);
//     //     printf("%6.3f ",sqrt(fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i));
//     //     if (i % 10 == 9) printf("\n");
//     // }
	
// }


// void sin_performance_test(int loops) {
//     float res = 0.0f;
//     float phase = 0.0f;
//     auto time1 = esp_timer_get_time();

//     for ( auto l=0; l < loops; l++) {
//         res += sin( phase );
//         phase += 0.000001f;
//     }
//     auto time2 = esp_timer_get_time();
//     Serial.printf("test_sin %d loops => time: %lld\n", loops, time2 - time1);
//     Serial.print(res);
// }

// void weaver_performance_test(int loops) {

//     dsp_init();

//     auto time1 = esp_timer_get_time();
//     //float sum_total = 0.0f;

//     float res = 0.0f;
//     for ( auto l=0; l < loops; l++) {

//         auto sample_i = 0.1f + 0.00001f * l;
//         auto sample_q = 0.2f + 0.00001f * l;

//         dsp_demod_weaver_sample(&sample_i, &sample_q);
//         res = sample_i + sample_q;
//     }
//     auto time2 = esp_timer_get_time();
//     Serial.printf("weaver_performance_test %d loops => time: %lld\n", loops, time2 - time1);
//     Serial.print(res);
// }
