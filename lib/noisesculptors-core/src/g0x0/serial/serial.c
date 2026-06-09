
#include "rcc.h"
#include "gpio.h"
#include "serial.h"
#include "usart_uart.h"
#include <stdint.h>

/* ===== Clocks (RCC) — STM32G0 =====
   RCC_IOPENR:  GPIO port clocks
     bit0=GPIOAEN, bit1=GPIOBEN, bit2=GPIOCEN, bit6=GPIOFEN
   RCC_APBENR2: USART1EN bit[14]
   RCC_APBENR1: USART2EN bit[17]
*/
#define USART1EN  14u  /* RCC_APBENR2 */
#define USART2EN  17u  /* RCC_APBENR1 */

/* ===== USART bits ===== */
#define UE   0u  /* CR1: USART enable */
#define RE   2u  /* CR1: Receiver enable */
#define TE   3u  /* CR1: Transmitter enable */
#define RXNE 5u  /* ISR: RX not empty */
#define TXE  7u  /* ISR: TX empty */

/* ===== Helpers to set AF without branching ===== */
static inline void gpio_to_af(volatile unsigned *MODER, volatile unsigned *AFx,
                              unsigned pin, unsigned af, int high_reg)
{
    /* MODER[2*pin+1:2*pin] = 0b10 (Alternate) */
    *MODER = (*MODER & ~(0x3u << (pin*2))) | (0x2u << (pin*2));
    /* AFR: choose AFRL or AFRH */
    if (high_reg) {
        unsigned shift = (pin - 8u) * 4u;
        *AFx = (*AFx & ~(0xFu << shift)) | ((af & 0xFu) << shift);
    } else {
        unsigned shift = pin * 4u;
        *AFx = (*AFx & ~(0xFu << shift)) | ((af & 0xFu) << shift);
    }
}

/* ====================== USART1 mappings ====================== */
/* PA9 (TX), PA10 (RX) AF1 */
void init_usart1_pa10_pa9(const clock_info_t *ci, unsigned baud)
{
    /* Clocks */
    *RCC_IOPENR   |= (1u << GPIOAEN);
    *RCC_APBENR2  |= (1u << USART1EN);

    /* PA9 -> AF1, PA10 -> AF1 */
    gpio_to_af(GPIOA_MODER, GPIOA_AFRH, 9u, 1u, 1);
    gpio_to_af(GPIOA_MODER, GPIOA_AFRH,10u, 1u, 1);

    /* USART1 basic 8N1 @ oversampling by 16 */
    *USART1_CR1 &= ~(1u << UE);
    *USART1_CR1 |=  (1u << TE) | (1u << RE);
    *USART1_BRR  =  ci->pclk1_apb1_hz / baud;   /* APB2 clock for USART1 */
    *USART1_CR1 |=  (1u << UE);
}

/* PB6 (TX), PB7 (RX) AF0 */
void init_usart1_pb7_pb6(const clock_info_t *ci, unsigned baud)
{
    *RCC_IOPENR  |= (1u << GPIOBEN);
    *RCC_APBENR2 |= (1u << USART1EN);

    /* PB6 -> AF0, PB7 -> AF0 */
    gpio_to_af(GPIOB_MODER, GPIOB_AFRL, 6u, 0u, 0);
    gpio_to_af(GPIOB_MODER, GPIOB_AFRL, 7u, 0u, 0);

    *USART1_CR1 &= ~(1u << UE);
    *USART1_CR1 |=  (1u << TE) | (1u << RE);
    *USART1_BRR  =  ci->pclk1_apb1_hz / baud;
    *USART1_CR1 |=  (1u << UE);
}

/* IO: write/read on USART1 */
void usart1_write_char(int c)
{
    while (!(*USART1_ISR & (1u << TXE))) {}
    *USART1_TDR = (uint32_t)(uint8_t)c;
}
int usart1_read_char(void)
{
    while (!(*USART1_ISR & (1u << RXNE))) {}
    return (int)(*USART1_RDR & 0xFFu);
}

/* ====================== USART2 mappings ====================== */
/* PA2 (TX), PA3 (RX) AF1 */
void init_usart2_pa3_pa2(const clock_info_t *ci, unsigned baud)
{
    *RCC_IOPENR  |= (1u << GPIOAEN);
    *RCC_APBENR1 |= (1u << USART2EN);

    /* PA2 -> AF1, PA3 -> AF1 */
    gpio_to_af(GPIOA_MODER, GPIOA_AFRL, 2u, 1u, 0);
    gpio_to_af(GPIOA_MODER, GPIOA_AFRL, 3u, 1u, 0);

    *USART2_CR1 &= ~(1u << UE);
    *USART2_CR1 |=  (1u << TE) | (1u << RE);
    *USART2_BRR  =  ci->pclk1_apb1_hz / baud;   /* APB1 clock for USART2 */
    *USART2_CR1 |=  (1u << UE);
}

/* PA14 (RX), PA15 (TX) AF1 */
void init_usart2_pa15_pa14(const clock_info_t *ci, unsigned baud)
{
    *RCC_IOPENR  |= (1u << GPIOAEN);
    *RCC_APBENR1 |= (1u << USART2EN);

    /* PA15 -> AF1 (TX), PA14 -> AF1 (RX) */
    gpio_to_af(GPIOA_MODER, GPIOA_AFRH, 15u, 1u, 1);
    gpio_to_af(GPIOA_MODER, GPIOA_AFRH, 14u, 1u, 1);

    *USART2_CR1 &= ~(1u << UE);
    *USART2_CR1 |=  (1u << TE) | (1u << RE);
    *USART2_BRR  =  ci->pclk1_apb1_hz / baud;
    *USART2_CR1 |=  (1u << UE);
}

void usart2_write_char(int c)
{
    while (!(*USART2_ISR & (1u << TXE))) {}
    *USART2_TDR = (uint32_t)(uint8_t)c;
}
int usart2_read_char(void)
{
    while (!(*USART2_ISR & (1u << RXNE))) {}
    return (int)(*USART2_RDR & 0xFFu);
}

/* ====================== Unified facade ====================== */

void init_serial(const clock_info_t *ci, unsigned baud)
{
#if defined(USART1_PA10_RX_PA9_TX)
    init_usart1_pa10_pa9(ci, baud);
#elif defined(USART1_PB7_RX_PB6_TX)
    init_usart1_pb7_pb6(ci, baud);
#elif defined(USART2_PA3_RX_PA2_TX)
    init_usart2_pa3_pa2(ci, baud);
#elif defined(USART2_PA15_RX_PA14_TX)
    init_usart2_pa15_pa14(ci, baud);
#else
# error "Pick one USART pin mapping in serial_g0.h"
#endif
}

void serial_write_char(int c)
{
#if defined(USART1_PA10_RX_PA9_TX) || defined(USART1_PB7_RX_PB6_TX)
    usart1_write_char(c);
#elif defined(USART2_PA3_RX_PA2_TX) || defined(USART2_PA15_RX_PA14_TX)
    usart2_write_char(c);
#endif
}

int serial_read_char(void)
{
#if defined(USART1_PA10_RX_PA9_TX) || defined(USART1_PB7_RX_PB6_TX)
    return usart1_read_char();
#elif defined(USART2_PA3_RX_PA2_TX) || defined(USART2_PA15_RX_PA14_TX)
    return usart2_read_char();
#endif
}

