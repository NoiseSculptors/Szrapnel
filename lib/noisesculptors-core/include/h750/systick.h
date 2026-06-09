#ifndef SYSTICK_h
#define SYSTICK_h

#include <stdint.h>

#define SYSTICK_CTRL   SYST_CSR
#define SYSTICK_LOAD   SYST_RVR
#define SYSTICK_VAL    SYST_CVR
#define SYSTICK_CALIB  SYST_CALIB

#define SYST_CSR_COUNTFLAG 16
#define SYST_CSR_CLKSOURCE 3
#define SYST_CSR_TICKINT   1
#define SYST_CSR_ENABLE    0

#define SYST_CSR     (volatile uint32_t *)(0xE000E010)
#define SYST_RVR     (volatile uint32_t *)(0xE000E014)
#define SYST_CVR     (volatile uint32_t *)(0xE000E018)
#define SYST_CALIB   (volatile uint32_t *)(0xE000E01C)

void init_systick_1ms(void);

#endif
