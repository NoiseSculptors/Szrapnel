
#include "rng.h"
#include "io.h"
#include <stdint.h>

/* Fill the stereo audio buffer with random samples */
void audio_feed(int32_t *stereo_buffer, uint32_t samples)
{
    /* Generate left/right samples */
    for (uint32_t i = 0; i < samples; i += 2) {

        stereo_buffer[i + 0] = rng_rnd();
        stereo_buffer[i + 1] = rng_rnd();
    }

    /* Draw and update waveform display */
    lcd_draw_waveform(stereo_buffer, 180);
    lcd_flush_fb();
}

int main(void)
{
    /* Initialize peripherals */
    io_init();

    /* Start DAC DMA audio loop */
    audio_start();

    return 0;
}

