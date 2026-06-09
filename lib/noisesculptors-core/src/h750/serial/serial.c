
#include "gpio.h"
#include "rcc.h"
#include "init.h"
#include "memory.h"
#include "serial.h"
#include "usart_uart_lpuart.h"
#include <stdint.h>

/* Configure basic USART settings:
      - Disable USART before configuration
      - Word length: 8 bits
      - Parity: none
      - Stop bits: 1
      - Enable TX and RX */

/* Set baud rate
      - BRR = APB2_CLK / Desired_Baud
      - for calibration - sending 'U' continuously on 115200
        should give about 57.6kHz signal */

/****************************** LPUART1 ***************************************/

#define LPUART1EN 3
#define UART4EN   19
#define UART5EN   20
#define UART7EN   30
#define UART8EN   31
#define USART1EN  4
#define USART2EN  17
#define USART3EN  18
#define USART6EN  5

void init_lpuart1_pa10_pa9(const clock_info_t *ci, unsigned baud)
{
    /* Clocks: GPIOA + LPUART1 (APB1L) */
    *RCC_AHB4ENR  |= (1<<GPIOAEN);
    *RCC_APB4ENR |= (1<<LPUART1EN);

    /* PA9 (TX, AF3), PA10 (RX, AF3) → alternate function */
    /* MODER */
    *GPIOA_MODER &= ~(0b11<<(9*2))  & ~(0b11<<(10*2));
    *GPIOA_MODER |=  (0b10<<(9*2))  |  (0b10<<(10*2));
    /* AFRH (pins >=8) */
    *GPIOA_AFRH &= ~(0b1111<<((9-8)*4)) & ~(0b1111<<((10-8)*4));
    *GPIOA_AFRH |=  (3<<((9-8)*4))     |  (3<<((10-8)*4));

    /* USART config: disable, enable TX/RX, set BRR, enable */
    *LPUART1_CR1 &= ~(1<<0);                /* UE=0 */
    *LPUART1_CR1 |=  (1<<3) | (1<<2);       /* TE|RE */
    *LPUART1_BRR  =   ci->pclk4_apb4_hz / baud;
    *LPUART1_PRESC  =  (0b1011<<0); /* clock prescaler div. by 256 */
    *LPUART1_CR1 |=  (1<<0);                /* UE=1 */
}

void init_lpuart1_pb7_pb6(const clock_info_t *ci, unsigned baud)
{
    /* Clocks: GPIOB + LPUART1 */
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB4ENR |= (1<<LPUART1EN);

    /* PB6 (TX, AF8), PB7 (RX, AF8) */
    *GPIOB_MODER &= ~(0b11<<(6*2)) & ~(0b11<<(7*2));
    *GPIOB_MODER |=  (0b10<<(6*2)) |  (0b10<<(7*2));
    *GPIOB_AFRL &= ~(0b1111<<(6*4)) & ~(0b1111<<(7*4));
    *GPIOB_AFRL |=  (8<<(6*4))     |  (8<<(7*4));

    *LPUART1_CR1 &= ~(1<<0);
    *LPUART1_CR1 |=  (1<<3) | (1<<2);
    *LPUART1_BRR  =   ci->pclk4_apb4_hz / baud;
    *LPUART1_PRESC  =  (0b1011<<0);
    *LPUART1_CR1 |=  (1<<0);
}

/****************************** UART4 (APB1L) *********************************/

void init_uart4_pa1_pa0(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOAEN);
    *RCC_APB1LENR |= (1<<UART4EN);

    /* PA0 (TX, AF8), PA1 (RX, AF8) */
    *GPIOA_MODER &= ~(0b11<<(0*2)) & ~(0b11<<(1*2));
    *GPIOA_MODER |=  (0b10<<(0*2)) |  (0b10<<(1*2));
    *GPIOA_AFRL  &= ~(0b1111<<(0*4)) & ~(0b1111<<(1*4));
    *GPIOA_AFRL  |=  (8<<(0*4))      |  (8<<(1*4));

    *UART4_CR1 &= ~(1<<0);
    *UART4_CR1 |=  (1<<3) | (1<<2);
    *UART4_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART4_CR1 |=  (1<<0);
}

void init_uart4_pb8_pb9(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB1LENR |= (1<<UART4EN);

    /* PB9 (TX, AF8), PB8 (RX, AF8) */
    *GPIOB_MODER &= ~(0b11<<(9*2)) & ~(0b11<<(8*2));
    *GPIOB_MODER |=  (0b10<<(9*2)) |  (0b10<<(8*2));
    *GPIOB_AFRH  &= ~(0b1111<<((9-8)*4)) & ~(0b1111<<((8-8)*4));
    *GPIOB_AFRH  |=  (8<<((9-8)*4))      |  (8<<((8-8)*4));

    *UART4_CR1 &= ~(1<<0);
    *UART4_CR1 |=  (1<<3) | (1<<2);
    *UART4_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART4_CR1 |=  (1<<0);
}

void init_uart4_pc11_pc10(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOCEN);
    *RCC_APB1LENR |= (1<<UART4EN);

    /* PC10 (TX, AF8), PC11 (RX, AF8) */
    *GPIOC_MODER &= ~(0b11<<(10*2)) & ~(0b11<<(11*2));
    *GPIOC_MODER |=  (0b10<<(10*2)) |  (0b10<<(11*2));
    *GPIOC_AFRH  &= ~(0b1111<<((10-8)*4)) & ~(0b1111<<((11-8)*4));
    *GPIOC_AFRH  |=  (8<<((10-8)*4))      |  (8<<((11-8)*4));

    *UART4_CR1 &= ~(1<<0);
    *UART4_CR1 |=  (1<<3) | (1<<2);
    *UART4_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART4_CR1 |=  (1<<0);
}

void init_uart4_pd0_pd1(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIODEN);
    *RCC_APB1LENR |= (1<<UART4EN);

    /* PD1 (TX, AF8), PD0 (RX, AF8) */
    *GPIOD_MODER &= ~(0b11<<(1*2)) & ~(0b11<<(0*2));
    *GPIOD_MODER |=  (0b10<<(1*2)) |  (0b10<<(0*2));
    *GPIOD_AFRL  &= ~(0b1111<<(1*4)) & ~(0b1111<<(0*4));
    *GPIOD_AFRL  |=  (8<<(1*4))      |  (8<<(0*4));

    *UART4_CR1 &= ~(1<<0);
    *UART4_CR1 |=  (1<<3) | (1<<2);
    *UART4_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART4_CR1 |=  (1<<0);
}

/****************************** UART5 (APB1L) *********************************/

void init_uart5_pb12_pb13(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB1LENR |= (1<<UART5EN);

    /* PB13 (TX, AF14), PB12 (RX, AF14) */
    *GPIOB_MODER &= ~(0b11<<(13*2)) & ~(0b11<<(12*2));
    *GPIOB_MODER |=  (0b10<<(13*2)) |  (0b10<<(12*2));
    *GPIOB_AFRH  &= ~(0b1111<<((13-8)*4)) & ~(0b1111<<((12-8)*4));
    *GPIOB_AFRH  |=  (14<<((13-8)*4))     |  (14<<((12-8)*4));

    *UART5_CR1 &= ~(1<<0);
    *UART5_CR1 |=  (1<<3) | (1<<2);
    *UART5_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART5_CR1 |=  (1<<0);
}

void init_uart5_pb5_pb6(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB1LENR |= (1<<UART5EN);

    /* PB6 (TX, AF14), PB5 (RX, AF14) */
    *GPIOB_MODER &= ~(0b11<<(6*2)) & ~(0b11<<(5*2));
    *GPIOB_MODER |=  (0b10<<(6*2)) |  (0b10<<(5*2));
    *GPIOB_AFRL  &= ~(0b1111<<(6*4)) & ~(0b1111<<(5*4));
    *GPIOB_AFRL  |=  (14<<(6*4))     |  (14<<(5*4));

    *UART5_CR1 &= ~(1<<0);
    *UART5_CR1 |=  (1<<3) | (1<<2);
    *UART5_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART5_CR1 |=  (1<<0);
}

void init_uart5_pd2_pc12(const clock_info_t *ci, unsigned baud)
{
    /* Ports: GPIOD + GPIOC; Peripheral: UART5 on APB1L */
    *RCC_AHB4ENR  |= (1<<GPIODEN) | (1<<GPIOCEN);
    *RCC_APB1LENR |= (1<<UART5EN);
   // *RCC_C1_APB1LENR |= (1<<UART5EN);

    /* PC12 (TX, AF8) */
    *GPIOC_MODER &= ~(0b11<<(12*2));
    *GPIOC_MODER |=  (0b10<<(12*2));
    *GPIOC_AFRH  &= ~(0b1111<<((12-8)*4));
    *GPIOC_AFRH  |=  (8<<((12-8)*4));

    /* PD2 (RX, AF8) */
    *GPIOD_MODER &= ~(0b11<<(2*2));
    *GPIOD_MODER |=  (0b10<<(2*2));
    *GPIOD_AFRL  &= ~(0b1111<<(2*4));
    *GPIOD_AFRL  |=  (8<<(2*4));

    *UART5_CR1 &= ~(1<<0);
    *UART5_CR1 |=  (1<<3) | (1<<2);
    *UART5_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART5_CR1 |=  (1<<0);
}

/****************************** UART7 (APB1H) *********************************/

void init_uart7_pb3_pb4(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB1LENR |= (1<<UART7EN);

    /* PB4 (TX, AF11), PB3 (RX, AF11) */
    *GPIOB_MODER &= ~(0b11<<(4*2)) & ~(0b11<<(3*2));
    *GPIOB_MODER |=  (0b10<<(4*2)) |  (0b10<<(3*2));
    *GPIOB_AFRL  &= ~(0b1111<<(4*4)) & ~(0b1111<<(3*4));
    *GPIOB_AFRL  |=  (11<<(4*4))     |  (11<<(3*4));

    *UART7_CR1 &= ~(1<<0);
    *UART7_CR1 |=  (1<<3) | (1<<2);
    *UART7_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART7_CR1 |=  (1<<0);
}

void init_uart7_pe7_pe8(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOEEN);
    *RCC_APB1LENR |= (1<<UART7EN);

    /* PE8 (TX, AF7), PE7 (RX, AF7) */
    *GPIOE_MODER &= ~(0b11<<(8*2)) & ~(0b11<<(7*2));
    *GPIOE_MODER |=  (0b10<<(8*2)) |  (0b10<<(7*2));
    *GPIOE_AFRH  &= ~(0b1111<<((8-8)*4));   /* clear AFRH[3:0] for pin8 */
    *GPIOE_AFRL  &= ~(0b1111<<(7*4));       /* clear AFRL for pin7 */
    *GPIOE_AFRH  |=  (7<<((8-8)*4));        /* AF7 for pin8 */
    *GPIOE_AFRL  |=  (7<<(7*4));            /* AF7 for pin7 */

    *UART7_CR1 &= ~(1<<0);
    *UART7_CR1 |=  (1<<3) | (1<<2);
    *UART7_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART7_CR1 |=  (1<<0);
}

void init_uart7_pa8_pa15(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOAEN);
    *RCC_APB1LENR |= (1<<UART7EN);

    /* PA15 (TX, AF11), PA8 (RX, AF11) */
    *GPIOA_MODER &= ~(0b11<<(15*2)) & ~(0b11<<(8*2));
    *GPIOA_MODER |=  (0b10<<(15*2)) |  (0b10<<(8*2));
    *GPIOA_AFRH  &= ~(0b1111<<((15-8)*4)) & ~(0b1111<<((8-8)*4));
    *GPIOA_AFRH  |=  (11<<((15-8)*4))     |  (11<<((8-8)*4));

    *UART7_CR1 &= ~(1<<0);
    *UART7_CR1 |=  (1<<3) | (1<<2);
    *UART7_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART7_CR1 |=  (1<<0);
}

/****************************** UART8 (APB1H) *********************************/

void init_uart8_pe0_pe1(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOEEN);
    *RCC_APB1LENR |= (1<<UART8EN);

    /* PE1 (TX, AF8), PE0 (RX, AF8) */
    *GPIOE_MODER &= ~(0b11<<(1*2)) & ~(0b11<<(0*2));
    *GPIOE_MODER |=  (0b10<<(1*2)) |  (0b10<<(0*2));
    *GPIOE_AFRL  &= ~(0b1111<<(1*4)) & ~(0b1111<<(0*4));
    *GPIOE_AFRL  |=  (8<<(1*4))      |  (8<<(0*4));

    *UART8_CR1 &= ~(1<<0);
    *UART8_CR1 |=  (1<<3) | (1<<2);
    *UART8_BRR  =   ci->pclk1_apb1_hz / baud;
    *UART8_CR1 |=  (1<<0);
}

/****************************** USART1 (APB2) *********************************/

void init_usart1_pa10_pa9(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOAEN);
    *RCC_APB2ENR  |= (1<<USART1EN);

    /* PA9 (TX, AF7), PA10 (RX, AF7) */
    *GPIOA_MODER &= ~(0b11<<(9*2)) & ~(0b11<<(10*2));
    *GPIOA_MODER |=  (0b10<<(9*2)) |  (0b10<<(10*2));
    *GPIOA_AFRH  &= ~(0b1111<<((9-8)*4)) & ~(0b1111<<((10-8)*4));
    *GPIOA_AFRH  |=  (7<<((9-8)*4))      |  (7<<((10-8)*4));

    *USART1_CR1 &= ~(1<<0);
    *USART1_CR1 |=  (1<<3) | (1<<2);
    *USART1_BRR  =   ci->pclk2_apb2_hz / baud;
    *USART1_CR1 |=  (1<<0);
}

void init_usart1_pb15_pb14(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB2ENR  |= (1<<USART1EN);

    /* PB14 (TX, AF4), PB15 (RX, AF4) */
    *GPIOB_MODER &= ~(0b11<<(14*2)) & ~(0b11<<(15*2));
    *GPIOB_MODER |=  (0b10<<(14*2)) |  (0b10<<(15*2));
    *GPIOB_AFRH  &= ~(0b1111<<((14-8)*4)) & ~(0b1111<<((15-8)*4));
    *GPIOB_AFRH  |=  (4<<((14-8)*4))      |  (4<<((15-8)*4));

    *USART1_CR1 &= ~(1<<0);
    *USART1_CR1 |=  (1<<3) | (1<<2);
    *USART1_BRR  =   ci->pclk2_apb2_hz / baud;
    *USART1_CR1 |=  (1<<0);
}

void init_usart1_pb7_pb6(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB2ENR  |= (1<<USART1EN);

    /* PB6 (TX, AF7), PB7 (RX, AF7) */
    *GPIOB_MODER &= ~(0b11<<(6*2)) & ~(0b11<<(7*2));
    *GPIOB_MODER |=  (0b10<<(6*2)) |  (0b10<<(7*2));
    *GPIOB_AFRL  &= ~(0b1111<<(6*4)) & ~(0b1111<<(7*4));
    *GPIOB_AFRL  |=  (7<<(6*4))      |  (7<<(7*4));

    *USART1_CR1 &= ~(1<<0);
    *USART1_CR1 |=  (1<<3) | (1<<2);
    *USART1_BRR  =   ci->pclk2_apb2_hz / baud;
    *USART1_CR1 |=  (1<<0);
}

/****************************** USART2 (APB1L) ********************************/

void init_usart2_pa3_pa2(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOAEN);
    *RCC_APB1LENR |= (1<<USART2EN);

    /* PA2 (TX, AF7), PA3 (RX, AF7) */
    *GPIOA_MODER &= ~(0b11<<(2*2)) & ~(0b11<<(3*2));
    *GPIOA_MODER |=  (0b10<<(2*2)) |  (0b10<<(3*2));
    *GPIOA_AFRL  &= ~(0b1111<<(2*4)) & ~(0b1111<<(3*4));
    *GPIOA_AFRL  |=  (7<<(2*4))      |  (7<<(3*4));

    *USART2_CR1 &= ~(1<<0);
    *USART2_CR1 |=  (1<<3) | (1<<2);
    *USART2_BRR  =   ci->pclk1_apb1_hz / baud;
    *USART2_CR1 |=  (1<<0);
}

void init_usart2_pd6_pd5(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIODEN);
    *RCC_APB1LENR |= (1<<USART2EN);

    /* PD5 (TX, AF7), PD6 (RX, AF7) */
    *GPIOD_MODER &= ~(0b11<<(5*2)) & ~(0b11<<(6*2));
    *GPIOD_MODER |=  (0b10<<(5*2)) |  (0b10<<(6*2));
    *GPIOD_AFRL  &= ~(0b1111<<(5*4)) & ~(0b1111<<(6*4));
    *GPIOD_AFRL  |=  (7<<(5*4))      |  (7<<(6*4));

    *USART2_CR1 &= ~(1<<0);
    *USART2_CR1 |=  (1<<3) | (1<<2);
    *USART2_BRR  =   ci->pclk1_apb1_hz / baud;
    *USART2_CR1 |=  (1<<0);
}

/****************************** USART3 (APB1L) ********************************/

void init_usart3_pb11_pb10(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOBEN);
    *RCC_APB1LENR |= (1<<USART3EN);

    /* PB10 (TX, AF7), PB11 (RX, AF7) */
    *GPIOB_MODER &= ~(0b11<<(10*2)) & ~(0b11<<(11*2));
    *GPIOB_MODER |=  (0b10<<(10*2)) |  (0b10<<(11*2));
    *GPIOB_AFRH  &= ~(0b1111<<((10-8)*4)) & ~(0b1111<<((11-8)*4));
    *GPIOB_AFRH  |=  (7<<((10-8)*4))      |  (7<<((11-8)*4));

    *USART3_CR1 &= ~(1<<0);
    *USART3_CR1 |=  (1<<3) | (1<<2);
    *USART3_BRR  =   ci->pclk1_apb1_hz / baud;
    *USART3_CR1 |=  (1<<0);
}

void init_usart3_pc11_pc10(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOCEN);
    *RCC_APB1LENR |= (1<<USART3EN);

    /* PC10 (TX, AF7), PC11 (RX, AF7) */
    *GPIOC_MODER &= ~(0b11<<(10*2)) & ~(0b11<<(11*2));
    *GPIOC_MODER |=  (0b10<<(10*2)) |  (0b10<<(11*2));
    *GPIOC_AFRH  &= ~(0b1111<<((10-8)*4)) & ~(0b1111<<((11-8)*4));
    *GPIOC_AFRH  |=  (7<<((10-8)*4))      |  (7<<((11-8)*4));

    *USART3_CR1 &= ~(1<<0);
    *USART3_CR1 |=  (1<<3) | (1<<2);
    *USART3_BRR  =   ci->pclk1_apb1_hz / baud;
    *USART3_CR1 |=  (1<<0);
}

void init_usart3_pd9_pd8(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIODEN);
    *RCC_APB1LENR |= (1<<USART3EN);

    /* PD8 (TX, AF7), PD9 (RX, AF7) */
    *GPIOD_MODER &= ~(0b11<<(8*2)) & ~(0b11<<(9*2));
    *GPIOD_MODER |=  (0b10<<(8*2)) |  (0b10<<(9*2));
    *GPIOD_AFRH  &= ~(0b1111<<((8-8)*4)) & ~(0b1111<<((9-8)*4));
    *GPIOD_AFRH  |=  (7<<((8-8)*4))      |  (7<<((9-8)*4));

    *USART3_CR1 &= ~(1<<0);
    *USART3_CR1 |=  (1<<3) | (1<<2);
    *USART3_BRR  =   ci->pclk1_apb1_hz / baud;
    *USART3_CR1 |=  (1<<0);
}

/****************************** USART6 (APB2) *********************************/

void init_usart6_pc7_pc6(const clock_info_t *ci, unsigned baud)
{
    *RCC_AHB4ENR  |= (1<<GPIOCEN);
    *RCC_APB2ENR  |= (1<<USART6EN);

    /* PC6 (TX, AF7), PC7 (RX, AF7) */
    *GPIOC_MODER &= ~(0b11<<(6*2)) & ~(0b11<<(7*2));
    *GPIOC_MODER |=  (0b10<<(6*2)) |  (0b10<<(7*2));
    *GPIOC_AFRL  &= ~(0b1111<<(6*4)) & ~(0b1111<<(7*4));
    *GPIOC_AFRL  |=  (7<<(6*4))      |  (7<<(7*4));

    *USART6_CR1 &= ~(1<<0);
    *USART6_CR1 |=  (1<<3) | (1<<2);
    *USART6_BRR  =   ci->pclk2_apb2_hz / baud;
    *USART6_CR1 |=  (1<<0);
}


/**** write character *********************************************************/

#define TXE 7
#define RXNE 5

void lpuart1_write_char(int c) {
    /* Wait until TXE transmit data register empty */
    while ( !(*LPUART1_ISR & (1<<TXE)) ){}
    /* Load data into TDR */
    *LPUART1_TDR = c;
}

void uart4_write_char(int c) {
    while ( !(*UART4_ISR & (1<<TXE)) ){}
    *UART4_TDR = c;
}

void uart5_write_char(int c) {
    while ( !(*UART5_ISR & (1<<TXE)) ){}
    *UART5_TDR = c;
}

void uart7_write_char(int c) {
    while ( !(*UART7_ISR & (1<<TXE)) ){}
    *UART7_TDR = c;
}

void uart8_write_char(int c) {
    while ( !(*UART8_ISR & (1<<TXE)) ){}
    *UART8_TDR = c;
}

void usart1_write_char(int c) {
    while ( !(*USART1_ISR & (1<<TXE)) ){}
    *USART1_TDR = c;
}

void usart2_write_char(int c) {
    while ( !(*USART2_ISR & (1<<TXE)) ){}
    *USART2_TDR = c;
}

void usart3_write_char(int c) {
    while ( !(*USART3_ISR & (1<<TXE)) ){}
    *USART3_TDR = c;
}

void usart6_write_char(int c) {
    while ( !(*USART6_ISR & (1<<TXE)) ){}
    *USART6_TDR = c;
}

/**** read character **********************************************************/

int lpuart1_read_char(void) {
    /* Wait until RXNE read data register not empty */
    while ( !(*LPUART1_ISR & (1 << RXNE)) ){}
    // Return the received character
    return *LPUART1_RDR & 0xFF;
}

int uart4_read_char(void) {
    while ( !(*UART4_ISR & (1 << RXNE)) ){}
    return *UART4_RDR & 0xFF;
}

int uart5_read_char(void) {
    while ( !(*UART5_ISR & (1 << RXNE)) ){}
    return *UART5_RDR & 0xFF;
}

int uart7_read_char(void) {
    while ( !(*UART7_ISR & (1 << RXNE)) ){}
    return *UART7_RDR & 0xFF;
}

int uart8_read_char(void) {
    while ( !(*UART8_ISR & (1 << RXNE)) ){}
    return *UART8_RDR & 0xFF;
}

int usart1_read_char(void) {
    while ( !(*USART1_ISR & (1 << RXNE)) ){}
    return *USART1_RDR & 0xFF;
}

int usart2_read_char(void) {
    while ( !(*USART2_ISR & (1 << RXNE)) ){}
    return *USART2_RDR & 0xFF;
}

int usart3_read_char(void) {
    while ( !(*USART3_ISR & (1 << RXNE)) ){}
    return *USART3_RDR & 0xFF;
}

int usart6_read_char(void) {
    while ( !(*USART6_ISR & (1 << RXNE)) ){}
    return *USART6_RDR & 0xFF;
}

