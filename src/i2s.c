/* I2S Example

    This example code will output 100Hz sine wave and triangle wave to 2-channel of I2S driver
    it has a sin on a channel and a triangular on the other one (not working fine so far)
    Every 5 seconds, it will change bits_per_sample [16, 24, 32] for i2s data

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include <math.h>


#define SAMPLE_RATE     (36000)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (100)
#define PI              (3.14159265)
#define I2S_BCK_IO      (GPIO_NUM_26) //bit clock line BCLK - continuos serial clock SCK
#define I2S_WS_IO       (GPIO_NUM_25) //word select WS -  left right clock  LRCLK
#define I2S_DO_IO       (GPIO_NUM_22) //serial data SD - multiplexed data line
#define I2S_DI_IO       (-1)

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    //https://youtu.be/ejyt-kWmys8
    //direct memory access: peripheral access memory without cpu
    .dma_buf_count = 6,//how many buff 
    .dma_buf_len = 60,//how long is this buff 
    //TOCHECK is too long it's noisy; the signal doesnt full cover the input signal space 
    .use_apll = false,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
};

i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_IO,
    .ws_io_num = I2S_WS_IO,
    .data_out_num = I2S_DO_IO,
    .data_in_num = I2S_DI_IO                                               //Not used
};

static void setup_triangle_sine_waves(int bits)
{

    int *samples_data = malloc(((bits+8)/16)*SAMPLE_PER_CYCLE*4);
    unsigned int i, sample_val;
    double sin_float, triangle_float, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;
    size_t i2s_bytes_write = 0;

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits+8)/16)*SAMPLE_PER_CYCLE*4);

    triangle_float = -(pow(2, bits)/4 - 1);

    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(i * 2 * PI / SAMPLE_PER_CYCLE);
        if(sin_float >= 0)
            triangle_float += triangle_step;
        else
            triangle_float -= triangle_step;

        sin_float *= (pow(2, bits)/2 - 1);

        if (bits == 16) {
            sample_val = 0;
            sample_val += (short)triangle_float;
            sample_val = sample_val << 16;
            sample_val += (short) sin_float;
            samples_data[i] = sample_val;
        } else if (bits == 24) { //1-bytes unused
            samples_data[i*2] = ((int) triangle_float) << 8;
            samples_data[i*2 + 1] = ((int) sin_float) << 8;
        } else {
            samples_data[i*2] = ((int) triangle_float);
            samples_data[i*2 + 1] = ((int) sin_float);
        }
        //printf("%d\t%d\t%d\n", i, samples_data[i*2], samples_data[i*2+1]);

    }

    i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);

    /*
    clock= sample_rate * bits(I2S bit width) * 2(stereo)
    
    Misurati
    at 16 bit buck frequancy is 1.1429 MHz 
    at 24 bit buck frequancy is 1.6552 MHz
    at 32 bit buck frequancy is 2.4000 MHz
    */


    //it write for the left a sin and for the right a triangular.
    //the noise is caused by the fact that the buffer is to long and the signal is too short
    i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100);

    free(samples_data);
}

void app_main(void)
{
    //for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample)
    //depend on bits_per_sample
    //using 6 buffers, we need 60-samples per buffer
    //if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes
    //if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes
    
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);

    int test_bits = 16;//16
    while (1) {
        setup_triangle_sine_waves(test_bits);
        vTaskDelay(5000/portTICK_RATE_MS); //add a delay 
        test_bits += 8;
        if(test_bits > 32)
            test_bits = 16;//16

    }

}
