
#ifndef INIT_H
#define INIT_H

#include <stdint.h>

#define PLLSRC_HSI  0u
#define PLLSRC_CSI  1u
#define PLLSRC_HSE  2u

typedef struct {
    /* Inputs / PLL outputs */
    uint32_t hse_hz;
    uint32_t pll1_vco_hz;
    uint32_t pll1_p_hz;
    uint32_t pll1_q_hz;
    uint32_t pll1_r_hz;

    /* System / bus clocks */
    uint32_t sysclk_hz;   /* SYSCLK source = PLL1P in this setup */
    uint32_t hclk_hz;     /* AHB clock (HPRE applied) */
    uint32_t pclk1_apb1_hz;    /* APB1  */
    uint32_t pclk2_apb2_hz;    /* APB2  */
    uint32_t pclk3_apb3_hz;    /* APB3  */
    uint32_t pclk4_apb4_hz;    /* APB4  */

    /* Timer kernel clocks (APB prescaler x2 rule) */
    uint32_t tim_apb1_hz; /* e.g., TIM2/3/4/5… */
    uint32_t tim_apb2_hz; /* e.g., TIM1/TIM8   */
} clock_info_t;

clock_info_t init_clock_adv(
        uint32_t M,
        uint32_t N,
        uint32_t P,
        uint32_t Q,
        uint32_t R,
        uint32_t HSE
        );

clock_info_t init_clock(void);


/* valid pll src: PLLSRC_HSI, PLLSRC_CSI, PLLSRC_HSE */ 
void pll_1_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M);

void pll_2_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M);

void pll_3_start(uint32_t src, uint32_t M, uint32_t Nmul, uint32_t FRACN,
    uint32_t P, uint32_t Q, uint32_t R, uint32_t fref_after_M);

#endif

