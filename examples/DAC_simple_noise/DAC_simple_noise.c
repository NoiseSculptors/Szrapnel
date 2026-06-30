
#include "rng.h"
#include "io.h"
#include <stdint.h>

#define SAMPLE_RATE    384000
#define BUFFER_MS      50 /* max buffer size is 50ms mono or 25ms stereo
                             @384000 sample rate or 8x longer for 48000 */

/* Fill the audio buffer with random samples */
void audio_feed(int32_t *stereo_buffer, uint32_t mono_samples)
{
    /* Generate samples */
    for (uint32_t i = 0; i < mono_samples; i+=2){
        int32_t noise = (int32_t)rng_rnd() >> 1; /* lower the volume */
        stereo_buffer[i+0] = stereo_buffer[i+1] = noise;
    }

    /* Draw and update waveform display */
    lcd_draw_waveform(stereo_buffer, SAMPLE_RATE/(BUFFER_MS*4));
    lcd_flush_fb();
}

int main(void)
{
    /* Initialize peripherals */
    io_init();
    /* MONO/STEREO is for calculating buffer length (samples to send to DAC)
     * and configuring DMA */
    audio_config(SAMPLE_RATE, MONO, BUFFER_MS);

    /* Start DAC DMA audio loop */
    audio_loop_start();

    return 0;
}

