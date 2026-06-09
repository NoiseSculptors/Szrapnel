
#include "init.h"
#include "systick.h"
#include <stdint.h>

extern clock_info_t ci;

void init_systick_1ms(void)
{
    uint32_t systick_clk = ci.hclk_hz / 8u;
    uint32_t reload = (systick_clk / 1000u) - 1u; // 1kHz tick

    *SYST_RVR = reload & 0x00FFFFFFu; // clamp to 24-bit
    *SYST_CVR = 0; // clear current
    *SYST_CSR = (1<<SYST_CSR_CLKSOURCE) |
                (1<<SYST_CSR_TICKINT) |
                (1<<SYST_CSR_ENABLE);
}

