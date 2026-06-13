
#include "arm_math.h"
#include "io.h"
#include <stdint.h>

#define FREQ_HZ_L     440u
#define FREQ_HZ_R     440.5f

static uint32_t phaseL, phaseR = 0;

static const uint32_t phaseStepL =
    (uint32_t)(((uint64_t)FREQ_HZ_L << 32) / SAMPLE_RATE);
static const uint32_t phaseStepR =
    (uint32_t)(((double)FREQ_HZ_R * 4294967296.0) / (double)SAMPLE_RATE);

void audio_fill_buffer(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    for (uint32_t i = 0; i < samples_in_buffer; i += 2) {
        phaseL += phaseStepL;
        phaseR += phaseStepR;

        /* Use the top 31 bits as Q31 phase input */
        q31_t sinL = arm_sin_q31((q31_t)(phaseL >> 1));
        q31_t sinR = arm_sin_q31((q31_t)(phaseR >> 1));

        audio_buffer[i]   = sinL;
        audio_buffer[i+1] = sinR;
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

