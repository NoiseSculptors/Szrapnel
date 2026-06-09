
#include "dwt.h"
#include "delay.h"
#include "init.h"
#include <stdint.h>


extern clock_info_t ci;

void init_dwt(void)
{
    delay_ms(10); /* without this delay, when using dwt directly after
                     init_clock(), does not work as intended
    *DWT_LAR = 0xC5ACCE55; and magic key to unlock DWT does not help */

    *DEMCR |= (1u << 24);  /* enable trace and debug block */
    *DWT_CYCCNT = 0;
    *DWT_CTRL  |= 1u;      /* enable cycle counter */
}

void dwt_start(void)
{
    *DWT_CYCCNT = 0;       /* reset counter */
}

uint32_t dwt_now(void)
{
    return *DWT_CYCCNT;
}

double dwt_cycles_to_s(uint32_t cycles)
{
    return (double)cycles/(double)ci.sysclk_hz;
}

double dwt_cycles_to_ms(uint32_t cycles)
{
    return (double)cycles/((double)ci.sysclk_hz/(double)1000.0);
}

double dwt_cycles_to_us(uint32_t cycles)
{
    return (double)cycles/((double)ci.sysclk_hz/(double)1000000.0);
}

