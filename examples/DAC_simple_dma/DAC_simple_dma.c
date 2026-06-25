
#include "rng.h"
#include "io.h"
#include <stdint.h>

void audio_feed(int32_t *stereo_buffer, uint32_t samples)
{
    for(uint32_t i=0;i<samples;i++)
        stereo_buffer[i] = rng_rnd();

    lcd_draw_waveform(stereo_buffer, samples);
}

int main(void)
{
    io_init();
    audio_loop_start();
    return 0;
}

