
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "i2s_interface.h"
#include "es8388.h"
#include "i2c_scanner.h"

#include "filters.h"


static volatile uint32_t loop_cnt_1hz;
static volatile int64_t last_time1;
static volatile int64_t last_time2;
static volatile int64_t last_time3;
static volatile bool show_times = false;


static volatile float peak_i;
static volatile float peak_q;

static volatile float peakm_i;
static volatile float peakm_q;

static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];

#define FILTER_BUFFER_SIZE (HILBERT_TAPS+1)

static float filter_buf_i[ FILTER_BUFFER_SIZE ];
static float filter_buf_q[ FILTER_BUFFER_SIZE ];

static float dc_i;
static float dc_q;
static float acg_gain = 1.0f;

const float dc_filter_k = 0.9f;

inline void dsp_init() {

    for ( auto i=0; i < FILTER_BUFFER_SIZE; i++) {
        filter_buf_i[i] = (float)i * 0.0f;
        filter_buf_q[i] = (float)i * 0.0f;
    }

    dc_i = 0.0f;
    dc_q = 0.0f;

    acg_gain = 1.0f;
}

inline void perf_test(int loops) {

    auto time1 = esp_timer_get_time();
    float sum_total = 0.0f;

    float last_i = 0.0f;
    float last_q = 0.0f;

    for ( auto l=0; l < loops; l++) {

        auto sample_i = 0.1f + 0.00001f * l;
        auto sample_q = 0.2f + 0.00001f * l;

        // keep track of DC in I and Q channel
        dc_i += ( sample_i - dc_i) * dc_filter_k;
        dc_q += ( sample_q - dc_q) * dc_filter_k;

        // subtract DC for both channels
        sample_i -= dc_i;
        sample_q -= dc_q;

        // apply automatic gain
        sample_i *= acg_gain;
        sample_q *= acg_gain;

        // insert new sample filter into the filter buffer
        filter_buf_i[ HILBERT_TAPS ] = sample_i;
        filter_buf_q[ HILBERT_TAPS ] = sample_q;

        // apply Hilbert bandpass/phase shift filter for I and Q channel
        for ( auto i=0; i < HILBERT_TAPS; i++) {

            // get next sample
            auto inp_i = filter_buf_i[i + 1];
            auto inp_q = filter_buf_q[i + 1];

            // shift sample in buffer
            filter_buf_i[i] = inp_i;
            filter_buf_q[i] = inp_q;
            
            // apply Hilber tranform filter
            sample_i += inp_i * hilbert_00[i]; 
            sample_q += inp_q * hilbert_90[i]; 

        }

        last_i = sample_i;
        last_q = sample_q;

        sum_total += sample_i + sample_q;
    }

    auto time2 = esp_timer_get_time();
    Serial.printf("Float performance test %d loops => Time1 %lld\n", loops, time2 - time1);
    Serial.printf("dc_i %10.2f dc_q %10.2f\n", dc_i, dc_q);
    Serial.printf("last_i %10.2f last_q %10.2f\n", last_i, last_q);
    Serial.print(sum_total);
}

float sin_table[256];
int phase = 0;



static void dump_info() {
    Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("ES8388 REGISTER DUMP\n");
    //scanner_scan(I2C_SDA, I2C_SCL, 400000);

    es8388_read_range(0, 53);
    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}




TaskHandle_t  Core0TaskHnd ;


void Core0TaskSetup()
{
    Serial.println( " &&&&&&&&&&&&&&&&&&&&&&& CORE task 0 setup ");
    ES8388_Setup();
    ES8388_SelectInput(ADC_CHANNEL_1);    
}

void Core0TaskLoop()
{    
    if (Serial.available() > 0) {
        auto bt  = Serial.read();
        switch ( bt ) {

            case '1':
                Serial.println("Switch to ADC input 1: ");
                //ES8388_SetMicGain(4);
                ES8388_SelectInput(ADC_CHANNEL_1);
            break;
            case '2':
                Serial.println("Switch to ADC input 2: ");
                //ES8388_SetMicGain(5);
                ES8388_SelectInput(ADC_CHANNEL_2);
            break;
            case '3':
                Serial.println("Switch to ADC DIFF input 1: ");
                //ES8388_SetMicGain(5);
                ES8388_SelectInput(ADC_CHANNEL_DIFF_1);
            break;
            case '4':
                Serial.println("Switch to ADC DIFF input 2: ");
                //ES8388_SetMicGain(5);
                ES8388_SelectInput(ADC_CHANNEL_DIFF_2);
            break;
            case 'x':
                Serial.println("-------------------");
                es8388_read_range(0x09, 0x0C);
            break;

            case 'r':
                dump_info();
            break;
            case 'i':
                scanner_scan(I2C_SDA, I2C_SCL, 400000);
            break;
            case 't':
                show_times = !show_times;
            break;
            default:
                // say what you got:
                Serial.print("Unknown command: ");
                Serial.println(bt);
        }
    }

    if ( ( loop_cnt_1hz ) >= SAMPLE_RATE && show_times)
    {
        auto pk_q = peak_q;
        auto pk_i = peak_i;
        peak_q = 0.0f;
        peak_i = 0.0f;

        auto pkm_q = peakm_q;
        auto pkm_i = peakm_i;
        peakm_q = 0.0f;
        peakm_i = 0.0f;

        Serial.printf("Time1 %lld time2 = %lld\n", last_time2 -last_time1, last_time3 - last_time2);
        Serial.printf("PEAK VALUES Q= %5.3f I = %5.3f\n", pk_q, pk_i);
        Serial.printf("PEAK MIN VALUES Q= %5.3f I = %5.3f\n", pkm_q, pkm_i);

        loop_cnt_1hz = 0;
    }
}

void Core0Task(void *parameter)
{
    Serial.println( " &&&&&&&&&&&&&&&&&&&&&&& Core0TaskLoop ");
    Core0TaskSetup();

    while (true)
    {
        Core0TaskLoop();
        /* this seems necessary to trigger the watchdog */
        delay(1);
        yield();
    }
}


inline void Core0TaskInit()
{
    /* we need a second task for the terminal output */
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}



#define LPF_FILTER_TAPS 15

static float sig_raw_i[LPF_FILTER_TAPS];
static float sig_raw_q[LPF_FILTER_TAPS];
static float i_s[LPF_FILTER_TAPS];
static float q_s[LPF_FILTER_TAPS];					// Filtered I/Q samples


static float lpf3_62[LPF_FILTER_TAPS] = {
    0.1f, 0.1f, 0.2f, 0.2f,0.1f, 0.1f, 0.2f, 0.2f,0.1f, 0.1f, 0.2f, 0.2f,0.1f, 0.1f, 0.2f
};


// int16_t lpf3_62[15] =  {  3,  3,  5,  7,  9, 10, 11, 11, 11, 10,  9,  7,  5,  3,  3};	// Pass: 0-3000, Stop: 6000-31250
// int16_t lpf3_31[15] =  { -2, -3, -3,  1, 10, 21, 31, 35, 31, 21, 10,  1, -3, -3, -2};	// Pass: 0-3000, Stop: 6000-15625
// int16_t lpf3_15[15] =  {  3,  4, -3,-14,-15,  6, 38, 53, 38,  6,-15,-14, -3,  4,  3};	// Pass: 0-3000, Stop: 4500-7812
// int16_t lpf7_62[15] =  { -2, -1,  1,  7, 16, 26, 33, 36, 33, 26, 16,  7,  1, -1, -2};	// Pass: 0-7000, Stop: 10000-31250
// int16_t lpf7_31[15] =  { -1,  4,  9,  2,-12, -2, 40, 66, 40, -2,-12,  2,  9,  4, -1};	// Pass: 0-7000, Stop: 10000-15625
// int16_t lpf15_62[15] = { -1,  3, 12,  6,-12, -4, 40, 69, 40, -4,-12,  6, 12,  3, -1};	// Pass: 0-15000, Stop: 20000-31250


volatile int rx_cnt=0;
float last_sample =0;




inline bool do_rx_sample( float* psample_i, float* psample_q ) {

    float sample_q = *psample_q;
    float sample_i = *psample_i;

    //last_sample = sin_table[phase];
    //phase = (phase + 17)%256;

    //*psample_i = last_sample * 0.99f;
    //*psample_q = last_sample * 0.50f;


    if ( sample_q > peak_q ) peak_q = sample_q;
    if ( sample_i > peak_i ) peak_i = sample_i;

    if ( sample_q < peakm_q ) peakm_q = sample_q;
    if ( sample_i < peakm_i ) peakm_i = sample_i;


    *psample_i = ( sample_q + sample_i ) / 2.0 ;
    *psample_q = ( sample_q + sample_i ) / 2.0 ;

    return true;

	/* 
	 * Shift-in I and Q raw samples 
	 */

    for ( auto i=0; i < LPF_FILTER_TAPS-1; i++) {
        sig_raw_q[i] = sig_raw_q[i+1];
        sig_raw_i[i] = sig_raw_i[i+1];
    }

    // add new sample to end of buffer
    sig_raw_q[LPF_FILTER_TAPS-1] = sample_q;
    sig_raw_i[LPF_FILTER_TAPS-1] = sample_i;

	/*
	 * Low pass filter + decimation
	 */
	rx_cnt = (rx_cnt+1)&3;                          // Calculate only every fourth sample
	if (rx_cnt>0) {
        *psample_i = last_sample;
        *psample_q = last_sample;
        return true;                                // So net sample time is 64us or 15.625 kHz
    }
	
	for (auto i=0; i<LPF_FILTER_TAPS-1; i++)        // Shift decimated samples
	{
		q_s[i] = q_s[i+1];
		i_s[i] = i_s[i+1];
	}

	float q_accu = 0;                               // Initialize accumulators
	float i_accu = 0;

	for (auto i=0; i<LPF_FILTER_TAPS; i++)          // Low pass FIR filter
	{
		q_accu += sig_raw_q[i]*lpf3_62[i];	        // Fc=3kHz, at 62.5 kHz raw sampling		
		i_accu += sig_raw_i[i]*lpf3_62[i];            // Fc=3kHz, at 62.5 kHz raw sampling	
	}

	q_s[LPF_FILTER_TAPS-1] = q_accu;
	i_s[LPF_FILTER_TAPS-1] = i_accu;

	/*** DEMODULATION ***/

	/* 
	 * USB demodulate: I[7] - Qh, 
	 * Qh is Classic Hilbert transform 15 taps, 12 bits (see Iowa Hills calculator)
	 */	
	q_accu =    (q_s[0]-q_s[14])*( 315.0f/4096.0f) + 
                (q_s[2]-q_s[12])*( 440.0f/4096.0f) + 
                (q_s[4]-q_s[10])*( 734.0f/4096.0f) + 
                (q_s[6]-q_s[ 8])*(2202.0f/4096.0f);


    return false;
}


inline void do_rx() {

    for ( auto i=0; i < SAMPLE_BUFFER_SIZE; i++) {
        do_rx_sample( &fl_sample[i], &fr_sample[i] );
    }

}


void setup()
{
   for ( int i=0; i< 256; i++ ) {
        sin_table[i] = sin(i * 2 * 3.14159 / 256);
    }

    delay(500);
    Serial.begin(115200);
    Serial.println();

    dsp_init();


    perf_test(1);
    perf_test(10);
    perf_test(100);
    perf_test(1000);
    perf_test(10000);
    perf_test(44100 * 1 );
    perf_test(44100 * 4 );

    setup_i2s();
    Serial.printf("inialized DAC \n");

    WiFi.mode(WIFI_OFF);
    btStop();

    dump_info();

    esp_timer_init();

    // ENABLE LOUDSPEAKER (SET CTRL = GPIO32 TO HIGH)
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);


    Core0TaskInit();
}




// sample loop
void loop()
{
    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;
    loop_cnt_1hz+= SAMPLE_BUFFER_SIZE;

    auto time1 = esp_timer_get_time();

    memset(fl_sample, 0, sizeof(fl_sample));
    memset(fr_sample, 0, sizeof(fr_sample));

    i2s_read_stereo_samples_buff(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);
    auto time2 = esp_timer_get_time();

    do_rx();


    /* function blocks and returns when sample is put into buffer */
    if (i2s_write_stereo_samples_buff(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE))
    {
        /* nothing for here */
    }
   auto time3 = esp_timer_get_time();


    // i2s_read_stereo_samples(&fl_sample, &fr_sample);
    // Delay_Process(&fl_sample, &fr_sample);

    // if (i2s_write_stereo_samples(&fl_sample, &fr_sample))
    // {
    //     /* nothing for here */
    // }

    last_time1 = time1;
    last_time2 = time2;
    last_time3 = time3;
}

