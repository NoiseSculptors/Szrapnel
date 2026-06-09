#ifndef DWT_H
#define DWT_H

#include "init.h"
#include <stdint.h>

void init_dwt(void);
void dwt_start(void);
#define dwt_stop dwt_now
uint32_t dwt_now(void);

double dwt_cycles_to_s(uint32_t cycles);
double dwt_cycles_to_ms(uint32_t cycles);
double dwt_cycles_to_us(uint32_t cycles);

#define DWT_CTRL    (volatile uint32_t *)(0xE0001000u)
#define DWT_CYCCNT  (volatile uint32_t *)(0xE0001004u)
#define DEMCR       (volatile uint32_t *)(0xE000EDFCu)
#define DWT_LAR     (volatile uint32_t *)(0xE0001FB0u)


#endif
