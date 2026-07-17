
#include "rcc.h"
#include "gpio.h"
#include "timer.h"
#include <stdint.h>

void io_hc_sr04_init(void)
{
    *RCC_AHB4ENR |= (1u<<GPIOBEN)|(1u<<GPIOEEN);

    *RCC_APB2ENR |= (1<<17)| // TIM16EN
                    (1<<0);  // TIM1EN

    /* PE11 AF1:TIM1_CH2
       PB8  AF1:TIM16_CH1 */
    gpio_ctrl(GPIOB, GPIO_MODE, GPIO8,  MODE_AF);
    gpio_ctrl(GPIOE, GPIO_MODE, GPIO11, MODE_AF);
    gpio_ctrl(GPIOB, GPIO_AFRH, GPIO8,  AF1);
    gpio_ctrl(GPIOE, GPIO_AFRH, GPIO11, AF1);

    /* PB8 (TIM16_CH1) periodically transmits 10u
       signal that triggers distance measurment */
    *TIM16_PSC   = 479;
    *TIM16_ARR   = 15000;
    *TIM16_CCR1  = 5;
    *TIM16_CCMR1 = (6<<4);
    *TIM16_CCER  = (1<<0);
    *TIM16_BDTR  = (1<<15);
    *TIM16_CR1   = (1<<0);

    *TIM1_PSC   = 479;
    *TIM1_ARR   = 0xffff;
    *TIM1_CCMR1 = (1<<8)|(2<<0);
    *TIM1_SMCR  = (6<<4)|(4<<0);
    *TIM1_CCER  = (1<<4)|(3<<0);
    *TIM1_CR1   = 1;

}


uint32_t io_hc_sr04_get_raw(void)
{
    static uint32_t distance;

  //if (*TIM1_SR & (1U << 2)) {
    distance = *TIM1_CCR1;
    if(distance > 5900)
        distance = 5900;
    if(distance < 100)
        distance = 100;
  //*TIM1_SR &= ~(1U << 2);
  //}
    return distance;
}

float io_hc_sr04_get_cm(void)
{
    return (float)io_hc_sr04_get_raw() * 0.0343f; // us -> cm
}

