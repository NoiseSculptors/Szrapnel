#include "delay.h"

#include "gpio.h"
#include "init.h"
#include "io.h"
#include "rcc.h"
#include "rng.h"
#include "spi_i2s.h"
#include <stdint.h>
#include <math.h>

static void init_i2s(void)
{
    /* start GPIOA clock */
    *RCC_AHB4ENR |= (1<<GPIOAEN);

    gpio_ctrl(GPIOA, GPIO_OSPEED, GPIO4|GPIO5|GPIO7, OSPEED_MEDIUM);
    gpio_ctrl(GPIOA, GPIO_MODE,   GPIO4|GPIO5|GPIO7, MODE_AF);
    gpio_ctrl(GPIOA, GPIO_AFRL,   GPIO4|GPIO5|GPIO7, AF5);

    /* start SPI1 clock */
#define SPI1EN 12
    *RCC_APB2ENR |= (1<<SPI1EN);

    /* disable I2S  */
#define SPE 0
    *SPI1_CR1 &= ~(1<<SPE); 

#define SPI123SEL 12
    /* SPI/I2S1,2 and 3 kernel clock source selection */
    *RCC_D2CCIP1R &= ~(0b111<<SPI123SEL);
    *RCC_D2CCIP1R |= (0b1<<SPI123SEL); /* use pll2 kernel clock */

#define I2SMOD  0
#define I2SCFG  1 /* 010: master - transmit */
#define I2SSTD  4 /* 0:I2S Philips, 1: MSB justified, 2: LSB, 3: PCM standard */
#define DATALEN 8 /* 00:16bit, 01:24bit, 10:32bit */
#define CHLEN   10
#define I2SDIV  16  
    /* enable I2S mode, Philips mode,  */
    *SPI1_I2SCFGR = 0;
    *SPI1_I2SCFGR |= (6<<I2SDIV)  | /* 384k sampling freq, 24.576MHz */
                     (1u<<CHLEN)   | /* 0-16bit 1-32bit */
                     (2u<<DATALEN) | /* 0-16bit 1-24bit 2-32bit */
                     (0u<<I2SSTD)  |
                     (2u<<I2SCFG)  |
                     (1u<<I2SMOD);

    /* enable I2S */
    *SPI1_CR1 |= (1<<SPE);

#define CSTART 9
    *SPI1_CR1 |= (1<<CSTART);
}

/*
 Send data in I2S3, do a simple blocking write: wait until TX FIFO has space,
 then write.
 */
static void i2s1_send_data(uint32_t data)
{
            while ((*SPI1_SR & (1<<1)) == 0) {} // TXE:1
            *SPI1_TXDR = data;
}

    

/*
 Generate a 1kHz test square wave 
 */
static void i2s1_test_square_wave(void)
{
    static int32_t amplitudeL = 2147483647;
    static int32_t amplitudeR = -2147483648;
    static int counter = 0;

    i2s1_send_data(amplitudeL);
    i2s1_send_data(amplitudeR);

    counter++;

    if (counter >= 192) {
        amplitudeL = (amplitudeL > 0) ? -2147483648 : 2147483647;
        amplitudeR = (amplitudeR > 0) ? -2147483648 : 2147483647;
        counter = 0;
    }
}

/*
 Generate a 1kHz test sinewave 
 */
static void i2s1_test_sine_wave(void)
{
    static float phase = 0.0f;
    const float two_pi = 6.28318530718f;
    const float freq = 1000.0f;          // 1kHz test tone
    const float sample_rate = 384000.0f;
    const float step = two_pi * freq / sample_rate;
    const float amplitude = 1.0f;

    phase += step;
    if (phase >= two_pi)
        phase = 0.0f;

    float l__sample = sinf(phase);
    float r__sample = sinf(phase);

    int32_t l_sample = (int32_t)(l__sample * -2147483647.0f * amplitude);
    int32_t r_sample = (int32_t)(r__sample * 2147483647.0f * amplitude);

    i2s1_send_data(l_sample); // Left
    i2s1_send_data(r_sample); // Right

}

/*
 Generate a 1kHz sawtooth signal 
 */
static void i2s1_sawtooth (void)
{

    static int32_t sampleL = 0;
    static int32_t sampleR = 0;
    const float step = 2000.0f/384000.0f * 2147483647.0f;

    i2s1_send_data(sampleL);
    i2s1_send_data(sampleR);

    sampleL += (int32_t)step ;
    sampleR -= (int32_t)step ;

    if (sampleL >= 2147483647)
        sampleL = -2147483648;

    if (sampleR <= -2147483647)
        sampleR = 2147483648;
}

/*
 Generate a 1kHz triangle signal 
 */
static void i2s1_triangle(void)
{
    static int8_t upL = 1, upR = 0;

    static int32_t sampleL = 0;
    static int32_t sampleR = 0;
    const int32_t step = (int32_t)(4000.0f / 384000.0f * 2147483647.0f);

    if (upL){
        if (sampleL >= 2147483647 - step) upL = 0;
        else sampleL += step;
    } else {
        if (sampleL <= -2147483648 + step) upL = 1;
        else sampleL -= step;
    }

    if (upR){
        if (sampleR >= 2147483647 - step) upR = 0;
        else sampleR += step;
    } else {
        if (sampleR <= -2147483648 + step) upR = 1;
        else sampleR -= step;
    }

    i2s1_send_data(sampleL);
    i2s1_send_data(sampleR);
}

/*
 Generate noise signal 
 */
static void i2s1_noise(void)
{
    int32_t noiseL, noiseR;

        noiseL = rng_rnd();
        noiseR = rng_rnd();
        i2s1_send_data(noiseL);
        i2s1_send_data(noiseR);
    //for(int i=0;i<192;i++){
   // }

}

int main(void)
{

    uint16_t bg = rgb565(0xff,0xff,0x00);
    uint16_t fg = 0;
    uint8_t active_signal = 0;

    init_clock();
    init_rng();

    pll_2_start(PLLSRC_HSE, 16, 181, 3968, 1, 128, 128, 26000000u);

    io_lcd_init();

    lcd_clear_flush(bg);
    lcd_printf(5,10,1,1,fg,bg,"demo:\n\n"
                              "1kHz square\n"
                              "     sine\n"
                              "     sawtooth\n"
                              "     triangle\n"
                              "     noise");
    lcd_flush_fb();

    init_i2s();

    while(1)
    {
        for(int i=0; i<500000;i++){
            switch(active_signal){
                case 0: i2s1_test_square_wave(); break;
                case 1: i2s1_test_sine_wave(); break;
                case 2: i2s1_sawtooth(); break;
                case 3: i2s1_triangle(); break;
                case 4: i2s1_noise(); break;
            }
        }

        active_signal++;
        active_signal %= 5;
    }

    return 0;
}

