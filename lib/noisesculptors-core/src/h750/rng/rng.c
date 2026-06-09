
#include "rng.h"
#include "rcc.h"

void init_rng(void)
{

#define IE 3

#define RNGSEL 8
    *RCC_D2CCIP2R &= ~(0x3u<<RNGSEL);
    *RCC_D2CCIP2R |= (0x1u<<RNGSEL); /* 1: pll1_q_ck as periph clock */
    *RCC_AHB2ENR &= ~(0x3u<<6); /* RNGEN */
    *RCC_AHB2ENR |= (0x1u<<6);
#define RNGEN 2
    *RNG_CR |= (1<<RNGEN) | (1<<IE);

}

unsigned rng_rnd(void)
{
#define DRDY 0
/* not needed for synth, but, RM0433 Rev8 p.1261/3353
"When DRDY is set, it is recommended to always verify that RNG_DR is different
from zero. Because when it is the case a seed error occurred between RNG_SR
polling and RND_DR output reading (rare event)."
*/
    /* wait until random data ready */
    while(!(*RNG_SR&(1<<DRDY))){}
    return *RNG_DR;
}

