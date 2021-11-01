
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "i2s_interface.h"
#include "es8388.h"
#include "i2c_scanner.h"

#include "dsp.h"
#include "tests.h"


static volatile uint32_t loop_cnt_1hz;
static volatile int64_t last_time1;
static volatile int64_t last_time2;
static volatile int64_t last_time3;
static volatile bool show_times = false;


static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];


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
    Serial.println( " --- Core0TaskSetup ---");
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
//        auto pk_q = peak_q;
//        auto pk_i = peak_i;
//        peak_q = 0.0f;
//        peak_i = 0.0f;

  //      auto pkm_q = peakm_q;
    //    auto pkm_i = peakm_i;
      //  peakm_q = 0.0f;
        //peakm_i = 0.0f;

        //Serial.printf("Time1 %lld time2 = %lld\n", last_time2 -last_time1, last_time3 - last_time2);
        //Serial.printf("PEAK VALUES Q= %5.3f I = %5.3f\n", pk_q, pk_i);
        //Serial.printf("PEAK MIN VALUES Q= %5.3f I = %5.3f\n", pkm_q, pkm_i);

        loop_cnt_1hz = 0;
    }
}

void Core0Task(void *parameter)
{
    Core0TaskSetup();

    while (true)
    {
        Core0TaskLoop();
        delay(1);
        yield();
    }
}

inline void Core0TaskInit()
{
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}


volatile int rx_cnt=0;
float last_sample =0;


inline void do_rx() {
    for ( auto i=0; i < SAMPLE_BUFFER_SIZE; i++) {
        //do_rx_sample( &fl_sample[i], &fr_sample[i] );
        dsp_demod_weaver_sample(&fl_sample[i], &fr_sample[i]);
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


    weaver_performance_test(1);
    weaver_performance_test(10);
    weaver_performance_test(100);
    weaver_performance_test(1000);
    weaver_performance_test(10000);
    weaver_performance_test(44100 * 1 );
    weaver_performance_test(44100 * 4 );
    dsp_init();

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

