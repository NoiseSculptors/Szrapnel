
#include "io.h"
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE    384000
#define BUFFER_MS      25

#define TWO_PI 6.283185307179586f

#define FREQ_HZ_L     440.0f
#define FREQ_HZ_R     440.5f

static float phaseL, phaseR;
static const float phaseStepL = (TWO_PI * FREQ_HZ_L) / (float)SAMPLE_RATE;
static const float phaseStepR = (TWO_PI * FREQ_HZ_R) / (float)SAMPLE_RATE;

void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    for (uint32_t i = 0; i < samples_in_buffer; i += 2) {

        phaseL += phaseStepL;
        phaseR += phaseStepR;

        if (phaseL >= TWO_PI) phaseL -= TWO_PI;
        if (phaseR >= TWO_PI) phaseR -= TWO_PI;

        audio_buffer[i]   = (int32_t)(sinf(phaseL) * 2147483647.0f);
        audio_buffer[i+1] = (int32_t)(sinf(phaseR) * 2147483647.0f);
    }

    lcd_draw_waveform(audio_buffer, SAMPLE_RATE/(BUFFER_MS*8));
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    audio_loop_start();
    return 0;
}

