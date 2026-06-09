
#include "delay.h"
#include "gpio.h"
#include "init.h"
#include "rcc.h"

int main(void) {

    /* connect a LED **through a resistor** (at least a 270OHM one) to the PB6 */

    init_clock();

    /* enable clock for GPIOB */
    *RCC_AHB4ENR |= (0x1u<<GPIOBEN);

    /* PB6, configure as output */
    gpio_ctrl(GPIOB, GPIO_MODE, GPIO6, MODE_OUTPUT);

    while (1) {
        /*
           The BSRR (Bit Set/Reset Register) allows you to set or clear
           specific bits in an output port by writing a 1 to either the "Set"
           half (lower 16 bits) or the "Reset" half (upper 16 bits). Writing a
           0 to any bit has **no effect**, enabling atomic, single-cycle
           manipulation of individual pins without needing a read-modify-write
           cycle.
        */

        /* like this */
        *GPIOB_BSRR = (1<<(6+16));
        delay_ms(500);
        *GPIOB_BSRR = (1<<6);
        delay_ms(500);

        /* or like this */
        *GPIOB_BSRR = 0x400000;
        delay_ms(500);
        *GPIOB_BSRR = 0x40;
        delay_ms(500);
    }
}

