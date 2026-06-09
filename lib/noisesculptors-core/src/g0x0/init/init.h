
#ifndef INIT_H
#define INIT_H

#include<stdint.h>

typedef struct {
    uint32_t clk_hz;
    uint32_t pll_vco_hz;
    uint32_t pll_r_hz;
    uint32_t sysclk_hz;
    uint32_t hclk_hz;
    uint32_t pclk1_apb1_hz;
    uint32_t tim_apb1_hz;
} clock_info_t;

clock_info_t init_sysclk_pll(
        uint32_t src,
        uint32_t M,
        uint32_t N,
        uint32_t R,
        uint32_t osc_hz
        );

clock_info_t init_clock(void);

#endif

