
#include "gpio.h"
#include "io.h"
#include "rcc.h"
#include "spi_i2s.h"

/* PCM5102A is connected to:
   A4  I2S1_WS
   A5  I2S1_CK
   A7  I2S1_SDO
*/

void io_dac_init(void)
{
    /* start gpiox clocks */
    *RCC_AHB4ENR |= (1<<GPIOAEN);

    gpio_ctrl(GPIOA, GPIO_OSPEED, GPIO4|GPIO5|GPIO7, OSPEED_MEDIUM);
    gpio_ctrl(GPIOA, GPIO_MODE, GPIO4|GPIO5|GPIO7, MODE_AF);
    gpio_ctrl(GPIOA, GPIO_AFRL, GPIO4|GPIO5|GPIO7, AF5);

    /* start spi1 clock */
#define SPI1EN 12
    *RCC_APB2ENR |= (1<<SPI1EN);

    /* disable I2S */
#define SPE 0
    *SPI1_CR1 &= ~(1<<SPE); 

#define SPI123SEL 12
    /* 0: pll1_q_ck clock selected as SPI/I2S123 kernel clock,
       default after reset */
    *RCC_D2CCIP1R &= ~(0b111<<SPI123SEL);
    /* 1:pll2_p_ck as SPI(I2S)1,2,3 kernel clock
       STM32H750 has 3 independant PLL blocks,
       use pll2 with FRAC to fine-tune the kernel clock */
    *RCC_D2CCIP1R |= (1u<<SPI123SEL);

#define I2SMOD  0
#define I2SCFG  1 /* 010: master - transmit */
#define I2SSTD  4 /* 0:I2S Philips, 1: MSB justified, 2: LSB, 3: PCM standard */
#define DATALEN 8
#define CHLEN   10
#define I2SDIV  16  
#define MCKOE   25 /* enable master clock output */
    /* enable I2S mode, master transmit, Philips mode,  */
    *SPI1_I2SCFGR = 0;
    *SPI1_I2SCFGR |= (0b1<<MCKOE)    |
#if   SAMPLING_FREQ == 48
                     (4 << I2SDIV)   |
#elif SAMPLING_FREQ == 96
                     (2 << I2SDIV)   |
#elif SAMPLING_FREQ == 192
                     (1 << I2SDIV)   |
#elif SAMPLING_FREQ == 384
                     (0 << I2SDIV)   |
#else
#  error "Unsupported SAMPLING_FREQ"
#endif
                     (0b1<<CHLEN)    | /* 0-16bit 1-32bit */
                     (0b01<<DATALEN) | /* 0-16bit 1-24bit 2-32bit */
                     (0b00<<I2SSTD)  |
                     (0b010<<I2SCFG) |
                     (0b1<<I2SMOD);

    /* enable I2S */
    *SPI1_CR1 |= (1<<SPE);

#define CSTART 9
    *SPI1_CR1 |= (1<<CSTART);
}

