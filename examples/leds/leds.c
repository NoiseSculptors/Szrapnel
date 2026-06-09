
#include "delay.h"
#include "gpio.h"
#include "init.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "serial.h"
#include "systick.h"
#include "io.h"
#include <stdint.h>

/* each bit (0-29) represents a led,
   leds[0] corresponds to the top left led */
extern uint32_t leds;

int main(void) {

    init_clock();
    init_rng();
    io_init();

    printf("-------\n");

    /* works if systick led routine not active */
    for(int i=0;i<30;i++){
        led_on(i);
//        delay_ms(30);
    }

    /* activate systick, trigger every ms */
    init_systick_1ms();

    uint8_t led_fb[5] = {
        0b011110,
        0b000010,
        0b011110,
        0b010000,
        0b011110,
    };

    fb_to_leds(led_fb);

    delay_ms(1000);

    while(1){
        leds = rng_rnd();
        delay_ms(150);
    }
}

