
#ifndef PWR_H
#define PWR_H

#include "memory.h"
#include <stdint.h>

#define PWR_CR1        ((volatile uint32_t *)(PWR + 0x00))
#define PWR_CR2        ((volatile uint32_t *)(PWR + 0x04))
#define PWR_CR3        ((volatile uint32_t *)(PWR + 0x08))
#define PWR_CR4        ((volatile uint32_t *)(PWR + 0x0C))
#define PWR_SR1        ((volatile uint32_t *)(PWR + 0x10))
#define PWR_SR2        ((volatile uint32_t *)(PWR + 0x14))
#define PWR_SCR        ((volatile uint32_t *)(PWR + 0x18))
#define PWR_PUCRA      ((volatile uint32_t *)(PWR + 0x20))
#define PWR_PDCRA      ((volatile uint32_t *)(PWR + 0x24))
#define PWR_PUCRB      ((volatile uint32_t *)(PWR + 0x28))
#define PWR_PDCRB      ((volatile uint32_t *)(PWR + 0x2C))
#define PWR_PUCRC      ((volatile uint32_t *)(PWR + 0x30))
#define PWR_PDCRC      ((volatile uint32_t *)(PWR + 0x34))
#define PWR_PUCRD      ((volatile uint32_t *)(PWR + 0x38))
#define PWR_PDCRD      ((volatile uint32_t *)(PWR + 0x3C))
#define PWR_PUCRE      ((volatile uint32_t *)(PWR + 0x40))
#define PWR_PDCRE      ((volatile uint32_t *)(PWR + 0x44))
#define PWR_PUCRF      ((volatile uint32_t *)(PWR + 0x48))
#define PWR_PDCRF      ((volatile uint32_t *)(PWR + 0x4C))

#endif

