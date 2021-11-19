#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef M_PI
    #define M_PI (3.14159274101257324)
#endif

#define ESP32_AUDIO_KIT

// DMA SIZES (experimental values)
// input  1024 samples
// output 1024 samples
// total  2048 samples, margin x 2.5?
// alloc  5120 samples total (requires 20k mem)
static const uint32_t DMA_SAMPLES = 1024;
static const uint32_t DMA_BUFFERS =    5;
static const uint32_t FFT_SAMPLES = 2048;

// on board led
#define BLINK_LED_PIN     19 // IO19 -> D5

// DON't change this, filters are tuned for the frequency
#define SAMPLE_RATE 48000
#define SAMPLE_SIZE_16BIT

// i2c address of es8388
static const uint8_t ES8388_ADDR = 0x10;

static const uint8_t ES8388_PIN_SDA  =18;
static const uint8_t ES8388_PIN_SCL  =23;
static const uint8_t ES8388_PIN_MCLK =0;
static const uint8_t ES8388_PIN_SCLK =5;
static const uint8_t ES8388_PIN_LRCK =25;
static const uint8_t ES8388_PIN_DIN  =26;
static const uint8_t ES8388_PIN_DOUT =35;

/* i2c shared with codec */
static const uint8_t I2C_SDA =ES8388_PIN_SDA;
static const uint8_t I2C_SCL =ES8388_PIN_SCL;

static const uint8_t I2S_MCLK_PIN= ES8388_PIN_MCLK;
static const uint8_t I2S_BCLK_PIN= ES8388_PIN_SCLK;
static const uint8_t I2S_WCLK_PIN= ES8388_PIN_LRCK;
static const uint8_t I2S_DOUT_PIN= ES8388_PIN_DIN;
static const uint8_t I2S_DIN_PIN = ES8388_PIN_DOUT;

// AUDIO KIT PORTS
#define GPIO_PA_EN                  GPIO_NUM_21
#define GPIO_SEL_PA_EN              GPIO_SEL_21

static const uint8_t PIN_K1 =36;
static const uint8_t PIN_K2 =13;
static const uint8_t PIN_K3 =19;
static const uint8_t PIN_K4 =23;
static const uint8_t PIN_K5 =18;
static const uint8_t PIN_K6 =5;

static const uint8_t PIN_PTT_BUTTON = PIN_K1;
static const uint8_t PIN_ROT_BUTTON = PIN_K2;
static const uint8_t PIN_ROT_UP     = PIN_K3;
static const uint8_t PIN_ROT_DOWN   = PIN_K4;

//#define USE_PSRAM

#endif /* CONFIG_H_ */
