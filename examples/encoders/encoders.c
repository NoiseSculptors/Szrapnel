
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
    init_systick_1ms();
    io_init();

    io_enc_t encoder0, encoder1;

    while(1){
        static uint16_t tmp0,tmp1;
        encoder0 = io_enc_read(0);
        encoder1 = io_enc_read(1);
        if((encoder0.value!=tmp0) || (encoder1.value!=tmp1))
            printf("%04x %04x\n", encoder0.value, encoder1.value);
        tmp0 = encoder0.value;
        tmp1 = encoder1.value;
    }
}

