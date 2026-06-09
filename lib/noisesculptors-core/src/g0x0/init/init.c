#include "rcc.h"
#include "flash.h"
#include "init.h"
#include "pwr.h"
#include "timer.h"
#include <stdint.h>

clock_info_t ci;

/* --- PLLSRC encoding on STM32G0 (RCC_PLLCFGR[1:0]) --- 
   00: no clock, 01: HSI16, 10: HSE, 11: reserved (see RM0444) */
#define PLLSRC_HSI16 2u
#define PLLSRC_HSE   3u

static void pll_start(unsigned src, unsigned M, unsigned N, unsigned R)
{
    /* 1) Make sure HSI16 is ON */
    *RCC_CR |=  (1u << 8);                 // HSION
    while (!(*RCC_CR & (1u << 10))) {}     // HSIRDY

    /* 2) Disable PLL to reconfigure */
    *RCC_CR &= ~(1u << 24);                // PLLON=0
    while ( (*RCC_CR & (1u << 25)) ) {}    // wait PLLRDY==0
    
    *RCC_PLLCFGR = 0x00001000; /* == Reset value */

    unsigned cf = *RCC_PLLCFGR;

    cf |= (src & 3u) << 0;                 // PLLSRC
    /* M is encoded as (M-1) on G0 */
    cf |= (((M - 1u) & 0x7u) << 4);        // PLLM
    cf |= ((N & 0x7Fu) << 8);              // PLLN
    /* R divisor encode: /2->0, /4->1, /6->2, /8->3 (others reserved) */
    unsigned renc = (R==2u)?0u : (R==4u)?1u : (R==6u)?2u : 3u;
//  cf |= (((P - 1u) & 0x1Fu) << 17);      // PLLP
//  cf |= (((Q - 1u) & 0x7u) << 25);       // PLLQ
    cf |= ((renc & 0x7u) << 29);           // PLLR
    cf |= (1u << 28);                      // PLLREN

    *RCC_PLLCFGR = cf;

    /* 4) Enable PLL and wait ready */
    *RCC_CR |=  (1u << 24);                // PLLON
    while (!(*RCC_CR & (1u << 25))) {}     // PLLRDY
}

clock_info_t init_sysclk_pll(
        uint32_t src, uint32_t M, uint32_t N, uint32_t R, uint32_t osc_hz)
{
    /* PWR Range 1 (for 64 MHz) */
    *RCC_APBENR1 |= (1u << 28);
    unsigned pwr = *PWR_CR1;

    *RCC_CR |=  (1u << 8);                 // HSION
    while (!(*RCC_CR & (1u << 10))) {}     // HSIRDY

    unsigned SW;
    if(src == PLLSRC_HSI16){
        SW = 0u;
        *RCC_CFGR &= ~(0x7u << 0);
    } else { // HSE
        SW = 1u;
        *RCC_CFGR &= ~(0x7u << 0);
        *RCC_CFGR |= (SW << 0);
    }
    while (((*RCC_CFGR >> 3) & 0x7u) != SW) {}
   
    *PWR_CR1 |= (1u<<9u);

    /* Prescalers: AHB=/1, APB=/1 on G0 -> HPRE[11:8], PPRE[14:12] */
    *RCC_CFGR = (*RCC_CFGR &
                ~((0xFu << 8)   /* HPRE */
                 |(0x7u << 12)))/* PPRE */
              |  (0u << 8)      /* HPRE /1 */
              |  (0u << 12);    /* PPRE /1 */

    /* Start PLL */
   //  pll_start(src, M, N, R);

    /* 2) Disable PLL to reconfigure */
    *RCC_CR &= ~(1u << 24);    // PLLON=0
    *RCC_PLLCFGR = 0x00001000; // reset value
    while ( (*RCC_CR & (1u << 25)) ) {}    // wait PLLRDY==0

    unsigned cf = 0;
    cf |= (src & 3u) << 0                  // PLLSRC
       |  (((M - 1u) & 0x7u) << 4)         // PLLM
       |  ((N & 0x7Fu) << 8)               // PLLN
       |  (1u << 28u) // PLLREN
       |  (1u << 16u);// PLLPEN
    /* R divisor encode: /2->0, /4->1, /6->2, /8->3 (others reserved) */
    unsigned renc = (R==2u)?0u : (R==4u)?1u : (R==6u)?2u : 3u;

    *RCC_PLLCFGR = cf | 0x00001000; /* 0x1000 == reset value */

    *RCC_CR |=  (1u << 24);                // PLLON
    while (!(*RCC_CR & (1u << 25))) {}     // PLLRDY

    /* Compute clocks (integer mode) */
    uint32_t fref   = (osc_hz / M);
    uint32_t fvco   = fref * N;
    uint32_t fpll_r = (R ? (fvco / R) : 0u);

    /* FLASH latency: Range1 → 0WS≤24 MHz, 1WS≤48 MHz, 2WS≤64 MHz */
    unsigned ws = (fpll_r <= 24000000u) ? 0u
                 : (fpll_r <= 48000000u) ? 1u : 2u;
    *FLASH_ACR = (*FLASH_ACR & ~(7u << 0)) | (ws << 0);

    /* Switch SYSCLK to PLLR (CFGR.SW[2:0], wait SWS[2:0]) */
    SW = 0b10u; // PLLRCLK
    *RCC_CFGR &= (0x7u << 0) & 0x7u;
    *RCC_CFGR = (SW << 0);
    while (((*RCC_CFGR >> 3) & 0x7u) != SW) {}

    /* Fill clock_info */
    ci.clk_hz           = 0u;
    ci.pll_vco_hz       = fvco;
    ci.pll_r_hz         = fpll_r;
    ci.sysclk_hz        = fpll_r;
    ci.hclk_hz          = fpll_r;
    ci.pclk1_apb1_hz    = fpll_r;
    ci.tim_apb1_hz      = fpll_r;
    return ci;
}

/* 64 MHz from HSI16: M=2, N=16, R=2 */
clock_info_t init_clock(void)
{
    return init_sysclk_pll(PLLSRC_HSI16, 2u, 16u, 2u, 16000000u);
}

