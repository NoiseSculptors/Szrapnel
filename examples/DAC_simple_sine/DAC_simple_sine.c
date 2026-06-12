
#include <stdint.h>
#include <math.h>

#define SAMPLING_FREQ 48 
#include "io.h"

#define TWO_PI 6.283185307179586f

void audio_fill_buffer(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    static float phaseL, phaseR;

    for (uint32_t i = 0; i < samples_in_buffer; i += 2) {

        phaseL += (TWO_PI * 440.0f) / SAMPLE_RATE;
        phaseR += (TWO_PI * 440.5f) / SAMPLE_RATE;

        if (phaseL >= TWO_PI) phaseL -= TWO_PI;
        if (phaseR >= TWO_PI) phaseR -= TWO_PI;

        audio_buffer[i]   = (int32_t)(sinf(phaseL) * 2147483647.0f);
        audio_buffer[i+1] = (int32_t)(sinf(phaseR) * 2147483647.0f);
    }

    lcd_draw_waveform(audio_buffer, 300);
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    dac_dma_loop();
    return 0;
}

