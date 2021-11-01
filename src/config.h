
#ifndef CONFIG_H_
#define CONFIG_H_

/* use following when you are using the esp32 audio kit v2.2 */
#define ESP32_AUDIO_KIT /* project has not been tested on other hardware, modify on own risk */
#define ES8388_ENABLED /* use this if the Audio Kit is equipped with ES8388 instead of the AC101 */

#define SAMPLE_BUFFER_SIZE  64

/* on board led */
#define BLINK_LED_PIN     19 // IO19 -> D5

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT


#ifdef ES8388_ENABLED
    
    #define ES8388_ADDR 0x10

    #define ES8388_PIN_SDA  18
    #define ES8388_PIN_SCL  23

    #define ES8388_PIN_MCLK 0
    #define ES8388_PIN_SCLK 5
    #define ES8388_PIN_LRCK 25
    #define ES8388_PIN_DIN  26
    #define ES8388_PIN_DOUT 35


    /* i2c shared with codec */
    #define I2C_SDA ES8388_PIN_SDA
    #define I2C_SCL ES8388_PIN_SCL
#endif

// AUdIO KIT PORTS
#define GPIO_PA_EN                  GPIO_NUM_21
#define GPIO_SEL_PA_EN              GPIO_SEL_21


#endif /* CONFIG_H_ */
