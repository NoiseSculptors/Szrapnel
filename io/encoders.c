
// io_io.c (strong overrides)
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

void io_encoders_init(void) {
/********** Encoders **********************************************************/
    *RCC_AHB4ENR |= (1u<<GPIOAEN);

#define TIM2EN  0
    *RCC_APB1LENR |= (1u<<TIM2EN);

    gpio_ctrl(GPIOA, GPIO_MODE, GPIO0|GPIO1, MODE_AF);
    gpio_ctrl(GPIOA, GPIO_MODE, GPIO2|GPIO3, MODE_INPUT);
    gpio_ctrl(GPIOA, GPIO_AFRL, GPIO0|GPIO1, AF1); // TIM2 CH1 CH2

#define CC2E 4
#define CC1E 0
/* CC1E: Capture/Compare 1 output enable.
    0: Capture mode disabled / OC1 is not active
    1: Capture mode enabled / OC1 signal is output on the corresponding output pin */
    *TIM2_CCER &= ~((1<<CC2E)|(1<<CC1E));
    *TIM15_CCER &= ~((1<<CC2E)|(1<<CC1E));

#define SMS 0
#define CC2S 8
#define CC1S 0 
    /*
       TIM2 & TIM3 in new revision - TIM15 has unfortunately no encoder hw so
       software solution is used before new revision with TIM3 is manufactured
     */
    *TIM2_SMCR |= (3u<<SMS);  // Encoder mode 3: Counts on both TI1 and TI2
    *TIM2_CCMR1 &= ~0xFFFF;
    // Set channels as input mapped to TI1 and TI2
    *TIM2_CCMR1 |= (2u<<CC2S) | (2u<<CC1S);
    *TIM2_ARR = 0xFFFF;

    /* Enabe timers */
#define CEN 0
    *TIM2_CR1 |= (1u<<CEN);
    *TIM2_CNT = 1;
}

io_enc_t io_enc_read(size_t idx)
{
    static int16_t last0, last1;
    int16_t now, d16;

    switch(idx){
        case 0:
            now = (int16_t)(*GPIOA_IDR & 0xc)>>2;
            switch(now){
                case 0:
                    if(last0==1) d16++;
                    if(last0==2) d16--;
                    break;
                case 1: 
                    if(last0==3) d16++;
                    if(last0==0) d16--;
                    break;
                case 2:
                    if(last0==0) d16++;
                    if(last0==3) d16--;
                    break;
                case 3:
                    if(last0==2) d16++;
                    if(last0==1) d16--;
                    break;
            }
           //  rising:  02310
           //  falling: 01320
            last0 = now;
             break;
        case 1:
            now = (int16_t)*TIM2_CNT;
            d16 = (int16_t)(now - last1);
            last1 = now;
            break;
        default:
            now = 0; d16 = 0;
            break;
    }

    return (io_enc_t){ .value = now, .delta = d16, .pushed = 0 };
}

