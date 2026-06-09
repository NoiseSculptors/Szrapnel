
#include "rcc.h"
#include "pwr.h"
#include "flash.h"
#include "syscfg.h"
#include "init.h"
#include "timer.h"
#include "vtor.h"
#include <stdint.h>

#define PLLSRC      0
#define DIVM1       4
#define DIVM2       12
#define DIVM3       20

#define PLL1FRACEN  0
#define PLL1VCOSEL  1
#define PLL1RGE     2

#define PLL2FRACEN  4
#define PLL2VCOSEL  5
#define PLL2RGE     6

#define PLL3FRACEN  8
#define PLL3VCOSEL  9
#define PLL3RGE     10

#define DIVP1EN     16
#define DIVQ1EN     17
#define DIVR1EN     18
#define DIVP2EN     19
#define DIVQ2EN     20
#define DIVR2EN     21
#define DIVP3EN     22
#define DIVQ3EN     23
#define DIVR3EN     24

#define PLL1ON      24
#define PLL1RDY     25
#define PLL2ON      26
#define PLL2RDY     27
#define PLL3ON      28
#define PLL3RDY     29


clock_info_t ci;

static void pll_start_min(volatile uint32_t *DIVR, volatile uint32_t *FRACR,
        uint32_t on_bit, uint32_t rdy_bit, uint32_t rge_bit,
        uint32_t vcosel_bit, uint32_t fracen_bit, uint32_t divpen,
        uint32_t divqen, uint32_t divren, uint32_t divm_bit, uint32_t src,
        uint32_t M, uint32_t Nmul, uint32_t FRACN, uint32_t P, uint32_t Q,
        uint32_t R, uint32_t fref_after_M)
{
    *RCC_CR &= ~(1u<<on_bit);

    uint32_t v = *RCC_PLLCKSELR;
    v &= ~((0x3u<<PLLSRC) | (0x3Fu<<divm_bit));
    v |=  ((src&3u)<<PLLSRC) | ((M&0x3Fu)<<divm_bit);
    *RCC_PLLCKSELR = v;

    uint32_t cf = *RCC_PLLCFGR;

    cf &= ~((0x3u<<rge_bit)|(1u<<vcosel_bit)|(1u<<fracen_bit)|
            (1u<<divpen)|(1u<<divqen)|(1u<<divren));

    cf |= (((fref_after_M<2000000u)?
                0u:(fref_after_M<4000000u)?
                    1u:(fref_after_M<8000000u)?
                        2u:3u)<<rge_bit);

    if (P) cf |= (1u<<divpen);
    if (Q) cf |= (1u<<divqen);
    if (R) cf |= (1u<<divren);

    *DIVR = (((Nmul-1u)&0x1FFu)
          | (((P?P-1u:0u)&0x7Fu)<<9)
          | (((Q?Q-1u:0u)&0x7Fu)<<16)
          | (((R?R-1u:0u)&0x7Fu)<<24));

    if (FRACN) {
        *FRACR=((FRACN&0x1FFFu)<<3);
        cf|=(1u<<fracen_bit);
    } else {
        *FRACR=0u;
    }

    *RCC_PLLCFGR = cf;

    *RCC_CR |= (1u<<on_bit);

    while (!(*RCC_CR & (1u<<rdy_bit))) {}
}

void pll_1_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M)
{
    pll_start_min(RCC_PLL1DIVR, RCC_PLL1FRACR,
              PLL1ON, PLL1RDY, PLL1RGE, PLL1VCOSEL, PLL1FRACEN,
              DIVP1EN, DIVQ1EN, DIVR1EN, DIVM1,
              src, M, Nmul, FRACN, P, Q, R, fref_after_M);
}

void pll_2_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M)
{
    pll_start_min(RCC_PLL2DIVR, RCC_PLL2FRACR,
              PLL2ON, PLL2RDY, PLL2RGE, PLL2VCOSEL, PLL2FRACEN,
              DIVP2EN, DIVQ2EN, DIVR2EN, DIVM2,
              src, M, Nmul, FRACN, P, Q, R, fref_after_M);
}

void pll_3_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M)
{
    pll_start_min(RCC_PLL3DIVR, RCC_PLL3FRACR,
              PLL3ON, PLL3RDY, PLL3RGE, PLL3VCOSEL, PLL3FRACEN,
              DIVP3EN, DIVQ3EN, DIVR3EN, DIVM3,
              src, M, Nmul, FRACN, P, Q, R, fref_after_M);
}

clock_info_t init_clock(void) {
    return init_clock_adv(13u, 240u, 1u, 10u, 2u, 26000000u);
}

clock_info_t init_clock_adv (
        uint32_t M,
        uint32_t N,
        uint32_t P,
        uint32_t Q,
        uint32_t R,
        uint32_t HSE
        )
{

#define TIM6EN 4
    *RCC_APB1LENR |= (1<<TIM6EN); /* TIM6 used for delay functions */
    /*  Power / voltage scaling to VOS0 (overdrive)  */
    #define SCUEN       2
    #define LDOEN       1
    *PWR_CR3 |= (1u << SCUEN) | (1u << LDOEN);

    #define ACTVOSRDY   13
    while (!(*PWR_CSR1 & (1u << ACTVOSRDY))) {}

    /*  HSE on (no bypass)  */
    #define HSEBYP      18
    #define HSERDY      17
    #define HSEON       16
    *RCC_CR &= ~(1u << HSEBYP);
    *RCC_CR |=  (1u << HSEON);
    while (!(*RCC_CR & (1u << HSERDY))) {}

    /*  VOS0 (boost)  */
    #define VOS         14      /* PWR_D3CR[15:14] */
    #define VOSRDY      13
    *PWR_D3CR = (*PWR_D3CR & ~(0x3u << VOS)) | (0x3u << VOS);

    #define SYSCFGEN    1
    #define ODEN        0
    *RCC_APB4ENR  |= (1u << SYSCFGEN);
    *SYSCFG_PWRCR |= (1u << ODEN);
    while (!(*PWR_D3CR & (1u << VOSRDY))) {}

    /*  Prescalers: D1CPRE=/1, HPRE=/2, APB*=/2  */
    #define D1CPRE      8
    #define HPRE        0
    #define D1PPRE      4
    *RCC_D1CFGR =
        (0x0u << D1CPRE) |    /* /1  (CPU domain) */
        (0x8u << HPRE)   |    /* /2  (AHB/HCLK)   */
        (0x4u << D1PPRE);     /* /2  (APB3/PCLK3) */

    #define D2PPRE1     4
    #define D2PPRE2     8
    *RCC_D2CFGR =
        (0x4u << D2PPRE1) |   /* /2 (APB1/PCLK1) */
        (0x4u << D2PPRE2);    /* /2 (APB2/PCLK2) */

    #define D3PPRE      4
    *RCC_D3CFGR = (0x4u << D3PPRE); /* /2 (APB4/PCLK4) */

    pll_1_start(PLLSRC_HSE, M, N, 0u, P, Q, R, 26000000u);

    /*  Flash latency + write high-frequency  */
    #define LATENCY     0
    #define WRHIGHFREQ  4
    *FLASH_ACR = ((*FLASH_ACR & ~((0x7u << LATENCY) | (0x3u << WRHIGHFREQ))) |
                  (0x7u << LATENCY) | (0x2u << WRHIGHFREQ));

    /*  Switch SYSCLK to PLL1 (P output)  */
    #define SW          0
    #define SWS         3
    *RCC_CFGR = (*RCC_CFGR & ~(0x3u << SW)) | (0x3u << SW); /* 0b11 = PLL1 */
    while (((*RCC_CFGR >> SWS) & 0x3u) != 0x3u) {}

    /* ========= Integer clock math (no fractional considered) ========= */

    /* fvco = (HSE / M) * N  (integer math) */
    uint32_t fref       = (uint32_t)(HSE / M);              /* assume divisible */
    ci.hse_hz           = HSE;
    ci.pll1_vco_hz      = (uint32_t)((uint64_t)fref * (uint64_t)N);

    ci.pll1_p_hz        = (P ? (ci.pll1_vco_hz / P) : 0u);
    ci.pll1_q_hz        = (Q ? (ci.pll1_vco_hz / Q) : 0u);
    ci.pll1_r_hz        = (R ? (ci.pll1_vco_hz / R) : 0u);

    ci.sysclk_hz        = ci.pll1_p_hz;

    /* HPRE=/2 -> HCLK */
    ci.hclk_hz          = ci.sysclk_hz / 2u;

    /* APB prescalers all /2 here */
    ci.pclk1_apb1_hz         = ci.hclk_hz / 2u;  /* APB1 */
    ci.pclk2_apb2_hz         = ci.hclk_hz / 2u;  /* APB2 */
    ci.pclk3_apb3_hz         = ci.hclk_hz / 2u;  /* APB3 */
    ci.pclk4_apb4_hz         = ci.hclk_hz / 2u;  /* APB4 */

    /* Timer kernel clocks: if APB prescaler != /1 → timer_clk = PCLKx * 2 */
    ci.tim_apb1_hz      = ci.pclk1_apb1_hz * 2u; /* TIM2/3/4/5... */
    ci.tim_apb2_hz      = ci.pclk2_apb2_hz * 2u; /* TIM1/TIM8 */

    return ci;
}
