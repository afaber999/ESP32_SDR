#ifndef TESTS_H_INCLUDED
#define TESTS_H_INCLUDED

// void weaver_performance_test(int loops);
// void sin_performance_test(int loops);
// void test_kiss_fft();

#ifdef USE_PSRAM
    void test_fft_in_psram();
#endif

#endif