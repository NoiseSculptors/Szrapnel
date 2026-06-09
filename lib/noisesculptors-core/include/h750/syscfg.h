
#ifndef SYSCFG_H
#define SYSCFG_H

#include "memory.h"
#include <stdint.h>

#define PA     0U
#define PB     1U
#define PC     2U
#define PD     3U
#define PE     4U
#define PF     5U
#define PG     6U
#define PH     7U
#define PI     8U
#define PJ     9U
#define PK     10U

#define EXTI0  0U
#define EXTI1  4U
#define EXTI2  8U
#define EXTI3  12U
#define EXTI4  0U
#define EXTI5  4U
#define EXTI6  8U
#define EXTI7  12U
#define EXTI8  0U
#define EXTI9  4U
#define EXTI10 8U  
#define EXTI11 12U 
#define EXTI12 0U
#define EXTI13 4U
#define EXTI14 8U
#define EXTI15 12U

#define SYSCFG_PMCR          (volatile uint32_t *)(SYSCFG + 0x004U)
#define SYSCFG_EXTICR1       (volatile uint32_t *)(SYSCFG + 0x008U)
#define SYSCFG_EXTICR2       (volatile uint32_t *)(SYSCFG + 0x00CU)
#define SYSCFG_EXTICR3       (volatile uint32_t *)(SYSCFG + 0x010U)
#define SYSCFG_EXTICR4       (volatile uint32_t *)(SYSCFG + 0x014U)
#define SYSCFG_CCSR          (volatile uint32_t *)(SYSCFG + 0x020U)
#define SYSCFG_CCVR          (volatile uint32_t *)(SYSCFG + 0x024U)
#define SYSCFG_CCCR          (volatile uint32_t *)(SYSCFG + 0x028U)
#define SYSCFG_PWRCR         (volatile uint32_t *)(SYSCFG + 0x02cU)
#define SYSCFG_PKGR          (volatile uint32_t *)(SYSCFG + 0x124U)
#define SYSCFG_UR0           (volatile uint32_t *)(SYSCFG + 0x300U)
#define SYSCFG_UR2           (volatile uint32_t *)(SYSCFG + 0x308U)
#define SYSCFG_UR3           (volatile uint32_t *)(SYSCFG + 0x30CU)
#define SYSCFG_UR4           (volatile uint32_t *)(SYSCFG + 0x310U)
#define SYSCFG_UR5           (volatile uint32_t *)(SYSCFG + 0x314U)
#define SYSCFG_UR6           (volatile uint32_t *)(SYSCFG + 0x318U)
#define SYSCFG_UR7           (volatile uint32_t *)(SYSCFG + 0x31CU)
#define SYSCFG_UR8           (volatile uint32_t *)(SYSCFG + 0x320U)
#define SYSCFG_UR9           (volatile uint32_t *)(SYSCFG + 0x324U)
#define SYSCFG_UR10          (volatile uint32_t *)(SYSCFG + 0x328U)
#define SYSCFG_UR11          (volatile uint32_t *)(SYSCFG + 0x32CU)
#define SYSCFG_UR12          (volatile uint32_t *)(SYSCFG + 0x330U)
#define SYSCFG_UR13          (volatile uint32_t *)(SYSCFG + 0x334U)
#define SYSCFG_UR14          (volatile uint32_t *)(SYSCFG + 0x338U)
#define SYSCFG_UR15          (volatile uint32_t *)(SYSCFG + 0x33CU)
#define SYSCFG_UR16          (volatile uint32_t *)(SYSCFG + 0x340U)
#define SYSCFG_UR17          (volatile uint32_t *)(SYSCFG + 0x344U)

#endif

