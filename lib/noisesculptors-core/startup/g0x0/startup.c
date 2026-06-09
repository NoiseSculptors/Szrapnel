
#include "vtor.h"
#include <stdint.h>

/* Symbols from the linker script (g0x0/linker_*.ld) */
extern uint32_t __StackTop;

extern uint32_t __VectorTable;       /* ORIGIN(FLASH) or ORIGIN(SRAM) */

extern uint32_t __data_load_start__; /* LMA in FLASH (LOADADDR(.data)) */
extern uint32_t __data_start__;      /* VMA in SRAM */
extern uint32_t __data_end__;        /* VMA in SRAM */

extern uint32_t __bss_start__;       /* VMA in SRAM */
extern uint32_t __bss_end__;         /* VMA in SRAM */

void Reset_Handler(void);
void def_irq_handler(void);

/* ---------------- Core exception weak aliases (ARMv6-M: M0+) ---------------- */
void NMI_Handler(void)        __attribute__((weak, alias("def_irq_handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("def_irq_handler")));
void SVC_Handler(void)        __attribute__((weak, alias("def_irq_handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("def_irq_handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("def_irq_handler")));

/* ---------------- External IRQs (RM0454 Rev 5, Table 45) -------------------- */
/* Position 0..31 below are NVIC external interrupt numbers.                    */
/* Naming: IRQ_<AcronymFromTable>_Handler                                       */

void IRQ_WWDG_Handler(void)                         __attribute__((weak, alias("def_irq_handler")));            /* 0  */
void IRQ_RTC_TAMP_Handler(void)                     __attribute__((weak, alias("def_irq_handler")));            /* 2  (RTC / TAMP) */
void IRQ_FLASH_Handler(void)                        __attribute__((weak, alias("def_irq_handler")));            /* 3  */
void IRQ_RCC_Handler(void)                          __attribute__((weak, alias("def_irq_handler")));            /* 4  */
void IRQ_EXTI0_1_Handler(void)                      __attribute__((weak, alias("def_irq_handler")));            /* 5  */
void IRQ_EXTI2_3_Handler(void)                      __attribute__((weak, alias("def_irq_handler")));            /* 6  */
void IRQ_EXTI4_15_Handler(void)                     __attribute__((weak, alias("def_irq_handler")));            /* 7  */
void IRQ_DMA1_Channel1_Handler(void)                __attribute__((weak, alias("def_irq_handler")));            /* 9  */
void IRQ_DMA1_Channel2_3_Handler(void)              __attribute__((weak, alias("def_irq_handler")));            /* 10 */
void IRQ_DMA1_Channel4_5_6_7_DMAMUX_DMA2_Channel1_2_3_4_5_Handler(void)
                                                    __attribute__((weak, alias("def_irq_handler")));            /* 11 (combined) */
void IRQ_ADC_Handler(void)                           __attribute__((weak, alias("def_irq_handler")));           /* 12 (with EXTI17/18) */
void IRQ_TIM1_BRK_UP_TRG_COM_Handler(void)          __attribute__((weak, alias("def_irq_handler")));            /* 13 */
void IRQ_TIM1_CC_Handler(void)                       __attribute__((weak, alias("def_irq_handler")));           /* 14 */
void IRQ_TIM3_TIM4_Handler(void)                     __attribute__((weak, alias("def_irq_handler")));           /* 16 (TIM3+TIM4) */
void IRQ_TIM6_Handler(void)                          __attribute__((weak, alias("def_irq_handler")));           /* 17 */
void IRQ_TIM7_Handler(void)                          __attribute__((weak, alias("def_irq_handler")));           /* 18 */
void IRQ_TIM14_Handler(void)                         __attribute__((weak, alias("def_irq_handler")));           /* 19 */
void IRQ_TIM15_Handler(void)                         __attribute__((weak, alias("def_irq_handler")));           /* 20 */
void IRQ_TIM16_Handler(void)                         __attribute__((weak, alias("def_irq_handler")));           /* 21 */
void IRQ_TIM17_Handler(void)                         __attribute__((weak, alias("def_irq_handler")));           /* 22 */
void IRQ_I2C1_Handler(void)                          __attribute__((weak, alias("def_irq_handler")));           /* 23 */
void IRQ_I2C2_I2C3_Handler(void)                     __attribute__((weak, alias("def_irq_handler")));           /* 24 (combined) */
void IRQ_SPI1_Handler(void)                          __attribute__((weak, alias("def_irq_handler")));           /* 25 */
void IRQ_SPI2_SPI3_Handler(void)                     __attribute__((weak, alias("def_irq_handler")));           /* 26 (combined) */
void IRQ_USART1_Handler(void)                        __attribute__((weak, alias("def_irq_handler")));           /* 27 */
void IRQ_USART2_Handler(void)                        __attribute__((weak, alias("def_irq_handler")));           /* 28 */
void IRQ_USART3_USART4_USART5_USART6_Handler(void)   __attribute__((weak, alias("def_irq_handler")));           /* 29 (combined) */

/* ---------------- Small util ---------------- */
static inline void copy32(uint32_t *dst, const uint32_t *src, uint32_t *end){
    while (dst < end) { *dst++ = *src++; }
}
static inline void zero32(uint32_t *dst, uint32_t *end){
    while (dst < end) { *dst++ = 0u; }
}

/* Default handler */
void def_irq_handler(void) { while (1) {} }

/* Your main() */
int main(void);

/* ---------------- Reset sequence ---------------- */
void Reset_Handler(void)
{
    /* Point VTOR to the vector table (FLASH or SRAM, per linker script) */
    *SCB_VTOR = (uint32_t)&__VectorTable;   /* safe on G0 */
    __asm volatile ("dsb"); __asm volatile ("isb");

    /* Copy .data from FLASH to SRAM */
    copy32(&__data_start__, &__data_load_start__, &__data_end__);

    /* Zero .bss */
    zero32(&__bss_start__, &__bss_end__);

    /* If using C++/newlib init arrays, call __libc_init_array() here */

    (void)main();
    while (1) {}
}

/* ---------------- Vector table ------------------ */
__attribute__((section(".isr_vector")))
__attribute__((used, aligned(128)))
void (* const vector_table[])(void) = {
    (void (*)(void))(&__StackTop), /* 0: Initial MSP */
    Reset_Handler,                 /* 1: Reset */

    /* ---- Cortex-M0+ system exceptions ---- */
    NMI_Handler,                   /* 2: NMI */
    HardFault_Handler,             /* 3: HardFault */
    0, 0, 0, 0,                    /* 4..7: Reserved on v6-M (no Mem/BUS/Usage/RES) */
    0, 0, 0,                       /* 8..10: Reserved */
    SVC_Handler,                   /* 11: SVCall */
    0,                             /* 12: Reserved (no DebugMon) */
    0,                             /* 13: Reserved */
    PendSV_Handler,                /* 14: PendSV */
    SysTick_Handler,               /* 15: SysTick */

    /* ---- External interrupts (NVIC position 0..31) ---- */
    IRQ_WWDG_Handler,                                  /* 0  */
    0,                                                 /* 1  Reserved */
    IRQ_RTC_TAMP_Handler,                              /* 2  RTC / TAMP */
    IRQ_FLASH_Handler,                                 /* 3  */
    IRQ_RCC_Handler,                                   /* 4  */
    IRQ_EXTI0_1_Handler,                               /* 5  */
    IRQ_EXTI2_3_Handler,                               /* 6  */
    IRQ_EXTI4_15_Handler,                              /* 7  */
    0,                                                 /* 8  Reserved */
    IRQ_DMA1_Channel1_Handler,                         /* 9  */
    IRQ_DMA1_Channel2_3_Handler,                       /* 10 */
    IRQ_DMA1_Channel4_5_6_7_DMAMUX_DMA2_Channel1_2_3_4_5_Handler, /* 11 */
    IRQ_ADC_Handler,                                   /* 12 */
    IRQ_TIM1_BRK_UP_TRG_COM_Handler,                   /* 13 */
    IRQ_TIM1_CC_Handler,                               /* 14 */
    0,                                                 /* 15 Reserved */
    IRQ_TIM3_TIM4_Handler,                             /* 16 */
    IRQ_TIM6_Handler,                                  /* 17 */
    IRQ_TIM7_Handler,                                  /* 18 */
    IRQ_TIM14_Handler,                                 /* 19 */
    IRQ_TIM15_Handler,                                 /* 20 */
    IRQ_TIM16_Handler,                                 /* 21 */
    IRQ_TIM17_Handler,                                 /* 22 */
    IRQ_I2C1_Handler,                                  /* 23 */
    IRQ_I2C2_I2C3_Handler,                             /* 24 */
    IRQ_SPI1_Handler,                                  /* 25 */
    IRQ_SPI2_SPI3_Handler,                             /* 26 */
    IRQ_USART1_Handler,                                /* 27 */
    IRQ_USART2_Handler,                                /* 28 */
    IRQ_USART3_USART4_USART5_USART6_Handler,           /* 29 */
    0,                                                 /* 30 Reserved */
    0                                                  /* 31 Reserved */
};

