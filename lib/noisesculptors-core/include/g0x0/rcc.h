
#ifndef RCC_H
#define RCC_H

#include "memory.h"
#include <stdint.h>

#define GPIOAEN 0
#define GPIOBEN 1
#define GPIOCEN 2
#define GPIODEN 3
#define GPIOEEN 4
#define GPIOFEN 5

#define RCC_CR             ((volatile uint32_t *)(RCC + 0x00))
#define RCC_ICSCR          ((volatile uint32_t *)(RCC + 0x04))
#define RCC_CFGR           ((volatile uint32_t *)(RCC + 0x08))
#define RCC_PLLCFGR        ((volatile uint32_t *)(RCC + 0x0C))
#define RCC_CIER           ((volatile uint32_t *)(RCC + 0x18))
#define RCC_CIFR           ((volatile uint32_t *)(RCC + 0x1C))
#define RCC_CICR           ((volatile uint32_t *)(RCC + 0x20))
#define RCC_IOPRSTR        ((volatile uint32_t *)(RCC + 0x24))
#define RCC_AHBRSTR        ((volatile uint32_t *)(RCC + 0x28))
#define RCC_APBRSTR1       ((volatile uint32_t *)(RCC + 0x2C))
#define RCC_APBRSTR2       ((volatile uint32_t *)(RCC + 0x30))
#define RCC_IOPENR         ((volatile uint32_t *)(RCC + 0x34))
#define RCC_AHBENR         ((volatile uint32_t *)(RCC + 0x38))
#define RCC_APBENR1        ((volatile uint32_t *)(RCC + 0x3C))
#define RCC_APBENR2        ((volatile uint32_t *)(RCC + 0x40))
#define RCC_IOPSMENR       ((volatile uint32_t *)(RCC + 0x44))
#define RCC_AHBSMENR       ((volatile uint32_t *)(RCC + 0x48))
#define RCC_APBSMENR1      ((volatile uint32_t *)(RCC + 0x4C))
#define RCC_APBSMENR2      ((volatile uint32_t *)(RCC + 0x50))
#define RCC_CCIPR          ((volatile uint32_t *)(RCC + 0x54))
#define RCC_CCIPR2         ((volatile uint32_t *)(RCC + 0x58))
#define RCC_BDCR           ((volatile uint32_t *)(RCC + 0x5C))
#define RCC_CSR            ((volatile uint32_t *)(RCC + 0x60))

#endif

