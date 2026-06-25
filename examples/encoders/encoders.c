
#include "delay.h"
#include "gpio.h"
#include "init.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "serial.h"
#include "timer.h"
#include "systick.h"
#include "io.h"
#include <stdint.h>

/* each bit (0-29) represents a led,
   leds[0] corresponds to the top left led */
extern uint32_t leds;

static void print_encoder_status(io_enc_t enc0, io_enc_t enc1)
{
    static uint16_t tmp0,tmp1,tmp2,tmp3;
    enc0 = io_enc_read(0);
    enc1 = io_enc_read(1);
    if((enc0.value!=tmp0) || (enc1.value!=tmp1) ||
       (enc0.delta!=tmp2) || (enc1.delta!=tmp3)
            )
        printf("%04x %04x %02d %02d\n", enc0.value, enc1.value,
                                        enc0.delta, enc1.delta);
    tmp0 = enc0.value;
    tmp1 = enc1.value;
    tmp2 = enc0.delta;
    tmp3 = enc1.delta;
}

int main(void) {

    init_clock();
    io_encoders_init();
    io_serial_init();

    io_enc_t encoder0, encoder1;

    while(1){
        print_encoder_status(encoder0, encoder1);
    }
}

