
#include "nvic.h"
#include <stdint.h>

void irq_enable(int32_t irqn)
{
    if(irqn >=0)
        NVIC_ISER[irqn >> 5] = (uint32_t)1u << (irqn & 0x1Fu);
}

void irq_disable(int32_t irqn)
{
    if(irqn >=0)
        NVIC_ICER[irqn >> 5] = (uint32_t)1u << (irqn & 0x1Fu);
}

