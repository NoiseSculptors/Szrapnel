
#include "gpio.h"

void gpio_ctrl(uint32_t gpio_dev, uint8_t reg_off, uint16_t bit_mask, uint8_t op)
{
    uint8_t erase_msk;
    uint8_t mult;
    uint8_t afr_h;
    volatile uint32_t *reg;

    reg = (volatile uint32_t*)(gpio_dev + reg_off);

    switch(reg_off){
        /* x1 */
        case GPIO_OTYPE:
            erase_msk = 1;
            mult = 1;
        break;

        /* x2 */
        case GPIO_MODE:
        case GPIO_OSPEED:
        case GPIO_PUPD:
            erase_msk = 3;
            mult = 2;
        break;

        /* x4 split into AFRH:31..16, AFRL:15..0 */
        case GPIO_AFRH:
        case GPIO_AFRL:
            erase_msk = 0xf;
            mult = 4;
        break;
    }

    if(reg_off == GPIO_AFRH)
        afr_h = 8;
    else
        afr_h = 0;

    for(int i=0;i<=15;i++){
        if(bit_mask & (1u<<i)){
            *reg &= ~(erase_msk << (i-afr_h)*mult); 
            if(op>0)
                *reg |= (op << (i-afr_h)*mult); 
        }
    }
}

