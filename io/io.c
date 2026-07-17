
#include "delay.h"
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "i2c.h"
#include "nvic.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "rng.h"
#include "spi.h"
#include "serial.h"
#include "syscall.h"
#include "systick.h"
#include "timer.h"
#include "io.h"
#include <string.h>

extern uint32_t leds;
extern uint32_t lookup_moder[];
extern uint32_t lookup_bsrr[];

__attribute__((section(".itcm"),used))
void SysTick_Handler(void)
{

/*
__asm volatile("ldr   r3, =0xE000EF50");
__asm volatile("movs  r4, #0");
__asm volatile("str   r4, [r3]");
__asm volatile("dsb   sy");
__asm volatile("isb");
*/

    user_systick();

/*
  111111
  5432109876543210
e 1000000001111000
d 1110000011000000
c 1110011111110000
b 0000110000011011
a 1000011100000000

a8  | b0  |  c4  |  d6  | e3             
a9  | b1  |  c5  |  d7  | e4
a10 | b3  |  c6  |  d13 | e5
a15 | b4  |  c7  |  d14 | e6
      b10 |  c8  |  d15 | e15
      b11 |  c9  |
     if (x > LIM_09) x = LIM_09;
if (x < -LIM_09) x = -LIM_09;        c10 |
             c13 |
             c14 |
             c15 |
*/

    /* draw a frame on LCD, no synchronization */
#if 0
    static uint32_t dma_ctr;
    static uint8_t cmd[1] __attribute__((aligned(32))) = {0x2c}; // ramwr

    if(dma_ctr++ == 0){
        *GPIOD_BSRR = 0x2000000; // CS low - sending command
        lcd_dma_send(cmd, 1);    // sending one byte - ramwr
    } else if (dma_ctr == 2){
        *GPIOD_BSRR = 0x200;     // CS high - sending data
        lcd_dma_send((uint8_t*)fb, 25600);
    } else if (dma_ctr == 5){   /* 60 is very conservative
                                   5 still usable@ca 125MHz spi clock */
        dma_ctr = 0;
    }
#endif

    /* this part is for LEDs above keys */
    static uint32_t n;
    if (leds & (1U<<n)) {
        *GPIOD_MODER = lookup_moder[n];
        *GPIOD_BSRR = lookup_bsrr[n];
    } else {
        *GPIOD_BSRR = 0x3F0000;
    }
    if (++n >= 30) n = 0;
}

void io_init(void) {
    init_clock();
    init_rng();
    io_buttons_init();
    io_encoders_init();
    io_serial_init();
    /* io_dac_init() is started with audio_config */
    io_lcd_init();
}

