
#include "rng.h"
#include "gpio.h"
#include "io.h"
#include <stdint.h>

#define BTN25 (*GPIOC_IDR & (1<<7)) 
#define BTN30 (*GPIOE_IDR & (1<<3)) 

void audio_feed(int32_t *stereo_buffer, uint32_t samples)
{
    for (uint32_t i = 0; i < samples; i += 2) {

        int32_t L = BTN25 ? rng_rnd() : 0;
        int32_t R = BTN30 ? rng_rnd() : 0;

        stereo_buffer[i + 0] = L;
        stereo_buffer[i + 1] = R;
    }

    lcd_draw_waveform(stereo_buffer, 180);
    lcd_printf(0,0,1,1,WHITE,BLACK,"BTN25:L BTN30:R");
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_loop_start();

    return 0;
}

