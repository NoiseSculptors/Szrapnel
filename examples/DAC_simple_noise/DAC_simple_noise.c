
#include "rng.h"
#include "io.h"
#include <stdint.h>

void audio_fill_buffer(int32_t *stereo_buffer, uint32_t samples)
{
    for(uint32_t i=0;i<samples;i++)
        stereo_buffer[i] = rng_rnd();

    lcd_draw_waveform(stereo_buffer, 180);
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    dac_dma_loop();
    return 0;
}

