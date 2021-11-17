
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "i2s_interface.h"
#include "es8388.h"
#include "i2c_scanner.h"
#include "si5351.h"
#include "dsp.h"
#include "tests.h"
#include "post_filter.h"
#include "hmi.h"
#include "radio.h"
#include "push_button.h"

static volatile uint32_t loop_cnt_1hz;
static volatile int64_t last_time1;
static volatile int64_t last_time2;
static volatile int64_t last_time3;
static volatile bool show_times = false;
static volatile bool is_usb = true;

static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];

float sin_table[256];
int phase = 0;

volatile int rx_cnt=0;
float last_sample =0;

uint32_t vfo_frequency = 7074000;


static InvPushButton ptt_button(PIN_PTT_BUTTON);
static InvPushButton rot_up(PIN_ROT_UP);
static InvPushButton rot_down(PIN_ROT_DOWN);
static InvPushButton rot_button(PIN_ROT_BUTTON);

static void dump_info() {
    Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("ES8388 REGISTER DUMP\n");
    //scanner_scan(I2C_SDA, I2C_SCL, 400000);
    es8388_read_range(0, 53);

    Serial.printf("PTT button %d\n", ptt_button.is_pressed());


    Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

}

static uint32_t timer_cnt;
static hw_timer_t * timer = NULL;

// 1 ms timer
void IRAM_ATTR timer_isr() {
    timer_cnt++;
    
    // 10 ms 
    if (timer_cnt % 10 == 0) {
        ptt_button.update();
        rot_up.update();
        rot_down.update();
        rot_button.update();
    }
}

void dsp_task_setup()
{
    Serial.println( " --- DspTaskSetup ---");
    for ( int i=0; i< 256; i++ ) {
        sin_table[i] = sin(i * 2 * 3.14159 / 256);
    }

    // weaver_performance_test(1);
    // weaver_performance_test(10);
    // weaver_performance_test(100);
    // weaver_performance_test(1000);
    // weaver_performance_test(10000);
    weaver_performance_test(44100 * 1 );
    weaver_performance_test(44100 * 2 );

    // sin_performance_test(1);
    // sin_performance_test(10);
    // sin_performance_test(100);
    // sin_performance_test(1000);
    // sin_performance_test(10000);
    // sin_performance_test(44100 * 1 );
    // sin_performance_test(44100 * 4 );
    // test_kiss_fft();

    dsp_init();
    setup_i2s();
}


void dsp_task_loop()
{
    const bool usb = true;

    // DSP loop runs on core 0
    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;
    loop_cnt_1hz+= SAMPLE_BUFFER_SIZE;

    auto time1 = esp_timer_get_time();

    // REQUIRED ? memset(fl_sample, 0, sizeof(fl_sample));
    // REQUIRED ? memset(fr_sample, 0, sizeof(fr_sample));

    // get sample block from ADC
    i2s_read_stereo_samples_buff(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);
    auto time2 = esp_timer_get_time();

    // process block
    if (usb) {
        for ( auto i=0; i < SAMPLE_BUFFER_SIZE; i++) {
            dsp_demod_weaver_sample(&fl_sample[i], &fr_sample[i]);
        }
    } else {
        for ( auto i=0; i < SAMPLE_BUFFER_SIZE; i++) {
            dsp_demod_weaver_sample(&fr_sample[i], &fl_sample[i]);
        }
    }

    // send data block to DAC
    if (i2s_write_stereo_samples_buff(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE))
    {
    }
   auto time3 = esp_timer_get_time();

    last_time1 = time1;
    last_time2 = time2;
    last_time3 = time3;
}

void radio_setup()
{
    // Araduino setup, runs on core 1
    esp_timer_init();
    WiFi.mode(WIFI_OFF);
    btStop();

    Serial.begin(115200);
    Serial.println("Start ESP32 SDR v0.2c... stay tuned");

    // contol I2C from core 1
    ES8388_Setup();
    ES8388_SelectInput(ADC_CHANNEL_2);
    ES8388_SetMicGain(3);    

    si_init();
    hmi_init();

    // ENABLE LOUDSPEAKER (SET CTRL = GPIO32 TO HIGH)
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);

	SI_SETFREQ(0, vfo_frequency);
    SI_SETPHASE(0, 1);

    // setup timer 
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &timer_isr, true);
    timerAlarmWrite(timer, 1000, true);
    timerAlarmEnable(timer);
}

void radio_loop()
{
    // Araduino looop, runs on core 1
    si_evaluate();
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

            case 'U':
                Serial.println("Set to USB ");
                is_usb = true;
            break;
            case 'u':
                Serial.println("Set to LSB ");
                is_usb = false;
            break;

            case 'f': {
                Serial.println("Select filter 0..9 ");
                while (Serial.available() ==0 ) {
                    delay(1);
                }
                auto bti  = Serial.read();

                switch ( bti ) {
                    default:
                    case '1':
                        Serial.println("Filter SSB_1700");
                        post_filter_select(SSB_1700);
                    break;
                    case '2':
                        Serial.println("Filter SSB_2500");
                        post_filter_select(SSB_2500);
                    break;
                    case '4':
                        Serial.println("Filter SSB_2700");
                        post_filter_select(SSB_2700);
                    break;
                    case '5':
                        Serial.println("Filter SSB_3000");
                        post_filter_select(SSB_3000);
                    break;
                    case '6':
                        Serial.println("Filter CW_650_60");
                        post_filter_select(CW_650_60);
                    break;
                    case '7':
                        Serial.println("Filter CW_700_100");
                        post_filter_select(CW_700_100);
                    break;
                    case '8':
                        Serial.println("Filter CW_750_200");
                        post_filter_select(CW_750_200);
                    break;
                    case '9':
                        Serial.println("Filter CW_750_500");
                        post_filter_select(CW_750_500);
                    break;
                }
            } break;
            case 'm': {
                Serial.println("Set MIC gain 0..8 ");
                while (Serial.available() ==0 ) {
                    delay(1);
                }
                auto mic_gain  = Serial.read()- (int)'0';
                if ( mic_gain >=0 && mic_gain<= 9 ) {
                    Serial.print("to ");
                    Serial.println(mic_gain, DEC);
                    ES8388_SetMicGain(mic_gain);    
                }
            }
            case '+':
                Serial.println("Increment freq by 1000 Hz ");
                SI_INCFREQ(0,1000);
                vfo_frequency = SI_GETFREQ(0);
                Serial.printf("New freq: %d", vfo_frequency);
            break;
            case '-':
                Serial.println("Decrement freq by 100000 Hz ");
                SI_DECFREQ(0,100000);
                vfo_frequency = SI_GETFREQ(0);
                Serial.printf("New freq: %d", vfo_frequency);
            break;
            case 'q':
                Serial.printf("Current timer %d\n", timer_cnt);
            break;

        }
    }

    if ( ( loop_cnt_1hz ) >= SAMPLE_RATE && show_times)
    {
        auto pk_i = get_peak_i();
        auto pk_q = get_peak_q();
        clear_peak();

        //Serial.printf("Time1 %lld time2 = %lld\n", last_time2 -last_time1, last_time3 - last_time2);
        Serial.printf("PEAK VALUES Q= %5.3f I = %5.3f\n", pk_q, pk_i);
        loop_cnt_1hz = 0;
        //hmi_show_keys();
    }
}

