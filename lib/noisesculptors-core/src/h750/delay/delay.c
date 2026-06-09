
#include "delay.h"
#include "init.h"
#include "timer.h"
#include <stdint.h>

extern const clock_info_t ci;

/* use delay_ms instead of delay_us
   for values over 1000us as it is more accurate */

void delay_ms(uint32_t ms)
{
    volatile uint32_t ctr = ms * 10;
    volatile uint32_t ref = (ci.sysclk_hz / 2 / 10000)-22;
    *TIM6_PSC = 0;
    *TIM6_CR1 = 1;
    *TIM6_CNT = 0;
    while(ctr--){
        *TIM6_CNT = 0;
        while(*TIM6_CNT < ref){}
    }
    *TIM6_CR1 = 0;
}

void delay_us(uint32_t us)
{
    volatile uint32_t ctr = us;
    volatile uint32_t ref = (ci.sysclk_hz / 2 / 1000000)-21;
    *TIM6_PSC = 0;
    *TIM6_CR1 = 1;
    *TIM6_CNT = 0;
    while(ctr--){
        *TIM6_CNT = 0;
        while(*TIM6_CNT < ref){}
    }
    *TIM6_CR1 = 0;
}

