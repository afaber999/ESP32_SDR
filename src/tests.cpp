
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include "tests.h"
#include "dsp.h"

void weaver_performance_test(int loops) {

    dsp_init();

    auto time1 = esp_timer_get_time();
    //float sum_total = 0.0f;

    float res = 0.0f;
    for ( auto l=0; l < loops; l++) {

        auto sample_i = 0.1f + 0.00001f * l;
        auto sample_q = 0.2f + 0.00001f * l;

        dsp_demod_weaver_sample(&sample_i, &sample_q, true);
        res = sample_i + sample_q;
    }
    auto time2 = esp_timer_get_time();
    Serial.printf("weaver_performance_test %d loops => time: %lld\n", loops, time2 - time1);
    Serial.print(res);
}
