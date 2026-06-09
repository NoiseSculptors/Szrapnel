
#include "gpio.h"
#include "i2c.h"
#include "rcc.h"

static void i2c_timingr(uint32_t periph, uint32_t apb_freq)
{

    (void)apb_freq;
/* TODO: calculate using apb_freq and i2c_speed 
static void i2c_timingr(uint32_t periph, uint32_t apb_freq, uint32_t i2c_speed) */

/* very fast, just for testing */

#define PRESC_VAL   0x4     /* timing prescaler, test 1-4 */
#define SCLDEL_VAL  0x0     /* data setup time */
#define SDADEL_VAL  0x0     /* data hold time */
#define SCLH_VAL    0x01    /* SCL high period */
#define SCLL_VAL    0x04    /* SCL low period */

    uint32_t timingr = 0;

    timingr = ((PRESC_VAL & 0xf)  << 28)|
             ((SCLDEL_VAL & 0xf)  << 20)| 
             ((SDADEL_VAL & 0xf)  << 16)|
               ((SCLH_VAL & 0xff) <<  8)|
               ((SCLL_VAL & 0xff) <<  0);

    *I2C_TIMINGR(periph) = timingr;
}

/* Write N bytes in a single write transfer (blocking) */
void i2c_write_bytes(uint32_t i2c_periph,
                     uint8_t addr,
                     const uint8_t *data,
                     uint16_t n)
{
    if (n == 0) return;

    /* CR2: SADD(7-bit<<1), RD_WRN=0, NBYTES, START=1, AUTOEND=0 */
    unsigned cr2 = ((unsigned)(addr) << 1) |
                   ((unsigned)n << 16) |
                   (1u << 13);

    *I2C_CR2(i2c_periph) = cr2;

    for (uint16_t i=0; i<n; i++) {

        /* wait txis */
        while(((*I2C_ISR(i2c_periph)) & (1u<<1)) == 0){} /* TXIS */

        *I2C_TXDR(i2c_periph) = data[i];
    }

    /* wait tc */
    while(((*I2C_ISR(i2c_periph)) & (1u<<6)) == 0){} /* TC */

    /* stop */
    *I2C_CR2(i2c_periph) |= (1u<<14); /* STOP */
}

#define I2C4EN 7
#define I2C3EN 23
#define I2C2EN 22
#define I2C1EN 21
#define ANFOFF 12 /* analog filter; disable for higher frequencies */
#define PE 0

/* AF4 PB6=SCL PB7=SDA */
void init_i2c1_pb6_pb7(const clock_info_t *ci)
{

    /* GPIO clocks */
    *RCC_AHB4ENR  |= (1u << GPIOBEN);

    /* I2C1 clock (APB1) */
    *RCC_APB1LENR |= (1u << I2C1EN);

    /* AF */
    *GPIOB_MODER &= ~(0x3u<<(6*2))  & ~(0x3u<<(7*2));
    *GPIOB_MODER |=  (0x2u<<(6*2))  |  (0x2u<<(7*2));

    /* Open-drain */
    *GPIOB_OTYPER |= (0x1u<<6) | (0x1u<<7);

    /* Pull-up */
    *GPIOB_PUPDR &= ~(0x3u<<(6*2))  & ~(0x3u<<(7*2));
    *GPIOB_PUPDR |=  (0x1u<<(6*2))  |  (0x1u<<(7*2));

    /* AF4 */
    *GPIOB_AFRL &= ~(0xfu<<(6*4)) & ~(0xfu<<(7*4));
    *GPIOB_AFRL &= ~(0b1111<<(6*4)) & ~(0b1111<<(7*4));
    *GPIOB_AFRL |=  (4<<(6*4))     |  (4<<(7*4));

    /* Disable */
    *I2C1_CR1 = 0;

    i2c_timingr(I2C1, ci->pclk1_apb1_hz);  /* configure *I2C_TIMINGR */

    *I2C1_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF4 PB8=SCL PB9=SDA */
void init_i2c1_pb8_pb9(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIOBEN);

    /* I2C1 clock (APB1) */
    *RCC_APB1LENR |= (1u << I2C1EN);

    /* PB8, PB9 -> AF */
    *GPIOB_MODER &= ~(0x3u<<(8*2)) & ~(0x3u<<(9*2));
    *GPIOB_MODER |=  (0x2u<<(8*2)) |  (0x2u<<(9*2));

    /* Open-drain */
    *GPIOB_OTYPER |= (1u<<8) | (1u<<9);

    /* Pull-up */
    *GPIOB_PUPDR &= ~(0x3u<<(8*2)) & ~(0x3u<<(9*2));
    *GPIOB_PUPDR |=  (0x1u<<(8*2)) |  (0x1u<<(9*2));

    /* AF4 */
    *GPIOB_AFRH &= ~(0xFu<<((8-8)*4)) & ~(0xFu<<((9-8)*4));
    *GPIOB_AFRH |=  (4u<<((8-8)*4))   |  (4u<<((9-8)*4));

    /* Disable */
    *I2C1_CR1 = 0;

    /* Timing on APB1 */
    i2c_timingr(I2C1, ci->pclk1_apb1_hz);

    *I2C1_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF4 PB10=SCL PB11=SDA */
void init_i2c2_pb10_pb11(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIOBEN);

    /* I2C2 clock (APB1) */
    *RCC_APB1LENR |= (1u << I2C2EN);

    /* PB10, PB11 -> AF */
    *GPIOB_MODER &= ~(0x3u<<(10*2)) & ~(0x3u<<(11*2));
    *GPIOB_MODER |=  (0x2u<<(10*2)) |  (0x2u<<(11*2));

    /* Open-drain */
    *GPIOB_OTYPER |= (1u<<10) | (1u<<11);

    /* Pull-up */
    *GPIOB_PUPDR &= ~(0x3u<<(10*2)) & ~(0x3u<<(11*2));
    *GPIOB_PUPDR |=  (0x1u<<(10*2)) |  (0x1u<<(11*2));

    /* AF4 */
    *GPIOB_AFRH &= ~(0xFu<<((10-8)*4)) & ~(0xFu<<((11-8)*4));
    *GPIOB_AFRH |=  (4u<<((10-8)*4))   |  (4u<<((11-8)*4));

    /* Disable */
    *I2C2_CR1 = 0;

    /* Timing on APB1 */
    i2c_timingr(I2C2, ci->pclk1_apb1_hz);

    *I2C2_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF4 PA8=SCL  PC9=SDA */
void init_i2c3_pa8_pc9(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIOAEN) | (1u << GPIOCEN);

    /* I2C3 clock (APB1) */
    *RCC_APB1LENR |= (1u << I2C3EN);

    /* PA8 -> AF */
    *GPIOA_MODER &= ~(0x3u<<(8*2));
    *GPIOA_MODER |=  (0x2u<<(8*2));
    *GPIOA_OTYPER |= (1u<<8);
    *GPIOA_PUPDR &= ~(0x3u<<(8*2));
    *GPIOA_PUPDR |=  (0x1u<<(8*2));
    *GPIOA_AFRH  &= ~(0xFu<<((8-8)*4));
    *GPIOA_AFRH  |=  (4u<<((8-8)*4));

    /* PC9 -> AF */
    *GPIOC_MODER &= ~(0x3u<<(9*2));
    *GPIOC_MODER |=  (0x2u<<(9*2));
    *GPIOC_OTYPER |= (1u<<9);
    *GPIOC_PUPDR &= ~(0x3u<<(9*2));
    *GPIOC_PUPDR |=  (0x1u<<(9*2));
    *GPIOC_AFRH  &= ~(0xFu<<((9-8)*4));
    *GPIOC_AFRH  |=  (4u<<((9-8)*4));

    /* Disable */
    *I2C3_CR1 = 0;

    /* Timing on APB1 */
    i2c_timingr(I2C3,ci->pclk1_apb1_hz);

    *I2C3_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF4 PD12=SCL PD13=SDA */
void init_i2c4_pd12_pd13(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIODEN);

    /* I2C4 clock (APB4) */
    *RCC_APB4ENR |= (1u << I2C4EN);

    /* PD12, PD13 -> AF */
    *GPIOD_MODER &= ~(0x3u<<(12*2)) & ~(0x3u<<(13*2));
    *GPIOD_MODER |=  (0x2u<<(12*2)) |  (0x2u<<(13*2));

    /* Open-drain */
    *GPIOD_OTYPER |= (1u<<12) | (1u<<13);

    /* Pull-up */
    *GPIOD_PUPDR &= ~(0x3u<<(12*2)) & ~(0x3u<<(13*2));
    *GPIOD_PUPDR |=  (0x1u<<(12*2)) |  (0x1u<<(13*2));

    /* AF4 */
    *GPIOD_AFRH &= ~(0xFu<<((12-8)*4)) & ~(0xFu<<((13-8)*4));
    *GPIOD_AFRH |=  (4u<<((12-8)*4))   |  (4u<<((13-8)*4));

    /* Disable */
    *I2C4_CR1 = 0;

    /* Timing on APB4 */
    i2c_timingr(I2C4,ci->pclk4_apb4_hz);

    *I2C4_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF4 PB6=SCL  PD7=SDA */
void init_i2c4_pb6_pd7(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIOBEN) | (1u << GPIODEN);

    /* I2C4 clock (APB4) */
    *RCC_APB4ENR |= (1u << I2C4EN);

    /* PB6 -> AF */
    *GPIOB_MODER &= ~(0x3u<<(6*2));
    *GPIOB_MODER |=  (0x2u<<(6*2));
    *GPIOB_OTYPER |= (1u<<6);
    *GPIOB_PUPDR &= ~(0x3u<<(6*2));
    *GPIOB_PUPDR |=  (0x1u<<(6*2));
    *GPIOB_AFRL  &= ~(0xFu<<(6*4));
    *GPIOB_AFRL  |=  (4u<<(6*4));

    /* PD7 -> AF */
    *GPIOD_MODER &= ~(0x3u<<(7*2));
    *GPIOD_MODER |=  (0x2u<<(7*2));
    *GPIOD_OTYPER |= (1u<<7);
    *GPIOD_PUPDR &= ~(0x3u<<(7*2));
    *GPIOD_PUPDR |=  (0x1u<<(7*2));
    *GPIOD_AFRL  &= ~(0xFu<<(7*4));
    *GPIOD_AFRL  |=  (4u<<(7*4));

    /* Disable */
    *I2C4_CR1 = 0;

    /* Timing on APB4 */
    i2c_timingr(I2C4, ci->pclk4_apb4_hz);

    *I2C4_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}


/* AF6 PB8=SCL  PD9=SDA */
void init_i2c4_pb8_pd9(const clock_info_t *ci)
{
    /* GPIO clocks */
    *RCC_AHB4ENR |= (1u << GPIOBEN) | (1u << GPIODEN);

    /* I2C4 clock (APB4) */
    *RCC_APB4ENR |= (1u << I2C4EN);

    /* PB8 -> AF */
    *GPIOB_MODER &= ~(0x3u<<(8*2));
    *GPIOB_MODER |=  (0x2u<<(8*2));
    *GPIOB_OTYPER |= (1u<<8);
    *GPIOB_PUPDR &= ~(0x3u<<(8*2));
    *GPIOB_PUPDR |=  (0x1u<<(8*2));
    *GPIOB_AFRH  &= ~(0xFu<<((8-8)*4));
    *GPIOB_AFRH  |=  (6u<<((8-8)*4));

    /* PD9 -> AF */
    *GPIOD_MODER &= ~(0x3u<<(9*2));
    *GPIOD_MODER |=  (0x2u<<(9*2));
    *GPIOD_OTYPER |= (1u<<9);
    *GPIOD_PUPDR &= ~(0x3u<<(9*2));
    *GPIOD_PUPDR |=  (0x1u<<(9*2));
    *GPIOD_AFRH  &= ~(0xFu<<((9-8)*4));
    *GPIOD_AFRH  |=  (6u<<((9-8)*4));

    /* Disable */
    *I2C4_CR1 = 0;

    /* Timing on APB4 */
    i2c_timingr(I2C4, ci->pclk4_apb4_hz);

    *I2C4_CR1 = (1<<ANFOFF)|(1<<PE); /* analog filter off, enable */
}

