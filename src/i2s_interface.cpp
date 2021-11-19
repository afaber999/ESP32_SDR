#include <Arduino.h>
#include "config.h"
#include "i2s_interface.h"
#include <driver/i2s.h>

const i2s_port_t i2s_port_number = I2S_NUM_0;

// WRITE DMA_SAMPLES per channel at once
// fl_sample and fr_sample must be large enough to hold DMA_SAMPLES
bool i2s_write_buffer(int16_t* data)
{
    static size_t bytes_written = 0;    
    i2s_write(  i2s_port_number, 
                (const char *)data,
                2 * sizeof(int16_t) *DMA_SAMPLES,
                &bytes_written, 
                portMAX_DELAY);

    return ( bytes_written == DMA_SAMPLES * 2 * sizeof(uint16_t));
}


// READ DMA_SAMPLES
// two channels as in16
bool i2s_read_buffer(int16_t* data)
{
    static size_t bytes_read = 0;    
    i2s_read(   i2s_port_number, 
                (char *)data, 
                2 * sizeof(int16_t) *DMA_SAMPLES, 
                &bytes_read, 
                portMAX_DELAY);
    return ( bytes_read == DMA_SAMPLES * 2 * sizeof(uint16_t));
}


i2s_config_t i2s_configuration =
{
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // | I2S_MODE_DAC_BUILT_IN
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = DMA_BUFFERS,
    .dma_buf_len = DMA_SAMPLES,   // each buffer 2 x 2 x 1024 = 4k
    .use_apll = true,
};

i2s_pin_config_t pins =
{
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num =  I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_DIN_PIN
};

void setup_i2s()
{
    i2s_driver_install(i2s_port_number, &i2s_configuration, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_set_sample_rates(i2s_port_number, SAMPLE_RATE);
    i2s_start(i2s_port_number);

    // for es8388??
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
}
