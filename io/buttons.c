
#include "delay.h"
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "i2c.h"
#include "nvic.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "rng.h"
#include "spi.h"
#include "serial.h"
#include "syscall.h"
#include "systick.h"
#include "timer.h"
#include "io.h"
#include <string.h>

/*
 buttons:                   side: 
|A6 |B0 |C5 |D6 |E3 |       |D12|
|A8 |B1 |C6 |D7 |E4 |       |C1 |
|A9 |B10|C7 |D13|E5 |       |C0 | 
|A10|B11|C8 |D14|E6 |
|A15|B3 |C9 |D15|E15|
|   |B4 |C10|   |   |
|   |   |C13|   |   |
|   |   |C14|   |   |
|   |   |C15|   |   |

|Btn| GPIO |Btn | GPIO |Btn | GPIO |
|01 | D13  | 16 | C5   | A  | D12  |
|02 | C8   | 17 | D7   | B  | C1   |
|03 | B10  | 18 | E6   | C  | C0   |
|04 | B4   | 19 | C6   |    |      |
|05 | C13  | 20 | A10  |    |      |
|06 | C15  | 21 | C10  |    |      |
|07 | D14  | 22 | B0   |    |      |
|08 | C9   | 23 | B3   |    |      |
|09 | E15  | 24 | E5   |    |      |
|10 | A6   | 25 | C7   |    |      |
|11 | E4   | 26 | A9   |    |      |
|12 | C14  | 27 | A15  |    |      |
|13 | D15  | 28 | B1   |    |      |
|14 | A8   | 29 | D6   |    |      |
|15 | B11  | 30 | E3   |    |      |
------------------
with side buttons:
A 1000011101000000 0x8740
B 0000110000011011 0x0c1b
C 1110011111100011 0xe7e3
D 1111000011000000 0xf0c0
E 1000000001111000 0x8078
------------------
without side buttons:
A 1000011101000000 0x8740
B 0000110000011011 0x0c1b
C 1110011111100000 0xe7e0
D 1110000011000000 0xe0c0
E 1000000001111000 0x8078
------------------
side buttons:
C 0000000000000011 0x0003
D 0001000000000000 0x1000
------------------
*/

void io_buttons_init(void) {
    *RCC_AHB4ENR |= (1u<<GPIOAEN)|
                    (1u<<GPIOBEN)|
                    (1u<<GPIOCEN)|
                    (1u<<GPIODEN)|
                    (1u<<GPIOEEN);

/********** Buttons ***********************************************************/
    gpio_ctrl(GPIOA, GPIO_MODE, GPIO6|GPIO8|GPIO9|GPIO10|GPIO15, MODE_INPUT);
    gpio_ctrl(GPIOB, GPIO_MODE, GPIO0|GPIO1|GPIO3|GPIO4|GPIO10|GPIO11, MODE_INPUT);
    gpio_ctrl(GPIOC, GPIO_MODE, GPIO0|GPIO1|GPIO5|GPIO6|GPIO7|GPIO8|
                                GPIO9|GPIO10|GPIO13|GPIO14|GPIO15, MODE_INPUT);
    gpio_ctrl(GPIOD, GPIO_MODE, GPIO6|GPIO7|GPIO12|GPIO13|GPIO14|GPIO15,
                                MODE_INPUT);
    gpio_ctrl(GPIOE, GPIO_MODE, GPIO3|GPIO4|GPIO5|GPIO6|GPIO15, MODE_INPUT);
    /* default is pullup */
    gpio_ctrl(GPIOA, GPIO_PUPD, GPIO15, PUPD_PULLDOWN);
    gpio_ctrl(GPIOB, GPIO_PUPD, GPIO4, PUPD_PULLDOWN);
}

