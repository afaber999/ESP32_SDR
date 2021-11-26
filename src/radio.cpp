
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "i2s_interface.h"
#include "i2c_interface.h"
#include "es8388.h"
#include "si5351.h"
#include "dsp_fft.h"
#include "tests.h"
#include "post_filter.h"
#include "hmi.h"
#include "radio.h"
#include "push_button.h"
#include "kiss_fft.h"

bool has_si = false;
static volatile bool mute = false;
static volatile bool generate_tone = false;
static volatile bool bypass_mode = false;
//static DEMOD_MODE demod_mode = DEMOD_NONE;
static volatile DEMOD_MODE demod_mode = DEMOD_LSB;

static volatile uint32_t loop_cnt_1hz;
static volatile int64_t last_time1;
static volatile int64_t last_time2;
static volatile int64_t last_time3;
static volatile int64_t last_time4;
static volatile int64_t last_time9;
static volatile bool show_times = false;
static volatile bool is_usb = true;

static int16_t samples[DMA_SAMPLES * 2];

const uint32_t sin_table_size = 256;
float sin_table[sin_table_size];
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
    Serial.printf("ESP.getFreeHeap()    : %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() : %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize()    : %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap(): %d\n", ESP.getMaxAllocHeap());

    #ifdef USE_PSRAM
        Serial.printf("Total PSRAM          : %d\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM           : %d\n", ESP.getFreePsram());
    #endif

    Serial.printf("ES8388 REGISTER DUMP\n");
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
    for ( int i=0; i< sin_table_size; i++ ) {
        sin_table[i] = sinf(i * 2 * 3.14159f / (float)sin_table_size);
    }
    // for ( int i=0; i< sin_table_size; i++ ) {
    //     printf( "Sin[%d] = %6.2f ", i, sin_table[i]);
    // }

    // test_kiss_fft();
    if (!dsp_fft_init() ) {
        Serial.println("Failed to intialize DSP\n");
    }
    setup_i2s();
}

static int beat = 0;

void dsp_task_loop()
{
    // if ( (beat % 100 )== 0) {
    //     Serial.println("Still alive\n");
    // }
    // beat++;
    
    //const bool usb = true;
    // DSP loop runs on core 0

    last_time9 = esp_timer_get_time() - last_time1;
    auto time1 = esp_timer_get_time();

    // get sample block from ADC (DMA_SAMPLES)
    // since we have 2 channels, samples should 
    // hold 2 * DMA_SAMPLES of type int16_t
    if  (! i2s_read_buffer(samples) ) {
        Serial.println( "Can't read ADC samples\n");
    }

    // if ( beat == 1000) {
    //     Serial.println("Phase/amp check dump\n");
    //     for (auto i =0; i< DMA_SAMPLES; i++ ) {
    //         Serial.printf( "%d,%d,%d\n",i, samples[2*i], samples[2*i + 1]);
    //     }
    //     Serial.println("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    // }
    beat++;

    auto time2 = esp_timer_get_time();

    uint32_t phase1 = 0;
    uint32_t phase2 = 0;
    
    if (generate_tone) {
        // generate test pattern
        for ( auto i =0; i< DMA_SAMPLES; i++ ) {
            samples[2*i + 0 ] =  (int16_t)((sin_table[phase1 ]) * 0.5f * 32767.0f );
            samples[2*i + 1 ] =  (int16_t)((sin_table[phase1 ] + sin_table[phase2 ]) * 0.4 * 32767.0f);
            phase1 = (phase1 + 3 ) % sin_table_size;
            phase2 = (phase2 + 7 ) % sin_table_size;
        }
    }
    
    if ( !bypass_mode ) {
        dsp_fft_demod(samples, demod_mode, -618);
    }
    auto time3 = esp_timer_get_time();

    // send data block to DAC
    if (!i2s_write_buffer(samples) ) {
        Serial.println( "ERROR: can't write samples to DAC\n");
    }
    auto time4 = esp_timer_get_time();

    last_time1 = time1;
    last_time2 = time2;
    last_time3 = time3;
    last_time4 = time4;
    loop_cnt_1hz += DMA_SAMPLES;
}

void radio_setup()
{
    // Araduino setup, runs on core 1
    esp_timer_init();
    WiFi.mode(WIFI_OFF);
    btStop();

    Serial.begin(115200);
    Serial.println("Start ESP32 SDR v0.2d... stay tuned");

    i2c_init(I2C_SDA, I2C_SCL, 100000);
    has_si = i2c_scan() > 1;
    Serial.printf("Has SI %d\n", has_si);

    // contol I2C from core 1
    ES8388_Setup();
    ES8388_SelectInput(ADC_CHANNEL_2);
    ES8388_SetMicGain(0);

    if ( has_si ) {
        si_init();
    }
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

    // Serial.printf("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
    // auto psdRamBuffer = ps_malloc(500000);
    // Serial.printf("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
    // free(psdRamBuffer);
    // Serial.printf("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
}

void radio_loop()
{
    // Araduino looop, runs on core 1
    if ( has_si ) {
        si_evaluate();
    }
    
    if (Serial.available() > 0) {
        auto bt  = Serial.read();
        switch ( bt ) {

            case '1':
                Serial.println("Switch to ADC input 1: ");
                ES8388_SelectInput(ADC_CHANNEL_1);
            break;
            case '2':
                Serial.println("Switch to ADC input 2: ");
                ES8388_SelectInput(ADC_CHANNEL_2);
            break;
            case '3':
                Serial.println("Switch to ADC DIFF input 1: ");
                ES8388_SelectInput(ADC_CHANNEL_DIFF_1);
            break;
            case '4':
                Serial.println("Switch to ADC DIFF input 2: ");
                ES8388_SelectInput(ADC_CHANNEL_DIFF_2);
            break;
            case 'x':
                Serial.println("-------------------");
                es8388_read_range(0x09, 0x0C);
            break;

            case 'r':
                dump_info();
                Serial.printf("input:%lld processing:%lld output:%lld \n", last_time2 -last_time1, last_time3 - last_time2, last_time4 - last_time3);
            break;
            case 'i':
                i2c_scan();
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
            #ifdef USE_PSRAM
            case 'z':

                //test_fft_in_psram();
            break;
            #endif
            case '/':
                mute = !mute;
                ES8388_SetOUT1VOL(mute ? 0 : 33);
                ES8388_SetOUT2VOL(mute ? 0 : 33);
                Serial.printf( "Mute set to: %d\n", mute);
            break;
            case 'g':
                generate_tone = !generate_tone;
                Serial.printf( "Generate tone set to : %d\n", generate_tone);
            break;     
            case 'b':
                bypass_mode = !bypass_mode;
                Serial.printf( "Bypass mode tone set to : %d\n", bypass_mode);
            break;     
                   
            case 'p': {
                Serial.println("Set passband 2..9 ");
                while (Serial.available() ==0 ) {
                    delay(1);
                }
                auto passband  = Serial.read()- (int)'0';
                dsp_fft_filt.high = (float)passband * 1000.0f;
                dsp_fft_filt.updated_at = 0;
                break;
            }
            case 'd': {
                switch (demod_mode) {
                    case DEMOD_USB:
                        Serial.println("DEMOD SET TO LSB\n");
                        demod_mode = DEMOD_LSB;
                    break;
                    case DEMOD_LSB:
                        Serial.println("DEMOD SET TO NONE\n");
                        demod_mode = DEMOD_NONE;
                    break;
                    case DEMOD_NONE:
                        Serial.println("DEMOD SET TO USB\n");
                        demod_mode = DEMOD_USB;
                    break;
                } 
            }
        }
    }

    if ( ( loop_cnt_1hz ) >= SAMPLE_RATE && show_times)
    {
        auto pk_i = get_peak_i();
        auto pk_q = get_peak_q();
        clear_peak();

        Serial.printf("input:%lld processing:%lld output:%lld period %lld\n", last_time2 -last_time1, last_time3 - last_time2, last_time4 - last_time3,  last_time9);

        Serial.printf("PEAK VALUES Q= %5.3f I = %5.3f\n", pk_q, pk_i);
        loop_cnt_1hz = 0;
        //hmi_show_keys();
    }
}

