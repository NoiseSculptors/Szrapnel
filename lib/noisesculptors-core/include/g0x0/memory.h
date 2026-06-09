#ifndef MEMORY_H
#define MEMORY_H

/* ======================== IOPORT ======================== */
#define GPIOA   0x50000000U
#define GPIOB   0x50000400U
#define GPIOC   0x50000800U
#define GPIOD   0x50000C00U
#define GPIOE   0x50001000U
#define GPIOF   0x50001400U

/* ======================== AHB peripherals ======================== */
#define DMA1    0x40020000U
#define DMA2    0x40020400U
#define DMAMUX  0x40020800U
#define RCC     0x40021000U
#define EXTI    0x40021800U
#define FLASH   0x40022000U
#define CRC     0x40023000U

/* ======================== APB peripherals ======================== */
/* Low APB range */
#define TIM3    0x40000400U
#define TIM4    0x40000800U
#define TIM6    0x40001000U
#define TIM7    0x40001400U
#define TIM14   0x40002000U
#define RTC     0x40002800U
#define WWDG    0x40002C00U
#define IWDG    0x40003000U
#define SPI2    0x40003800U
#define SPI3    0x40003C00U
#define USART2  0x40004400U
#define USART3  0x40004800U
#define USART4  0x40004C00U
#define USART5  0x40005000U
#define I2C1    0x40005400U
#define I2C2    0x40005800U
#define I2C3    0x40008800U
#define USB     0x40005C00U
#define PWR     0x40007000U

/* High APB range */
#define SYSCFG  0x40010000U
#define TIM1    0x40012C00U
#define ADC     0x40012400U
#define USART1  0x40013800U
#define USART6  0x40013C00U
#define SPI1    0x40013000U
#define TIM15   0x40014000U
#define TIM16   0x40014400U
#define TIM17   0x40014800U
#define DBG     0x40015800U

/* ======================== Cortex-M0+ internal ======================== */
#define SCS     0xE000E000U  /* SysTick, NVIC, SCB, etc. */

#endif
