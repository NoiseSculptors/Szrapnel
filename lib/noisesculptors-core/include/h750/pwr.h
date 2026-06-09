#ifndef PWR_H
#define PWR_H

#include "memory.h"
#include <stdint.h>

#define PWR_CSR1      (volatile uint32_t *)(PWR + 0x004)
#define PWR_CR3       (volatile uint32_t *)(PWR + 0x00c)
#define PWR_D3CR      (volatile uint32_t *)(PWR + 0x018)

#endif
